import logging as log
from typing import List, Tuple
from concurrent.futures import ThreadPoolExecutor
from dataprotect_deployment.dp_server_interface import DataProtectDeployClient, UpgradeT
from dataprotect_deployment.client_manager import ClientManager
from config import PacificNode, DataBackupConfig
from config import Config
from pydantic import BaseModel

def run_preinstall_parallel(
        client_mgr: ClientManager,
        nodes,
        image_package_name: str,
        upgrade_flag: bool = False
):
    node_names = [node for node in nodes] if upgrade_flag else [node.name for node in nodes]
    log.info(f'Start to preinstall dataprotect at nodes: {node_names}')
    with ThreadPoolExecutor() as executor:
        for node in nodes:
            node_ip = node if upgrade_flag else node.management_internal_ip
            cli = client_mgr.get_dataprotect_deployment_client(node_ip)
            executor.submit(cli.dataprotect_preinstall, image_package_name, 'd7')
    log.info(f'Successfully preinstalled dataprotect at nodes {node_names}')


def get_pm_pe_replicas(primary_client: DataProtectDeployClient) -> Tuple[int, int]:
    simbaos_status = primary_client.simbaos_get_status()
    if simbaos_status['status'] != 'deployed':
        raise Exception('Simbaos is not deployed')

    nodes = simbaos_status['nodes']
    master_cnt = len([n for n in nodes if n['role'] == 'master'])
    workers_cnt = len([n for n in nodes if n['role'] == 'worker'])
    pe_replicas = master_cnt + workers_cnt
    return master_cnt, pe_replicas


def get_pm_pe_replicas_from_nodes(nodes_cnt) -> Tuple[int, int]:
    if nodes_cnt <= 3:
        return 3, nodes_cnt
    if nodes_cnt <= 16:
        return 4, nodes_cnt
    if nodes_cnt <= 32:
        return 5, nodes_cnt


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

    pm_replicas, pe_replicas = get_pm_pe_replicas_from_nodes(len(client_mgr.config.get_nodes()))
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

    pm_replicas, pe_replicas = get_pm_pe_replicas_from_nodes(len(client_mgr.config.get_nodes()))
    log.info('Start to expand dataprotect cluster, '
             f'pm_replicas={pm_replicas}, '
             f'pe_replicas={pe_replicas}')

    run_preinstall_parallel(client_mgr, added_nodes, image_package_name)
    dataprotect_expand_dict = {"chart_package_name": chart_package_name, "master_replicas": pm_replicas,
                               "worker_replicas": pe_replicas, "device_type": "e6000"}
    cli.dataprotect_expand(**dataprotect_expand_dict)
    log.info("Successfully expanded dataprotect cluster")

def upgrade(
    client_mgr: ClientManager,
    added_nodes: List[PacificNode],
    image_package_name: str,
    chart_package_name: str,
):
    cli = client_mgr.get_primary_fsm_client()
    _, nodes = client_mgr.config.get_upgrade_nodes(client_mgr.config.user, client_mgr.config.password)
    master_replicas, worker_replicas = get_pm_pe_replicas_from_nodes(len(nodes))
    log.info('Start to expand dataprotect cluster, '
             f'pm_replicas={master_replicas}, '
             f'pe_replicas={worker_replicas}')
    upgrade_flag = True
    run_preinstall_parallel(client_mgr, added_nodes, image_package_name, upgrade_flag)
    dataprotect_expand_dict = {"chart_package_name": chart_package_name, "master_replicas": master_replicas,
                               "worker_replicas": worker_replicas, "device_type": "e6000"}
    cli.dataprotect_upgrade(**dataprotect_expand_dict)
    log.info("Successfully upgrade dataprotect cluster")

def perform_upgrade_pre_checks(client_mgr: ClientManager, sys: str, pwd: str):
    log.info("Start pre upgrade check")
    cli = client_mgr.get_primary_fsm_client()
    ip = cli.address.split(":", 1)[0]
    body_data = UpgradeT(ip = ip, user_name=sys, password=pwd)
    cli.perform_pre_upgrade_checks(body_data)
    log.info("Succefully perform upgrade checks")

def perform_upgrade_databackup(client_mgr: ClientManager, sys: str, pwd: str):
    log.info("Start upgrade databackup")
    cli = client_mgr.get_primary_fsm_client()
    body_data = UpgradeT(ip = cli.address, user_name=sys, password=pwd)
    cli.perform_upgrade_backup(body_data)
    log.info("Successfully upgrade databackup")

def perform_upgrade_post_checks(client_mgr: ClientManager, sys: str, pwd: str):
    log.info("Start post upgrade check")
    cli = client_mgr.get_primary_fsm_client()
    body_data = UpgradeT(ip = cli.address, user_name=sys, password=pwd)
    cli.perform_post_upgrade_checks(body_data)
    log.info("Succefully perform post upgrade checks")

def delete_node(
        cli: DataProtectDeployClient,
        node_name: str,
        simbaos_status,
        chart_package_name: str,
):
    log.info("Start to update dataprotect's arguments")
    nodes_cnt = len(simbaos_status['nodes']) - 1
    pm_replicas, pe_replicas = get_pm_pe_replicas_from_nodes(nodes_cnt)
    log.info(f'Start to update dataprotect cluster, '
             f'pm_replicas={pm_replicas}, '
             f'pe_replicas={pe_replicas}')

    dataprotect_expand_dict = {"chart_package_name": chart_package_name, "master_replicas": pm_replicas,
                               "worker_replicas": pe_replicas, "device_type": "e6000"}
    cli.dataprotect_expand(**dataprotect_expand_dict)
    log.info("Successfully updated dataprotect's arguments")


class DataProtectManager:
    config: DataBackupConfig

    def __init__(self, config: DataBackupConfig):
        self.config = config
        self.priliminary_client = self.config.preliminary_node.dpclient

    def preinstall(self, node_list, image_package_name):
        log.info(f"Start load dataprotect images")
        with ThreadPoolExecutor() as executor:
            result = []
            for node in node_list:
                ip = node.ip
                client = self.config.nodes[ip].dpclient
                r = executor.submit(client.dataprotect_preinstall, image_package_name, 'd8')
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
    def upgrade_dataprotect(self, dataprotect_upgrade_dict):
        log.info(f"Start upgrade dataprotect")
        self.priliminary_client.dataprotect_upgrade(**dataprotect_upgrade_dict)
        log.info(f"Successfully upgrae dataprotect")

    def perform_upgrade_pre_checks(self, upgradeData: UpgradeT):
        log.info("Start pre upgrade check")
        self.priliminary_client.perform_pre_upgrade_checks(upgradeData)
        log.info("Succefully perform upgrade checks")

    def perform_upgrade_backup(self, upgradeData: UpgradeT):
        log.info("Start upgrade databackup")
        self.priliminary_client.perform_upgrade_backup(upgradeData)
        log.info("Successfully upgrade databackup")

    def perform_upgrade_post_checks(self, upgradeData: UpgradeT):
        log.info("Start post upgrade check")
        self.priliminary_client.perform_post_upgrade_checks(upgradeData)
        log.info("Succefully perform post upgrade checks")