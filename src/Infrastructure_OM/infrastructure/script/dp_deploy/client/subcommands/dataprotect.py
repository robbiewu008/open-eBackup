import logging as log
from typing import List
from concurrent.futures import ThreadPoolExecutor
from dataprotect_deployment.dp_server_interface import DataProtectDeployClient
from dataprotect_deployment.client_manager import ClientManager
from config import PacificNode
from config import DataBackupConfig


def run_preinstall_parallel(
        client_mgr: ClientManager,
        nodes: List[PacificNode],
        image_package_name: str,
):
    node_names = [node.name for node in nodes]
    log.info(f'Start to preinstall dataprotect at nodes: {node_names}')
    with ThreadPoolExecutor() as executor:
        for node in nodes:
            cli = client_mgr.get_dataprotect_deployment_client(
                node.management_internal_ip)
            executor.submit(cli.dataprotect_preinstall, image_package_name)
    log.info(f'Successfully preinstalled dataprotect at nodes {node_names}')


def get_pm_pe_replicas(primary_client) -> (int, int):
    simbaos_status = primary_client.simbaos_get_status()
    if simbaos_status['status'] != 'deployed':
        raise Exception('Simbaos is not deployed')

    nodes = simbaos_status['nodes']
    master_cnt = len([n for n in nodes if n['role'] == 'master'])
    workers_cnt = len([n for n in nodes if n['role'] == 'worker'])
    pe_replicas = master_cnt + workers_cnt
    return master_cnt, pe_replicas


def install(
        client_mgr: ClientManager,
        chart_package_name,
        image_package_name,
        skip_image_load: bool,
):
    master_cli = client_mgr.get_primary_fsm_client()
    if skip_image_load:
        log.info('Skip loading dataprotect image')
    else:
        run_preinstall_parallel(
            client_mgr,
            client_mgr.config.get_nodes(),
            image_package_name
        )

    pm_replicas, pe_replicas = get_pm_pe_replicas(client_mgr.get_primary_fsm_client())
    log.info(f'Start to install dataprotect cluster, '
             f'pm_replicas={pm_replicas}, '
             f'pe_replicas={pe_replicas}')

    master_cli.dataprotect_install(
        chart_package_name,
        pm_replicas,
        pe_replicas,
        device_type="e6000"
    )
    log.info('Successfully installed dataprotect')


def reset(master_cli: DataProtectDeployClient):
    log.info('Start to reset dataprotect cluster')
    master_cli.dataprotect_reset()
    log.info('Successfully reset dataprotect cluster')


def expand(
        client_mgr: ClientManager,
        added_nodes: List[PacificNode],
        image_package_name: str,
        chart_package_name: str,
):
    """扩容备份软件
    1. 在added_nodes上运行preinstall加载镜像
    2. 根据simbaos集群状态扩容备份软件
    """
    cli = client_mgr.get_primary_fsm_client()

    pm_replicas, pe_replicas = get_pm_pe_replicas(client_mgr.get_primary_fsm_client())
    log.info('Start to expand dataprotect cluster, '
             f'pm_replicas={pm_replicas}, '
             f'pe_replicas={pe_replicas}')

    run_preinstall_parallel(client_mgr, added_nodes, image_package_name)
    dataprotect_expand_dict = {"chart_package_name": chart_package_name, "master_replicas": pm_replicas,
                               "worker_replicas": pe_replicas, "device_type": "e6000"}
    cli.dataprotect_expand(**dataprotect_expand_dict)
    log.info("Successfully expanded dataprotect cluster")


def delete_node(
        cli: DataProtectDeployClient,
        node_name: str,
        simbaos_status,
        chart_package_name: str,
):
    log.info("Start to update dataprotect's arguments")
    simbaos_nodes = [
        n for n in simbaos_status['nodes']
        if n['name'] != node_name
    ]
    master_replicas = len([n for n in simbaos_nodes if n['role'] == 'master'])
    workers_cnt = len([n for n in simbaos_nodes if n['role'] == 'worker'])
    worker_replicas = master_replicas + workers_cnt
    log.info(f'Start to update dataprotect cluster, '
             f'pm_replicas={master_replicas}, '
             f'pe_replicas={worker_replicas}')

    dataprotect_expand_dict = {"chart_package_name": chart_package_name, "master_replicas": master_replicas,
                               "worker_replicas": worker_replicas, "device_type": "e6000"}
    cli.dataprotect_expand(**dataprotect_expand_dict)
    log.info("Successfully updated dataprotect's arguments")


class DataProtectManager:
    config: DataBackupConfig

    def __init__(self, config):
        self.config = config
        self.priliminary_client = self.config.preliminary_node.dpclient

    def preinstall(self, node_list, image_package_name):
        log.info(f"Start load dataprotect images")
        with ThreadPoolExecutor() as executor:
            result = []
            for node in node_list:
                ip = node.ip
                client = self.config.nodes[ip].dpclient
                r = executor.submit(client.dataprotect_preinstall, image_package_name)
                result.append(r)
            for r in result:
                r.result()
        log.info(f"Successfully load dataprotect images")

    def install(self, chart_package_name):
        log.info(f"Start install dataprotect")
        pm_replicas, pe_replicas = get_pm_pe_replicas(self.priliminary_client)
        self.priliminary_client.dataprotect_install(chart_package_name,
                                                    pm_replicas,
                                                    pe_replicas,
                                                    self.config.device_type)
        log.info(f"Successfully install dataprotect")

    def pre_expand(self, nodes):
        log.info(f"Start pre expand dataprotect")
        for node in nodes:
            self.priliminary_client.sync_cert(node.ip, node.user, node.passwd)
        log.info(f"Successfully pre expand dataprotect")

    def expand(self, **kwargs):
        log.info(f"Start expand dataprotect")
        self.priliminary_client.dataprotect_expand(**kwargs)
        log.info(f"Successfully expand dataprotect")

    def reset(self):
        log.info(f"Start reset dataprotect")
        self.priliminary_client.dataprotect_reset()
        log.info(f"Successfully reset dataprotect")
