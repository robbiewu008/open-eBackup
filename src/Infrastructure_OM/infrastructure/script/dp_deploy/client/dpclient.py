import math
import warnings
import logging as log
import sys
import os
from enum import IntEnum
from sys import exit
from urllib3.exceptions import SubjectAltNameWarning, InsecureRequestWarning
import yaml
from typing import List, Union
import click
from concurrent.futures import ThreadPoolExecutor, Future


import consts
from read_excel import read_xlsx
from subcommands import namespace as ns
from subcommands import dataprotect as dp
from subcommands import simbaos as sbos, packages as pkgs
from subcommands import pacific
from subcommands import dpserver
from dataprotect_deployment.client_manager import ClientManager
from dataprotect_deployment.pacific_interface import PacificClient
from dataprotect_deployment.dp_server_interface import DataProtectDeployClient
from dataprotect_deployment.dp_server_upgrade_interface import UpgradeDataProtectDeployClient, CheckType
from dataprotect_deployment.om_rest_interface import get_cluster_nodes_ip
from config import Config, PacificNode, StoragePoolConfig, DataBackupConfig
from models.process_manager_databackup import ProcessManagerDataBackup
from e6000_read_excel import read_e6000_xlsx
from client_exception import (
    DuplicatedNodesException,
    QueryStoragePoolIdFailException,
    SmbOSAlreadyInstallException,
    EDSFailException,
    SmartkitException,
    NodesAlreadyInClusterException,
    InvalidNodeNameException,
    SmbOSNotDeployException,
    InvaidHostNameException,
    ConnectClusterFailException,
    InvalidConfigFIleException,
    FailtoRollbackException,
)
from config import smart_response, SmartKitResponse
from upgrade.upgrade_util import UpgradeStruct, UpgradeTrait
from upgrade.upgrade_component import (
    get_cluster_nodes_ip_for_e1000,
    upgrade_get_pm_pe_replicas,
    upload,
    preinstall,
    upgrade_databackup,
    upgrade_dp,
)

config: Union[Config, DataBackupConfig] = None
client_mgr: ClientManager = None


def check_hostname(hostname: str) -> bool:
    return hostname.lower() == hostname and hostname.isalnum()


def _read_and_config(config_file, simbaos_package=None, chart_package=None, image_package=None) -> Config:
    try:
        password = None
        if not simbaos_package:
            if config_file == "-":
                config_file = sys.stdin.fileno()
            with open(config_file, "r", encoding="utf-8") as f:
                config_map = yaml.safe_load(f)
            if config_map.get("password"):
                config = Config(**config_map)
                return config
        else:
            config_map, password = read_e6000_xlsx(config_file, simbaos_package, chart_package, image_package)
        config = Config(password=password, **config_map)
        config.validate()
        return config
    except Exception as e:
        log.exception(f"Invalid config file. Check your config file. {e}")
        raise InvalidConfigFIleException(e)


def _pacific_config(config: Config):
    try:
        pcli = PacificClient(config)
        servers = pacific.get_servers(pcli)
        config.set_nodes(servers)
        for s in servers:
            if not check_hostname(s.name):
                log.error(f"Invalid hostname {s.name}. Hostname can only " f"contain lowercase letters and numbers")
                raise InvaidHostNameException(s.name)
    except Exception as e:
        log.error(f"Unable to connect cluster. " f"Check your config file and cluster status, {e}")
        raise ConnectClusterFailException(e)


def new_config_from_file(
    config_file, simbaos_package=None, chart_package=None, image_package=None, upgrade=False
) -> Config:

    suggestion = ""
    meessage_cn = ""
    meessage_en = ""
    try:
        config = _read_and_config(config_file, simbaos_package, chart_package, image_package)
        if not upgrade:
            _pacific_config(config)
    except SmartkitException as e:
        meessage_cn = e.message_cn
        meessage_en = e.message_en
        suggestion = e.suggestion
    except Exception:
        process_name = ("_check_config",)
        meessage_cn = f"{process_name}阶段出现异常"
        meessage_en = f"There are some errors happening in {process_name}"
        suggestion = "Please collect logs and contact engineer support"
    finally:
        if suggestion == "":
            return config
        r = SmartKitResponse(
            step_id="0",
            process="_check_config",
            progress="14%",
            errorMgCn=f"{meessage_cn}",
            errorMgEn=f"{meessage_en}",
            suggestion=suggestion,
        )
        log.error(f"{r.json()}")
        exit(1)


def upgrade_config_file(config_file: str) -> DataBackupConfig:
    with open(config_file, "r", encoding="utf-8") as f:
        config_map = yaml.safe_load(f)
    config_map["upgrade"] = True
    ip = config_map["preliminary_node"].get("ip")
    user = config_map["preliminary_node"]["op_admin_user"]
    pwd = config_map["preliminary_node"]["op_admin_pwd"]
    flag, node_info = get_cluster_nodes_ip(ip, user, pwd)
    if not flag:
        err_msg = node_info[0] if node_info else ""
        msg = f"Fail to get other nodes' ip, err_msg {err_msg}"
        log.error(msg)
        raise Exception(msg)
    config_map["upgrade_nodes"] = node_info
    config = DataBackupConfig(**config_map)
    return config


def new_config_from_file_databackup(config_file: str, upgrade: bool = False) -> DataBackupConfig:
    try:
        if upgrade:
            if config_file == "-":
                config_file = sys.stdin.fileno()
            config = upgrade_config_file(config_file)
        else:
            config_map = read_xlsx(config_file)
            config_map["preliminary_node"]["op_admin_user"] = None
            config_map["preliminary_node"]["op_admin_pwd"] = None
            for node in config_map.get("expand_nodes", []):
                node["op_admin_user"] = None
                node["op_admin_pwd"] = None
            config_map["upgrade_nodes"] = None
            config = DataBackupConfig(**config_map)
            config.validate()
    except Exception as e:
        log.error(f"Invalid config file. Check your config file. {e}")
        exit(1)
    return config


@click.group()
def main():
    log.basicConfig(
        level=log.INFO,
        format="%(asctime)s %(levelname)s: %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
    )

    # 忽略未检查SAN警告，DM证书没有包含SAN
    warnings.simplefilter("ignore", SubjectAltNameWarning)
    warnings.simplefilter("ignore", InsecureRequestWarning)
    log.info("Start parsing config")


class CtxEseriesObj:
    client: UpgradeDataProtectDeployClient
    cluster_client: dict[str, UpgradeDataProtectDeployClient]
    ip: str
    name: str
    pwd: str


class CtxEseriesUpgrade:
    obj: CtxEseriesObj


@main.group("upgrade")
@click.pass_context
@click.option("--dp_ip", required=True)
@click.option("--rest_ip")
@click.option("--user", required=True)
@click.option("--pwd", required=True)
def eseries_upgrade(ctx: CtxEseriesUpgrade, dp_ip: str, rest_ip: str, user: str, pwd: str):
    ctx.obj = CtxEseriesObj()
    ctx.obj.ip = rest_ip
    ctx.obj.name = user
    ctx.obj.pwd = pwd
    ctx.obj.client = UpgradeDataProtectDeployClient(
        dpserver_address=dp_ip, op_admin_user=user, op_admin_pwd=pwd, rest_ip=rest_ip
    )


@eseries_upgrade.command("precheck")
@click.pass_context
@click.option("--type", required=True)
def eseries_upgrade_precheck(ctx: CtxEseriesUpgrade, type: CheckType):
    ctx.obj.client.upgrade_check(type)


@eseries_upgrade.command("backup")
@click.pass_context
def eseries_upgrade_backup(ctx: CtxEseriesUpgrade):
    ctx.obj.client.upgrade_backup()


@eseries_upgrade.command("upload_e1000")
@click.option("--chart_path", required=True)
@click.option("--image_path", required=True)
@click.pass_context
def eseries_upgrade_upload_e1000(ctx: CtxEseriesUpgrade, chart_path: str, image_path: str):
    get_node_is_success, nodes_info = get_cluster_nodes_ip_for_e1000(ctx.obj.ip, ctx.obj.name, ctx.obj.pwd)
    if not get_node_is_success:
        msg = f"Fail to get onodes' info, {nodes_info}"
        log.error(f"{msg}")
        raise Exception(msg)
    user = ctx.obj.name
    pwd = ctx.obj.pwd
    ctx.obj.cluster_client = {
        ip: UpgradeDataProtectDeployClient(ip, op_admin_user=user, op_admin_pwd=pwd) for ip, _ in nodes_info
    }
    futures: list[Future[bool]] = []
    with ThreadPoolExecutor() as executor:
        for client in [*ctx.obj.cluster_client.values()]:
            for path in [image_path, chart_path]:
                futures.append(executor.submit(upload, client, path))
    if all([future.result() for future in futures]):
        log.info("Successfully upload all packages.")
    else:
        log.info("Fail to upload all packages")


@eseries_upgrade.command("upload_e6000")
@click.option("--chart_path", required=True)
@click.option("--image_path", required=True)
@click.pass_context
def eseries_upgrade_upload_e6000(ctx: CtxEseriesUpgrade, chart_path: str, image_path: str):
    futures: list[Future[bool]] = []
    client = ctx.obj.client
    with ThreadPoolExecutor() as executor:
        for path in [image_path, chart_path]:
            futures.append(executor.submit(upload, client, path))
    if all([future.result() for future in futures]):
        log.info("Successfully upload all packages.")
    else:
        log.error("Fail to upload all packages")


@eseries_upgrade.command("preinstall_dp")
@click.option("--type", required=True, help="E1000 or E6000")
@click.option("--pakcage_name", required=True, help="dp image package name")
@click.pass_context
def eseries_preinstall_dp(ctx: CtxEseriesUpgrade, type: str, package_name: str):
    log_f = log.Logger("Logger")
    preinstall_succeed = preinstall(log_f, [*ctx.obj.cluster_client.values()], package_name, type)
    if not preinstall_succeed:
        log_f.error("Fail to preinstall")
        return
    log_f.info("Succeefully preinstall")
    return


@eseries_upgrade.command("preinstall_dp")
@click.option("--type", required=True, help="E1000 or E6000")
@click.option("--chart_name", required=True, help="dp image package name")
@click.pass_context
def eseries_preinstall_dp(ctx: CtxEseriesUpgrade, type: str, chart_name: str):
    log_f = log.Logger("Logger")
    master_cnt, pe_replicas = upgrade_get_pm_pe_replicas(ctx.obj.client, log_f)
    if not master_cnt or not pe_replicas:
        return
    upgrade_succeed = upgrade_dp(ctx.obj.client, log_f, chart_name, master_cnt, pe_replicas, type)
    if not upgrade_succeed:
        return
    return


@eseries_upgrade.command("postcheck")
@click.pass_context
@click.option("--type", required=True)
def eseries_upgrade_postcheck(ctx: CtxEseriesUpgrade, type: CheckType):
    ctx.obj.client.upgrade_check(type)


@main.group()
@click.option("-f", "--config_file", help="config file")
@click.option("-P1", "--simbaos_package", help="file paths")
@click.option("-P2", "--chart_package")
@click.option("-P3", "--image_package")
@click.option("-U", "--upgrade")
def e6000(config_file, simbaos_package, chart_package, image_package, upgrade=False):
    global config
    global client_mgr
    config = new_config_from_file(config_file, simbaos_package, chart_package, image_package, upgrade)
    client_mgr = ClientManager(config, upgrade)
    log.info("Parsing config completed")


@main.group()
@click.option("-F", "-f", "--config_file", required=True, help="config file")
@click.option("-P1", "--simbaos_package", help="file paths")
@click.option("-P2", "--chart_package")
@click.option("-P3", "--image_package")
@click.option("-U", "--upgrade")
def e1000(config_file, simbaos_package, chart_package, image_package, skip_upgrade_simbaos=False, upgrade=False):
    global config
    global client_mgr
    config = new_config_from_file_databackup(config_file, upgrade)

    config.simbaos_package_path = simbaos_package
    config.chart_package_path = chart_package
    config.image_package_path = image_package
    config.skip_upgrade_simbaos = skip_upgrade_simbaos
    log.info("Parsing config completed")


@e1000.command("config_test")
def e1000_config_test():
    log.info(config)
    log.info(client_mgr.dp_clients)


@e1000.command("install")
@click.option("--no_rollback", is_flag=True)
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


class Ctx:
    obj: ProcessManagerDataBackup


@e1000.group("upgrade")
@click.pass_context
@click.option("-sysadmin", "--sysadmin", required=True)
@click.option("-password", "--password", required=True)
def e1000_upgrade_group(ctx: Ctx, sysadmin, password):
    global config
    ctx.obj = ProcessManagerDataBackup(config, False, sysadmin=sysadmin, password=password)


@e1000_upgrade_group.command("upload_packages")
@click.pass_context
def e1000_upgrade_upload_packages(ctx: Ctx):
    ctx.obj.exec("upload_packages")


@e1000_upgrade_group.command("pre_upgrade_dataprotect")
@click.pass_context
def e1000_pre_upgrade_dataprotect(ctx: Ctx):
    ctx.obj.exec("pre_upgrade_dataprotect")


@e1000_upgrade_group.command("dataprotect")
@click.pass_context
def e1000_upgrade_dataprotect(ctx: Ctx):
    ctx.obj.exec("upgrade_dataprotect")


@e1000_upgrade_group.command("simbaos")
@click.pass_context
def e1000_upgrade_simbaos(ctx: Ctx):
    ctx.obj.exec("upgrade_simbaos")


@e1000_upgrade_group.command("post_check")
@click.pass_context
def e1000_upgrade_post_upgrade_check(ctx: Ctx):
    ctx.obj.exec("post_upgrade_check")


class Steps(IntEnum):
    Pacific = 0
    Namespace = 1
    UploadPackages = 2
    InstallSimbOS = 3
    InstallDataBackup = 4


def rollback(step: Steps, **kwargs):
    if kwargs.get("no_rollback"):
        return
    if client_mgr is None:
        return

    cli = kwargs.get("cli")
    pacific_cli = client_mgr.pacific_client
    account_id = kwargs.get("account_id")
    pool_id = kwargs.get("pool_id")

    try:
        if step >= Steps.InstallDataBackup:
            dp.reset(cli)
            nfs_namespaces = consts.OP_NFS_LIST
            for n in nfs_namespaces:
                ns.namespace_delete(cli, n, account_id)
        if step >= Steps.InstallSimbOS:
            sbos.reset(client_mgr, config.get_nodes())
        if step >= Steps.UploadPackages:
            for package_name in kwargs["package_names"]:
                pkgs.delete(cli, package_name)
        if step >= Steps.Namespace:
            nfs_namespaces = consts.BASE_NFS_LIST
            for n in nfs_namespaces:
                ns.namespace_delete(cli, n, account_id)
            for dpcli in client_mgr.dp_clients.values():
                dpcli.umount_nfs_share()
            log.info("Successfully umount nfs at all nodes")
        if step >= Steps.Pacific:
            if pool_id is not None:
                pacific.delete_converged_eds_file_service(pacific_cli, pool_id)
                pacific.delete_storage_pool(pacific_cli, pool_id)
    except Exception as e:
        log.error(f"Failed to rollback, {e}")
        raise FailtoRollbackException(e)


def upload_packages(cli: DataProtectDeployClient, rollback_kwargs):
    packages_to_upload = [
        config.simbaos.package,
        config.dataprotect.chart,
        config.dataprotect.image,
    ]
    try:
        for package_path in packages_to_upload:
            if package_path:
                log.info(f"Start upload and unpack package {package_path}")
                pkgs.upload(cli, package_path)
    except Exception as e:
        log.error(f"Failed to upload and unpack packages, {e}")
        rollback(step=Steps.UploadPackages, **rollback_kwargs)
        raise


def create_storage_pool(pcli: PacificClient, storage_pool_config: StoragePoolConfig, rollback_kwargs):
    try:
        pool_id = pacific.create_storage_pool(pcli, storage_pool_config, consts.STORAGE_POOL_NAME)
        rollback_kwargs["pool_id"] = pool_id

        pacific.create_converged_eds_file_service(pcli, pool_id)
        account_id = pacific.create_account(pcli, consts.STORAGE_ACCOUNT_NAME)
        rollback_kwargs["account_id"] = account_id
        return pool_id, account_id
    except Exception as e:
        log.error(f"Failed to create storage pool; {e}")
        rollback(step=Steps.Pacific, **rollback_kwargs)
        raise


def create_namespaces(cli: DataProtectDeployClient, pool_id, account_id, rollback_kwargs):
    log.info("Start create namespaces")
    nfs_namespaces = consts.OP_NFS_LIST + consts.BASE_NFS_LIST
    try:
        for n in nfs_namespaces:
            ns.namespace_create(cli, n, account_id, pool_id)
    except Exception as e:
        log.error(f"Failed to create namespaces; {e}")
        rollback(step=Steps.Namespace, **rollback_kwargs)
        raise EDSFailException


def install_simbaos(cli: DataProtectDeployClient, nodes: List[PacificNode], rollback_kwargs):
    try:
        simbaos_status = cli.simbaos_get_status()
        if simbaos_status["status"] == "deployed":
            log.error("Simbaos already deployed. Please reset simbaos " "before reinstall")
            raise SmbOSAlreadyInstallException
        sbos.install(config, client_mgr, nodes)
    except Exception as e:
        log.error(f"Failed to install simbaos, {e}")
        rollback(step=Steps.InstallSimbOS, **rollback_kwargs)
        raise


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
        log.error(f"Failed to install dataprotect, {e}")
        rollback(step=Steps.InstallDataBackup, **rollback_kwargs)
        raise


def check_configuration_completeness():
    try:
        config.check_for_completeness(check_all=True)
    except Exception as e:
        log.error(f"Invalid configuration file, {e}")
        raise


class E6000Install:
    def __init__(
        self,
        client_mgr: ClientManager,
        config: Config,
        no_rollback,
        skip_package_upload,
        skip_image_load,
        skip_install_simbaos,
    ):
        self.client_mgr = client_mgr
        self.config = config
        self.no_rollback = no_rollback
        self.skip_package_upload = skip_package_upload
        self.skip_image_load = skip_image_load
        self.skip_install_simbaos = skip_install_simbaos

    def _check_config(self):
        client_mgr: ClientManager = self.client_mgr
        config = self.config
        no_rollback = self.no_rollback
        check_configuration_completeness()
        nodes = config.get_nodes()
        node_names = [n.name for n in nodes]
        log.info(f"Start to install at nodes {node_names}")

        pacific_cli = client_mgr.get_pacific_client()
        cli = client_mgr.get_primary_fsm_client()
        rollback_kwargs = {
            "no_rollback": no_rollback,
            "cli": cli,
            "package_names": [
                os.path.basename(config.simbaos.package),
                os.path.basename(config.dataprotect.chart),
                os.path.basename(config.dataprotect.image),
            ],
        }
        self.cli = cli
        self.pacific_cli = pacific_cli
        self.nodes = nodes
        self.rollback_kwargs = rollback_kwargs

    def _create_storage_pool(self):
        pacific_cli = self.pacific_cli
        storage_pool = self.config.storage_pool
        rollback_kwargs = self.rollback_kwargs
        pool_id, account_id = create_storage_pool(pacific_cli, storage_pool, rollback_kwargs)
        self.pool_id = pool_id
        self.account_id = account_id

    def _create_namespaces(self):
        cli = self.cli
        pool_id = self.pool_id
        account_id = self.account_id
        rollback_kwargs = self.rollback_kwargs
        create_namespaces(cli, pool_id, account_id, rollback_kwargs)

    def _check_dpserver_versions(self):
        client_mgr = self.client_mgr
        nodes = self.nodes
        try:
            dpserver.check_dpserver_versions(client_mgr, nodes)
        except Exception as e:
            log.error(f"Failed to install, {e}")
            raise

    def _upload_packages(self):
        cli = self.cli
        rollback_kwargs = self.rollback_kwargs
        skip_package_upload = self.skip_package_upload
        if not skip_package_upload:
            upload_packages(cli, rollback_kwargs)

    def _install_simbaos(self):
        skip_install_simbaos = self.skip_install_simbaos
        cli = self.cli
        nodes = self.nodes
        rollback_kwargs = self.rollback_kwargs
        if not skip_install_simbaos:
            install_simbaos(cli, nodes, rollback_kwargs)

    def _install_dataprotect(self):
        cli = self.cli
        skip_image_load = self.skip_image_load
        rollback_kwargs = self.rollback_kwargs
        install_dataprotect(cli, skip_image_load, rollback_kwargs)


@e6000.command("install")
@click.option("--no_rollback", is_flag=True)
@click.option("--skip_package_upload", is_flag=True)
@click.option("--skip_image_load", is_flag=True)
@click.option("--skip_install_simbaos", is_flag=True)
def install(no_rollback, skip_package_upload, skip_image_load, skip_install_simbaos):
    e6 = E6000Install(
        client_mgr,
        config,
        no_rollback,
        skip_package_upload,
        skip_image_load,
        skip_install_simbaos,
    )
    task_list = [
        e6._check_config,
        e6._create_storage_pool,
        e6._create_namespaces,
        e6._check_dpserver_versions,
        e6._upload_packages,
        e6._install_simbaos,
        e6._install_dataprotect,
    ]
    total_step_nums = len(task_list)

    for i, process in enumerate(task_list):
        re, _ = smart_response(process, i, total_step_nums)
        if not re:
            break


@e6000.command("upgrade_dpserver")
@click.option("--package", required=True, help="new dpserver package")
@click.option("--nodes", help='node names splited by ","')
def upgrade_dpserver(package, nodes: str):
    if not nodes:
        nodes = ",".join(config.get_nodes_name())
        log.info(f"Start to upgrade dpserver at all nodes: {nodes}")
    nodes = nodes.split(sep=",")

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
        log.error(f"Failed to upgrade dpserver, {e}")
        exit(1)


@e6000.command("upgrade_simbaos")
@click.option("--package", required=True, help="new SimbaOS package")
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


@e6000.command("reset")
def reset():
    if client_mgr is None:
        return
    check_configuration_completeness()
    pacific_cli = client_mgr.get_pacific_client()
    cli = client_mgr.get_primary_fsm_client()
    account_id = pacific_cli.query_account(consts.STORAGE_ACCOUNT_NAME)
    rollback_kwargs = {
        "cli": cli,
        "account_id": account_id,
        "package_names": [
            os.path.basename(config.simbaos.package),
            os.path.basename(config.dataprotect.chart),
            os.path.basename(config.dataprotect.image),
        ],
    }

    pool_id = pacific.get_storage_pool_id(pacific_cli, consts.STORAGE_POOL_NAME)
    rollback_kwargs["pool_id"] = pool_id
    log.info("Start rollback")
    rollback(step=Steps.InstallDataBackup, **rollback_kwargs)
    log.info("Rollback succeed")


def rollback_expand(
    added_nodes: List[PacificNode],
    pool_id: int,
):
    """
    Reset SimbaOS at all added nodes and delete it from k8s cluster
    """
    log.info("start rollback expand cluster")
    try:
        sbos.reset(client_mgr, added_nodes)
        for node in added_nodes:
            sbos.delete_node(client_mgr, node)

        # 缩容文件服务
        if pool_id is not None:
            ip_list = [n.management_internal_ip for n in added_nodes]
            pcli = client_mgr.get_pacific_client()
            pacific.scale_down_converged_eds_file_service(pcli, pool_id, ip_list)
        log.info("Successfully rollback expand cluster")
    except Exception:
        log.error("Failed to rollback expand cluster")
        exit(1)


def check_nodes_duplication(nodes):
    cnts = {}
    for n in nodes:
        cnts[n] = 1 + cnts.get(n, 0)
    return [n for n in cnts if cnts[n] >= 2]


def expand_precheck(client_mgr: ClientManager, nodes: List[str]):
    """检查当前集群状态和扩容参数的有效性, 返回当前节点和新增节点"""
    invalid_node_names = [n for n in nodes if n not in config.get_nodes_name()]
    if len(invalid_node_names):
        raise InvalidNodeNameException(invalid_node_names)
    dup_nodes = check_nodes_duplication(nodes)
    if dup_nodes:
        raise DuplicatedNodesException(dup_nodes)
    simbaos_status = sbos.get_status(client_mgr)
    if simbaos_status["status"] != "deployed":
        log.error("SimbaOS is not deployed")
        raise SmbOSNotDeployException
    simbaos_node_names = [n["name"] for n in simbaos_status["nodes"]]
    nodes_already_in_cluster = [n for n in nodes if n in simbaos_node_names]
    if nodes_already_in_cluster:
        raise NodesAlreadyInClusterException(nodes_already_in_cluster)
    return (
        [n for n in config.get_nodes() if n.name in simbaos_node_names],
        [n for n in config.get_nodes() if n.name in nodes],
    )


class E6000Expand:
    def __init__(
        self,
        client_mgr: ClientManager,
        config: Config,
        nodes,
        current_nodes=None,
        added_nodes=None,
        pcli=None,
        pool_id=None,
        ip_list=None,
    ):
        self.client_mgr = client_mgr
        self.config = config
        self.nodes = nodes
        self.current_nodes = current_nodes
        self.added_nodes = added_nodes
        self.pcli = pcli
        self.pool_id = pool_id
        self.ip_list = ip_list


def _expand_check(info: E6000Expand) -> E6000Expand:
    client_mgr = info.client_mgr
    nodes = info.nodes
    info.current_nodes, info.added_nodes = expand_precheck(client_mgr, nodes)
    return info


def _get_expand_info(info: E6000Expand) -> E6000Expand:
    info.pcli = info.client_mgr.get_pacific_client()
    info.pool_id = pacific.get_storage_pool_id(info.pcli, consts.STORAGE_POOL_NAME)
    if info.pool_id is None:
        raise QueryStoragePoolIdFailException
    info.ip_list = [n.management_internal_ip for n in info.added_nodes]
    return info


def _expand_converged_eds_file_service(info: E6000Expand):
    pcli = info.pcli
    pool_id = info.pool_id
    ip_list = info.ip_list
    pacific.expand_converged_eds_file_service(pcli, pool_id, ip_list)


def _check_dpserver_versions(info: E6000Expand):
    client_mgr = info.client_mgr
    current_nodes = info.current_nodes
    added_nodes = info.added_nodes
    dpserver.check_dpserver_versions(client_mgr, current_nodes + added_nodes)


def _simbaos_expand(info: E6000Expand):
    client_mgr = info.client_mgr
    current_nodes = info.current_nodes
    added_nodes = info.added_nodes
    simbaos_package = os.path.basename(config.simbaos.package)
    sbos.expand(client_mgr, added_nodes, current_nodes, simbaos_package)


def _dataprocet_expand(info: E6000Expand):
    client_mgr = info.client_mgr
    added_nodes = info.added_nodes
    image_name = os.path.basename(config.dataprotect.image)
    chart_name = os.path.basename(config.dataprotect.chart)
    dp.expand(client_mgr, added_nodes, image_name, chart_name)


@e6000.command("expand")
@click.option("--nodes", help="node names separated by comma")
def expand(nodes=None):

    task_list = [
        _expand_check,
        _get_expand_info,
        _expand_converged_eds_file_service,
        _check_dpserver_versions,
        _simbaos_expand,
        _dataprocet_expand,
    ]
    if nodes is not None:
        nodes = nodes.split(",")
    else:
        nodes = config.expanded_node_names
    expand_info = E6000Expand(client_mgr, config, nodes)
    total_step_nums = len(task_list)
    for step, func in enumerate(task_list):
        re, info = smart_response(func, step, total_step_nums, info=expand_info)
        if not re:
            if info.added_nodes is not None:
                rollback_expand(info.added_nodes, info.pool_id)
            break


@e6000.command("delete_node")
@click.option("--node_name", required=True, help="node to be removed")
def delete_node(node_name):
    cli = client_mgr.get_primary_fsm_client()

    try:
        dpserver.check_dpserver_versions(client_mgr, config.get_nodes())
        simbaos_status = cli.simbaos_get_status()
        if not simbaos_status["status"] == "deployed":
            log.warning(f"SimbaOS is not deployed, skip delete {node_name}")
            exit(0)

        simbaos_node = [n for n in simbaos_status["nodes"] if n["name"] == node_name]
        if len(simbaos_node) == 0:
            log.warning(f"Node {node_name} is not in cluster")

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
        pacific.scale_down_converged_eds_file_service(pcli, pool_id, [deleted_node.management_internal_ip])

        # 5. umount nfs at deleted nodes
        dpserver.umount(client_mgr, [deleted_node])
    except Exception as e:
        log.error(f"Failed to delete node from cluster, {e}")
        exit(1)


@e6000.group()
def namespace():
    pass


@namespace.command("create")
@click.argument("namespace_name")
@click.option("--account_id", required=True, help="account id")
@click.option("--pool_id", required=True)
def namespace_create(namespace_name, account_id, pool_id):
    try:
        cli = client_mgr.get_primary_fsm_client()
        ns.namespace_create(cli, namespace_name, account_id, pool_id)
        log.info("Successfully created namespace")
    except Exception as e:
        log.error(f"Failed to create name space: {e}")
        exit(1)


@namespace.command("get")
@click.argument("namespace_name")
@click.option("--account_id", required=True)
def namespace_get(namespace_name, account_id):
    try:
        cli = client_mgr.get_primary_fsm_client()
        ns_id = ns.namespace_get(cli, namespace_name, account_id)
        if ns_id is None:
            log.info(f"Namespace {namespace_name} not found")
        else:
            log.info(f"Successfully got namespace, namespace_id: {ns_id}")
    except Exception as e:
        log.error(f"Failed to get namespace: {e}")
        exit(1)


@namespace.command("delete")
@click.argument("namespace_name")
@click.option("--account_id", required=True, help="account id")
def namespace_delete(namespace_name, account_id):
    try:
        cli = client_mgr.get_primary_fsm_client()
        ns.namespace_delete(cli, namespace_name, account_id)
    except Exception as e:
        log.error(f"Failed to delete namespace {namespace_name}: {e}")
        exit(1)


@e6000.group()
def package():
    pass


@e6000.group()
def simbaos():
    pass


@simbaos.command("install")
def simbaos_install():
    try:
        config.check_for_completeness(check_simbaos=True)
        sbos.install(config, client_mgr, config.get_nodes())
    except Exception as e:
        log.error(f"Failed to install simbaos, {e}")
        exit(1)


@simbaos.command("reset")
def simbaos_reset():
    try:
        sbos.reset(client_mgr, config.get_nodes())
    except Exception as e:
        log.error(f"Failed to reset simbaos, {e}")
        exit(1)


@e6000.group()
def dataprotect():
    pass


@dataprotect.command("install")
@click.option("--skip_image_load", is_flag=True, help="跳过加载镜像步骤")
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
            skip_image_load=skip_image_load,
        )
    except Exception as e:
        log.error(f"Failed to install dataprotect, {e}")
        exit(1)


@dataprotect.command("reset")
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
        log.error(f"Failed to reset dataprotect, {e}")
        exit(1)


class E6000UpgradeIns(UpgradeTrait):
    def __init__(self, upgrade_struct: UpgradeStruct):
        super().__init__(upgrade_struct)
        super().get_config()

    def get_config(self):
        super().get_config()
        log.info(f"Start to upgrade at node {[n.name for n in self.nodes]}")

    def upload_packages(self):
        if not self.upgrade_struct.skip_package_upload:
            upload_packages(self.client_mgr.get_primary_fsm_client(), self.rollback_kwargs)

    def upgrade_simbaos(self):
        if self.upgrade_struct.skip_upgrade_simbaos:
            return
        nodes = self.client_mgr.pacific_client.get_servers()
        sbos.run_preupgrade_paraller(self.client_mgr, nodes, self.simbaos_package_name, cert_type="pacific")
        sbos.upgrade(self.client_mgr, cert_type="pacific")
        sbos.run_postupgrade_paraller(self.client_mgr, nodes)

    def upgrade_dataprotect(self):
        dp.upgrade(self.client_mgr, self.nodes, self.dataprotect_image_name, self.dataprotect_chart_name)


class CtxE6000:
    obj: E6000UpgradeIns


@e6000.group("upgrade")
@click.pass_context
@click.option("-sysadmin", "--sysadmin", required=True)
@click.option("-password", "--password", required=True)
def e6000_upgrade_group(ctx: CtxE6000, sysadmin: str, password: str):
    global config
    global client_mgr
    ctx.obj = E6000UpgradeIns(UpgradeStruct(client_mgr=client_mgr, config=config, sysadmin=sysadmin, password=password))


@e6000_upgrade_group.command("upload_packages")
@click.pass_context
def e6000_upgrade_upload_packages(ctx: CtxE6000):
    ctx.obj.upload_packages()


@e6000_upgrade_group.command("dataprotect")
@click.pass_context
def e6000_upgrade_dataprotect(ctx: CtxE6000):
    ctx.obj.upgrade_dataprotect()


@e6000_upgrade_group.command("simbaos")
@click.pass_context
def e6000_upgrade_simbaos(ctx: CtxE6000):
    ctx.obj.upgrade_simbaos()


if __name__ == "__main__":
    main()
