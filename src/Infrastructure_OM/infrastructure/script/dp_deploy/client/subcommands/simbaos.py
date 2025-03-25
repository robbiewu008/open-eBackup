import os
import base64
from typing import List
import logging as log
from jinja2 import Template

from dataprotect_deployment.client_manager import ClientManager
from config import Config, SimbaOSConfigGenerator, PacificNode, SimbaOSNode
from config import SimbaOSRole, DataBackupConfig
from concurrent.futures import ThreadPoolExecutor
from client_exception import TooManyNodesToExpandException


def get_status(client_mgr: ClientManager):
    cli = client_mgr.get_primary_fsm_client()
    return cli.simbaos_get_status()


def install(
        config: Config,
        client_mgr: ClientManager,
        nodes: List[PacificNode]
):
    master_cli = client_mgr.get_primary_fsm_client()
    pcli = client_mgr.pacific_client

    simbaos_package_name = os.path.basename(config.simbaos.package)
    # preinstall simbaos at all nodes
    run_preinstall_parallel(client_mgr, nodes, simbaos_package_name)

    master0 = master_cli.gethostname()
    float_ipv6 = pcli.get_secondary_float_ip()
    float_ipv6 = float_ipv6 if float_ipv6 != '' else None

    excepted_master_cnt = get_expected_simbaos_master_cnt(nodes)
    masters = select_new_simabos_masters(excepted_master_cnt, nodes)
    workers = [n.name for n in nodes if n.name not in masters]

    simbaos_nodes = [
        SimbaOSNode.from_pacific_node(n, SimbaOSRole.Master)
        for n in nodes
    ]
    master_simbaos_nodes = [n for n in simbaos_nodes if n.name in masters]
    worker_simbaos_nodes = [n for n in simbaos_nodes if n.name in workers]

    cluster_config = SimbaOSConfigGenerator.generate_install_config(
        config.float_ip,
        float_ipv6,
        master_simbaos_nodes,
        worker_simbaos_nodes,
        master0,
        config.simbaos
    )
    base64_encoded_cluster_config = base64.b64encode(
        bytes(cluster_config, encoding='utf-8')
    ).decode()

    # install simbaos at master node
    log.info('Start to install simbaos at master, '
             f'master nodes is {masters} '
             f'worker nodes is {workers}')
    master_cli.simbaos_install(base64_encoded_cluster_config, "e6000")
    log.info("Successfully installed simbaos")


def reset(client_mgr: ClientManager, nodes: List[PacificNode]):
    node_names = [n.name for n in nodes]
    log.info(f'start reset simbaos at {node_names}')
    with ThreadPoolExecutor() as executor:
        result = []
        for n in nodes:
            cli = client_mgr.get_dataprotect_deployment_client(
                n.management_internal_ip)
            r = executor.submit(cli.simbaos_reset)
            result.append(r)
    for idx, r in enumerate(result):
        try:
            r.result()
        except Exception:
            node_name = nodes[idx].name
            log.warning(f'Unable to reset simbaos at {node_name}')
    log.info('Successfully reset simbaos')


def run_preinstall_parallel(
        client_mgr: ClientManager,
        nodes: List[PacificNode],
        simbaos_package_name: str
):
    node_names = [node.name for node in nodes]
    log.info(f'Start to preinstall simbaos at nodes: {node_names}')
    with ThreadPoolExecutor() as executor:
        result = []
        for node in nodes:
            cli = client_mgr.get_dataprotect_deployment_client(
                node.management_internal_ip)
            r = executor.submit(
                cli.simbaos_preinstall,
                simbaos_package_name,
                node.storage_ctrl_network or node.storage_frontend_ip,
                "e6000"
            )
            result.append(r)
        for r in result:
            r.result()
    log.info(f'Successfully preinstalled simbaos at nodes {node_names}')


def run_preupgrade_paraller(
    client_mgr: ClientManager,
    nodes: List[PacificNode],
    simbaos_package_name: str,
    cert_type: str
):
    node_names = [node.name for node in nodes]
    log.info(f'Start to pre upgrade simbaos')
    with ThreadPoolExecutor() as executor:
        result = []
        for node in nodes:
            cli = client_mgr.get_dataprotect_deployment_client(
                node.management_internal_ip)
            r = executor.submit(
                cli.simbaos_preupgrade,
                simbaos_package_name,
                node.storage_ctrl_network or node.storage_frontend_ip,
                cert_type
            )
            result.append(r)
        for r in result:
            r.result()
    log.info(f'Successfully pre upgrade simbaos at nodes {node_names}')


def run_postupgrade_paraller(
        client_mgr: ClientManager,
        nodes: List[PacificNode],
):
    node_names = [node.name for node in nodes]
    log.info(f'Start to post upgrade simbaos')
    with ThreadPoolExecutor() as executor:
        result = []
        for node in nodes:
            cli = client_mgr.get_dataprotect_deployment_client(
                node.management_internal_ip)
            r = executor.submit(
                cli.simbaos_postupgrade,
            )
            result.append(r)
        for r in result:
            r.result()
    log.info(f'Successfully post upgrade simbaos at nodes {node_names}')


def upgrade(client_mgr: ClientManager, cert_type: str):
    log.info(f"Start to upgrade simbaos")
    cli = client_mgr.get_primary_fsm_client()
    log.info(f"Successfully upgrade simbaos")
    return cli.simbaos_upgrade(cert_type)


def get_expected_simbaos_master_cnt(servers: List[PacificNode]) -> int:
    server_cnt = len(servers)
    if server_cnt <= 16:
        expected_simbaos_master_cnt = 3
    elif server_cnt <= 32:
        expected_simbaos_master_cnt = 5
    else:
        raise TooManyNodesToExpandException
    return expected_simbaos_master_cnt


def select_new_simabos_masters(
        master_cnt: int,
        added_servers: List[PacificNode]
) -> List[str]:
    # select master
    added_pacifc_management = [
        s.name for s in added_servers if s.is_management()
    ]
    new_simbaos_masters = added_pacifc_management
    master_cnt -= len(new_simbaos_masters)

    idx = 0
    while master_cnt > 0 and idx < len(added_servers):
        s = added_servers[idx]
        if s.is_storage() and s.name not in added_pacifc_management:
            new_simbaos_masters.append(s.name)
            master_cnt -= 1
        idx += 1
    return new_simbaos_masters


def expand(
        client_mgr: ClientManager,
        added_servers: List[PacificNode],
        current_servers: List[PacificNode],
        simbaos_package_name: str
):
    """
    Expand simbaos cluster
    """

    cli = client_mgr.get_primary_fsm_client()

    master = [s for s in current_servers if s.name == cli.gethostname()][0]
    master = SimbaOSNode.from_pacific_node(master, SimbaOSRole.Master)

    if len(added_servers) == 0:
        log.warn('No new nodes added, ignore expand simabos')
        return

    # preinstall simabos at all added nodes
    run_preinstall_parallel(client_mgr, added_servers, simbaos_package_name)

    simbaos_status = cli.simbaos_get_status()
    cur_simbaos_master_cnt = len([
        s for s in simbaos_status['nodes'] if s['role'] == 'master'
    ])
    expected_simbaos_master_cnt = get_expected_simbaos_master_cnt(
        current_servers + added_servers
    )
    new_simbaos_masters = select_new_simabos_masters(
        expected_simbaos_master_cnt - cur_simbaos_master_cnt,
        added_servers
    )

    added_simbaos_nodes = [
        SimbaOSNode.from_pacific_node(n, SimbaOSRole.Worker)
        for n in added_servers
    ]
    for s in added_simbaos_nodes:
        if s.name in new_simbaos_masters:
            s.set_role(SimbaOSRole.Master)
    expand_config = SimbaOSConfigGenerator.generate_expand_config(
        master,
        added_simbaos_nodes
    )

    base64_encoded_expand_config = base64.b64encode(
        bytes(expand_config, encoding='utf-8')).decode()

    log.info('Start to expand simbaos cluster')
    cli.simbaos_expand(base64_encoded_expand_config)
    log.info('Successfully expanded simbaos cluster')


def delete_node(
        client_mgr: ClientManager,
        node: PacificNode
):
    log.info(f'Start to delete node {node.name}')
    cli = client_mgr.get_primary_fsm_client()
    cli.simbaos_delete_node(node.name)
    log.info(f'Successfully deleted node {node.name}')


def gen_config(template, **kwargs):
    with open(template, "r") as f:
        context = f.read()
    t = Template(context)
    return t.render(**kwargs)


def get_base64_encoded_config(template, **kwargs):
    config = gen_config(template, **kwargs)
    return base64.b64encode(bytes(config, encoding='utf-8')).decode()


class SimbaosManager:
    def __init__(self, config: DataBackupConfig, template_dict, **kwargs):
        self.config = config
        self.template_dict = template_dict
        self.priliminary_client = self.config.preliminary_node.dpclient

        for key, value in kwargs.items():
            setattr(self, key, value)

    def preisntall(self, simbaos_package_name, node_list):
        log.info(f"Start preinstall simbaos")
        with ThreadPoolExecutor() as executor:
            result = []
            for node in node_list:
                client = node.dpclient
                r = executor.submit(client.simbaos_preinstall, simbaos_package_name, node.internal_address,
                                    self.config.device_type)
                result.append(r)
            for r in result:
                r.result()
        log.info(f"Successfully preinstall simbaos")

    def install(self, **kwargs):
        log.info(f"Start install simbaos")
        install_template = self.template_dict["install"]
        base64_install_config = get_base64_encoded_config(install_template, **kwargs)
        self.priliminary_client.simbaos_install(base64_install_config, self.config.device_type)
        log.info('Successfully install simbaos')

    def reset(self, node_list):
        log.info(f"Start reset simbaos on {node_list}")
        node_list = list(node_list)
        with ThreadPoolExecutor() as executor:
            result = []
            for node in node_list:
                cli = self.config.nodes[node.ip].dpclient
                r = executor.submit(cli.simbaos_reset)
                result.append(r)
        for idx, r in enumerate(result):
            try:
                r.result()
            except Exception:
                node_name = node_list[idx].name
                log.warning(f'Unable to reset simbaos at {node_name}')
        log.info('Successfully reset simbaos')

    def expand(self, **kwargs):
        log.info(f"Start expand simbaos")
        expand_template = self.template_dict["expand"]
        base64_expand_config = get_base64_encoded_config(expand_template, **kwargs)
        self.priliminary_client.simbaos_expand(base64_expand_config)
        log.info(f"Successfully expand simbaos")

    def delete_node(self, node_list):
        log.info(f"Start delete simbaos nodes, {node_list}")
        for node in node_list:
            self.priliminary_client.simbaos_delete_node(node.node_name)
        log.info(f"Successfully delete simbaos nodes")

    def get_status(self):
        status = self.priliminary_client.simbaos_get_status()
        return status

    def get_dataprotect_status(self):
        status = self.priliminary_client.dataprotect_status()
        return status
    
    def simbaos_pre_upgrade(self, cert_type: str):
        log.info(f"Start to upgrade simbaos")
        self.priliminary_client.simbaos_preupgrade(self.config.simbaos_package_path, self.config.preliminary_node.ip, cert_type)
        log.info(f"Successfully upgrade simbaos")
    
    def simbaos_upgrade(self, cert_type: str):
        log.info(f"Start to upgrade simbaos")
        self.priliminary_client.simbaos_upgrade(cert_type)
        log.info(f"Successfully upgrade simbaos")

    def simbaos_post_upgrade(self):
        log.info(f"Start to upgrade simbaos")
        self.priliminary_client.simbaos_postupgrade(self.config.simbaos_package_path, self.config.preliminary_node.ip)
        log.info(f"Successfully upgrade simbaos")