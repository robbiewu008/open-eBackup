import warnings
import logging as log
import os
from enum import IntEnum
from sys import exit
from urllib3.exceptions import SubjectAltNameWarning, InsecureRequestWarning
import yaml
from typing import List
import getpass

import click
import consts
from subcommands import namespace as ns
from subcommands import dataprotect as dp
from subcommands import simbaos as sbos, packages as pkgs
from subcommands import pacific
from subcommands import dpserver

from dataprotect_deployment.client_manager import ClientManager
from dataprotect_deployment.pacific_interface import PacificClient
from dataprotect_deployment.dp_server_interface import DataProtectDeployClient
from config import Config, PacificNode, StoragePoolConfig, DataBackupConfig
from models.process_manager_databackup import ProcessManagerDataBackup

config: [Config, DataBackupConfig] = None
client_mgr: ClientManager = None


def check_hostname(hostname: str) -> bool:
    return hostname.lower() == hostname and hostname.isalnum()


def new_config_from_file(config_file: str) -> Config:
    try:
        with open(config_file, 'r', encoding='utf-8') as f:
            config_map = yaml.safe_load(f)
        config = Config(**config_map)
        config.validate()
    except Exception as e:
        log.error(f'Invalid config file. Check your config file. {e}')
        exit(1)

    try:
        pcli = PacificClient(config)
        servers = pacific.get_servers(pcli)
        config.set_nodes(servers)
        for s in servers:
            if not check_hostname(s.name):
                log.error(f'Invalid hostname {s.name}. Hostname can only '
                          f'contain lowercase letters and numbers')
                exit(1)
    except Exception as e:
        log.error(f"Unable to connect cluster. "
                  f"Check your config file and cluster status, {e}")
        exit(1)
    return config


def new_config_from_file_databackup(config_file: str) -> DataBackupConfig:
    try:
        with open(config_file, 'r', encoding='utf-8') as f:
            config_map = yaml.safe_load(f)
        config = DataBackupConfig(**config_map)
        config.validate()
    except Exception as e:
        log.error(f'Invalid config file. Check your config file. {e}')
        exit(1)
    return config


@click.group()
def main():
    log.basicConfig(
        level=log.INFO,
        format='%(asctime)s %(levelname)s: %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S',
    )

    # 忽略未检查SAN警告，DM证书没有包含SAN
    warnings.simplefilter('ignore', SubjectAltNameWarning)
    warnings.simplefilter('ignore', InsecureRequestWarning)
    log.info("Start parsing config")


@main.group()
@click.option('-f', '--config_file', required=True, help='config file')
def e6000(config_file):
    global config
    global client_mgr
    config = new_config_from_file(config_file)
    client_mgr = ClientManager(config)
    log.info("Parsing config completed")


@main.group()
@click.option('-f', '--config_file', required=True, help='config file')
def e1000(config_file):
    global config
    global client_mgr
    config = new_config_from_file_databackup(config_file)
    log.info("Parsing config completed")


@e1000.command("config_test")
def e1000_config_test():
    log.info(config)
    log.info(client_mgr.dp_clients)


@e1000.command("install")
@click.option('--no_rollback', is_flag=True)
def e1000_install(no_rollback):
    # 需要添加一些安装前检查项
    global config
    processmanager = ProcessManagerDataBackup(config, no_rollback)
    processmanager.exec("install")


@e1000.command("expand")
@click.option("--no_rollback", is_flag=True)
def e1000_expand(no_rollback):
    global config
    processmanager = ProcessManagerDataBackup(config, no_rollback)
    processmanager.exec("expand")


class Steps(IntEnum):
    Pacific = 0
    Namespace = 1
    UploadPackages = 2
    InstallSimbOS = 3
    InstallDataBackup = 4


def rollback(step: Steps, **kwargs):
    if kwargs.get('no_rollback'):
        return
    if client_mgr is None:
        return

    cli = kwargs.get('cli')
    pacific_cli = client_mgr.pacific_client
    account_id = kwargs.get('account_id')
    pool_id = kwargs.get('pool_id')

    try:
        if step >= Steps.InstallDataBackup:
            dp.reset(cli)
            nfs_namespaces = consts.OP_NFS_LIST
            for n in nfs_namespaces:
                ns.namespace_delete(cli, n, account_id)
        if step >= Steps.InstallSimbOS:
            sbos.reset(client_mgr, config.get_nodes())
        if step >= Steps.UploadPackages:
            for package_name in kwargs['package_names']:
                pkgs.delete(cli, package_name)
        if step >= Steps.Namespace:
            nfs_namespaces = consts.BASE_NFS_LIST
            for n in nfs_namespaces:
                ns.namespace_delete(cli, n, account_id)
            for dpcli in client_mgr.dp_clients.values():
                dpcli.umount_nfs_share()
            log.info('Successfully umount nfs at all nodes')
        if step >= Steps.Pacific:
            if pool_id is not None:
                pacific.delete_converged_eds_file_service(pacific_cli, pool_id)
                pacific.delete_storage_pool(pacific_cli, pool_id)
    except Exception as e:
        log.error(f'Failed to rollback, {e}')
        exit(1)


def upload_packages(cli: DataProtectDeployClient, rollback_kwargs):
    packages_to_upload = [
        config.simbaos.package,
        config.dataprotect.chart,
        config.dataprotect.image
    ]

    try:
        for package_path in packages_to_upload:
            log.info(f'Start upload and unpack package {package_path}')
            pkgs.upload(cli, package_path)
    except Exception as e:
        log.error(f'Failed to upload and unpack packages, {e}')
        rollback(step=Steps.UploadPackages, **rollback_kwargs)
        exit(1)


def create_storage_pool(
        pcli: PacificClient,
        storage_pool_config: StoragePoolConfig,
        rollback_kwargs
):
    try:
        pool_id = pacific.create_storage_pool(
            pcli, storage_pool_config, consts.STORAGE_POOL_NAME)
        rollback_kwargs['pool_id'] = pool_id

        pacific.create_converged_eds_file_service(pcli, pool_id)
        account_id = pacific.create_account(pcli, consts.STORAGE_ACCOUNT_NAME)
        rollback_kwargs['account_id'] = account_id
        return pool_id, account_id
    except Exception as e:
        log.error(f'Failed to create storage pool; {e}')
        rollback(step=Steps.Pacific, **rollback_kwargs)
        exit(1)


def create_namespaces(cli: DataProtectDeployClient,
                      pool_id, account_id, rollback_kwargs):
    log.info("Start create namespaces")
    nfs_namespaces = consts.OP_NFS_LIST + consts.BASE_NFS_LIST
    try:
        for n in nfs_namespaces:
            ns.namespace_create(cli, n, account_id, pool_id)
    except Exception as e:
        log.error(f'Failed to create namespaces; {e}')
        rollback(step=Steps.Namespace, **rollback_kwargs)
        exit(1)


def install_simbaos(
        cli: DataProtectDeployClient,
        nodes: [PacificNode],
        rollback_kwargs
):
    try:
        simbaos_status = cli.simbaos_get_status()
        if simbaos_status['status'] == 'deployed':
            log.error('Simbaos already deployed. Please reset simbaos '
                      'before reinstall')
            exit(1)
        sbos.install(config, client_mgr, nodes)
    except Exception as e:
        log.error(f'Failed to install simbaos, {e}')
        rollback(step=Steps.InstallSimbOS, **rollback_kwargs)
        exit(1)


def install_dataprotect(
        cli: DataProtectDeployClient,
        skip_image_load: bool,
        rollback_kwargs,
):
    try:
        chart_package_name = os.path.basename(config.dataprotect.chart)
        image_package_name = os.path.basename(config.dataprotect.image)
        dp.install(
            client_mgr,
            chart_package_name,
            image_package_name,
            skip_image_load,
        )
    except Exception as e:
        log.error(f'Failed to install dataprotect, {e}')
        rollback(step=Steps.InstallDataBackup, **rollback_kwargs)
        exit(1)


def check_configuration_completeness():
    try:
        config.check_for_completeness(check_all=True)
    except Exception as e:
        log.error(f"Invalid configuration file, {e}")
        exit(1)


@e6000.command('install')
@click.option('--no_rollback', is_flag=True)
@click.option('--skip_package_upload', is_flag=True)
@click.option('--skip_image_load', is_flag=True)
@click.option('--skip_install_simbaos', is_flag=True)
def install(
        no_rollback,
        skip_package_upload,
        skip_image_load,
        skip_install_simbaos
):
    check_configuration_completeness()
    nodes = config.get_nodes()
    node_names = [n.name for n in nodes]
    log.info(f"Start to install at nodes {node_names}")

    pacific_cli = client_mgr.get_pacific_client()
    cli = client_mgr.get_primary_fsm_client()

    rollback_kwargs = {
        'no_rollback': no_rollback,
        'cli': cli,
        'package_names': [
            os.path.basename(config.simbaos.package),
            os.path.basename(config.dataprotect.chart),
            os.path.basename(config.dataprotect.image),
        ]
    }

    pool_id, account_id = create_storage_pool(
        pacific_cli,
        config.storage_pool,
        rollback_kwargs
    )
    create_namespaces(cli, pool_id, account_id, rollback_kwargs)

    try:
        dpserver.check_dpserver_versions(client_mgr, nodes)
    except Exception as e:
        log.error(f"Failed to install, {e}")
        exit(1)

    if not skip_package_upload:
        upload_packages(cli, rollback_kwargs)
    if not skip_install_simbaos:
        install_simbaos(cli, nodes, rollback_kwargs)
    install_dataprotect(cli, skip_image_load, rollback_kwargs)
    log.info("Successfully installed cluster")


@e6000.command('upgrade_dpserver')
@click.option('--package', required=True, help='new dpserver package')
@click.option('--nodes', help='node names splited by ","')
def upgrade_dpserver(package, nodes: str):
    if not nodes:
        nodes = ','.join(config.get_nodes_name())
        log.info(f'Start to upgrade dpserver at all nodes: {nodes}')
    nodes = nodes.split(sep=',')

    # current Pacific nodes in cluster
    cur_nodes_name = config.get_nodes_name()

    invalid_node_names = [n for n in nodes if n not in cur_nodes_name]
    if len(invalid_node_names):
        log.error(f"Invalid node names: {invalid_node_names}")
        exit(1)

    upgrade_nodes = [n for n in config.get_nodes() if n.name in nodes]

    try:
        cli = client_mgr.get_primary_fsm_client()
        pkgs.upload(cli, package)
        dpserver.upgrade(client_mgr, package, upgrade_nodes)
    except Exception as e:
        log.error(f'Failed to upgrade dpserver, {e}')
        exit(1)


@e6000.command('upgrade_simbaos')
@click.option('--package', required=True, help='new SimbaOS package')
def upgrade_simbaos(package):
    try:
        cli = client_mgr.get_primary_fsm_client()
        pkgs.upload(cli, package)
        nodes = client_mgr.pacific_client.get_servers()
        package_name = os.path.basename(package)
        sbos.run_preupgrade_paraller(client_mgr, nodes, package_name, cert_type="pacific")
        sbos.upgrade(client_mgr, cert_type="pacific")
        sbos.run_postupgrade_paraller(client_mgr, nodes)
    except Exception as e:
        log.error(f"Failed to upgrade SimbaOS, {e}")
        exit(1)


@e6000.command('reset')
def reset():
    if client_mgr is None:
        return
    check_configuration_completeness()
    pacific_cli = client_mgr.get_pacific_client()
    cli = client_mgr.get_primary_fsm_client()
    account_id = pacific_cli.query_account(consts.STORAGE_ACCOUNT_NAME)
    rollback_kwargs = {
        'cli': cli,
        'account_id': account_id,
        'package_names': [
            os.path.basename(config.simbaos.package),
            os.path.basename(config.dataprotect.chart),
            os.path.basename(config.dataprotect.image),
        ]
    }

    pool_id = pacific.get_storage_pool_id(
        pacific_cli,
        consts.STORAGE_POOL_NAME
    )
    rollback_kwargs['pool_id'] = pool_id
    log.info('Start rollback')
    rollback(step=Steps.InstallDataBackup, **rollback_kwargs)
    log.info('Rollback succeed')


def rollback_expand(
        added_nodes: List[PacificNode],
        pool_id: int,
):
    """
    Reset SimbaOS at all added nodes and delete it from k8s cluster
    """
    log.info('start rollback expand cluster')
    try:
        sbos.reset(client_mgr, added_nodes)
        for node in added_nodes:
            sbos.delete_node(client_mgr, node)

        # 缩容文件服务
        if pool_id is not None:
            ip_list = [n.management_internal_ip for n in added_nodes]
            pcli = client_mgr.get_pacific_client()
            pacific.scale_down_converged_eds_file_service(pcli, pool_id, ip_list)
        log.info('Successfully rollback expand cluster')
    except Exception:
        log.error("Failed to rollback expand cluster")
        exit(1)


def expand_precheck(client_mgr: ClientManager, nodes: [str]):
    """检查当前集群状态和扩容参数的有效性, 返回当前节点和新增节点"""
    invalid_node_names = [n for n in nodes if n not in config.get_nodes_name()]
    if len(invalid_node_names):
        raise Exception(f"Invalid node names: {invalid_node_names}")
    simbaos_status = sbos.get_status(client_mgr)
    if simbaos_status['status'] != 'deployed':
        log.error("SimbaOS is not deployed")
        exit(1)
    simbaos_node_names = [n['name'] for n in simbaos_status['nodes']]
    if any(n in simbaos_node_names for n in nodes):
        raise Exception("Node already in cluster")
    return (
        [n for n in config.get_nodes() if n.name in simbaos_node_names],
        [n for n in config.get_nodes() if n.name in nodes],
    )


@e6000.command('expand')
@click.option('--nodes', required=True, help="node names separated by comma")
def expand(nodes):
    added_nodes: [PacificNode] = None
    pool_id = None
    try:
        nodes = nodes.split(',')
        current_nodes, added_nodes = expand_precheck(client_mgr, nodes)

        # 开启文件服务
        pcli = client_mgr.get_pacific_client()
        pool_id = pacific.get_storage_pool_id(pcli, consts.STORAGE_POOL_NAME)
        if pool_id is None:
            raise Exception("Failed to query storage pool DataBackup's id")
        ip_list = [n.management_internal_ip for n in added_nodes]
        pacific.expand_converged_eds_file_service(pcli, pool_id, ip_list)

        # 检查dpserver版本信息
        dpserver.check_dpserver_versions(client_mgr, current_nodes + added_nodes)
        # 扩容simbaos
        simbaos_package = os.path.basename(config.simbaos.package)
        sbos.expand(client_mgr, added_nodes, current_nodes, simbaos_package)
        # 扩容备份软件
        image_name = os.path.basename(config.dataprotect.image)
        chart_name = os.path.basename(config.dataprotect.chart)
        dp.expand(client_mgr, added_nodes, image_name, chart_name)
    except Exception as e:
        log.error(f'Failed to expand cluster. {e}')
        if added_nodes is not None:
            rollback_expand(added_nodes, pool_id)
        exit(1)


@e6000.command('delete_node')
@click.option('--node_name', required=True, help='node to be removed')
def delete_node(node_name):
    cli = client_mgr.get_primary_fsm_client()

    try:
        dpserver.check_dpserver_versions(client_mgr, config.get_nodes())
        simbaos_status = cli.simbaos_get_status()
        if not simbaos_status['status'] == 'deployed':
            log.warning(f'SimbaOS is not deployed, skip delete {node_name}')
            exit(0)

        simbaos_node = [
            n for n in simbaos_status['nodes']
            if n['name'] == node_name
        ]
        if len(simbaos_node) == 0:
            log.warning(f'Node {node_name} is not in cluster')

        # 1. adjust dataprotect worker_replicas and master_replicas
        chart_name = os.path.basename(config.dataprotect.chart)
        dp.delete_node(cli, node_name, simbaos_status, chart_name)

        # 2. reset simbaos at node_name
        deleted_node = config.get_node(node_name)
        sbos.reset(client_mgr, [deleted_node])

        # 3. delete simbaos node from cluster
        sbos.delete_node(client_mgr, deleted_node)

        # 4. reset converged file service at deleted node
        pcli = client_mgr.get_pacific_client()
        pool_id = pacific.get_storage_pool_id(pcli, consts.STORAGE_POOL_NAME)
        pacific.scale_down_converged_eds_file_service(
            pcli, pool_id,
            [deleted_node.management_internal_ip]
        )

        # 5. umount nfs at deleted nodes
        dpserver.umount(client_mgr, [deleted_node])
    except Exception as e:
        log.error(f'Failed to delete node from cluster, {e}')
        exit(1)


@e6000.group()
def namespace():
    pass


@namespace.command('create')
@click.argument('namespace_name')
@click.option('--account_id', required=True, help='account id')
@click.option('--pool_id', required=True)
def namespace_create(namespace_name, account_id, pool_id):
    try:
        cli = client_mgr.get_primary_fsm_client()
        ns.namespace_create(cli, namespace_name, account_id, pool_id)
        log.info('Successfully created namespace')
    except Exception as e:
        log.error(f'Failed to create name space: {e}')
        exit(1)


@namespace.command('get')
@click.argument('namespace_name')
@click.option('--account_id', required=True)
def namespace_get(namespace_name, account_id):
    try:
        cli = client_mgr.get_primary_fsm_client()
        ns_id = ns.namespace_get(cli, namespace_name, account_id)
        if ns_id is None:
            log.info(f'Namespace {namespace_name} not found')
        else:
            log.info(f'Successfully got namespace, namespace_id: {ns_id}')
    except Exception as e:
        log.error(f'Failed to get namespace: {e}')
        exit(1)


@namespace.command('delete')
@click.argument('namespace_name')
@click.option('--account_id', required=True, help='account id')
def namespace_delete(namespace_name, account_id):
    try:
        cli = client_mgr.get_primary_fsm_client()
        ns.namespace_delete(cli, namespace_name, account_id)
    except Exception as e:
        log.error(f'Failed to delete namespace {namespace_name}: {e}')
        exit(1)


@e6000.group()
def package():
    pass


@e6000.group()
def simbaos():
    pass


@simbaos.command('install')
def simbaos_install():
    try:
        config.check_for_completeness(check_simbaos=True)
        sbos.install(config, client_mgr, config.get_nodes())
    except Exception as e:
        log.error(f'Failed to install simbaos, {e}')
        exit(1)


@simbaos.command('reset')
def simbaos_reset():
    try:
        sbos.reset(client_mgr, config.get_nodes())
    except Exception as e:
        log.error(f'Failed to reset simbaos, {e}')
        exit(1)


@e6000.group()
def dataprotect():
    pass


@dataprotect.command('install')
@click.option('--skip_image_load', is_flag=True, help='跳过加载镜像步骤')
def dataprotect_install(skip_image_load):
    try:
        config.check_for_completeness(check_dataprotect=True)
        cli = client_mgr.get_primary_fsm_client()
        pcli = client_mgr.get_pacific_client()
        account_id = pcli.query_account(consts.STORAGE_ACCOUNT_NAME)
        pool_id = pcli.query_storage_pool(consts.STORAGE_POOL_NAME)
        nfs_namespaces = consts.OP_NFS_LIST
        for n in nfs_namespaces:
            ns.namespace_create(cli, n, account_id, pool_id)

        chart_package_name = os.path.basename(config.dataprotect.chart)
        image_package_name = os.path.basename(config.dataprotect.image)
        dp.install(
            client_mgr=client_mgr,
            chart_package_name=chart_package_name,
            image_package_name=image_package_name,
            skip_image_load=skip_image_load
        )
    except Exception as e:
        log.error(f'Failed to install dataprotect, {e}')
        exit(1)


@dataprotect.command('reset')
def dataprotect_reset():
    try:
        pcli = client_mgr.get_pacific_client()
        account_id = pcli.query_account(consts.STORAGE_ACCOUNT_NAME)
        master_cli = client_mgr.get_primary_fsm_client()
        dp.reset(master_cli)
        nfs_namespaces = consts.OP_NFS_LIST
        for n in nfs_namespaces:
            ns.namespace_delete(master_cli, n, account_id)
    except Exception as e:
        log.error(f'Failed to reset dataprotect, {e}')
        exit(1)


if __name__ == "__main__":
    main()
