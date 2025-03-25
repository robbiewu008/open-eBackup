from typing import Dict, List, Union
from concurrent.futures import ThreadPoolExecutor
from dataprotect_deployment.dp_server_interface import DataProtectDeployClient
from dataprotect_deployment.pacific_interface import PacificClient

from config import Config, PacificNode, DataBackupConfig, get_auth_token, log


class ClientManager:
    dp_clients: Dict[str, DataProtectDeployClient]
    pacific_client: PacificClient
    primary_fsm_client: DataProtectDeployClient
    expand_dp_clients: Dict[str, DataProtectDeployClient]
    config: Union[Config, DataBackupConfig]

    def __init__(self, config: Union[Config, DataBackupConfig], upgrade=False):
        self.config = config
        device_type = self.config.device_type
        self.dp_clients: Dict[str, DataProtectDeployClient] = dict()
        if device_type == "E6000":
            if not upgrade:
                self.pacific_client = PacificClient(self.config)
            if upgrade:
                primary_node_address = self.config.float_ip
            else:
                (primary_node_address, _) = self.pacific_client.get_primary_fsm()
            if upgrade:
                sys_admin_name = self.config.user
                sys_admin_pwd = self.config.password
                self.dp_clients[primary_node_address] = DataProtectDeployClient(
                    address=primary_node_address, op_admin_user=sys_admin_name, op_admin_pwd=sys_admin_pwd
                )
                simbaos_info = self.dp_clients[primary_node_address].simbaos_get_status()
                get_upgrade_nodes, nodes_info = self.config.get_upgrade_nodes(sys_admin_name, sys_admin_pwd)
                if not get_upgrade_nodes:
                    err_msg = f"Fail to get the node info, {nodes_info}"
                    raise Exception(err_msg)
                for host in nodes_info:
                    nodes_name_list = [node.get("name") for node in simbaos_info.get("nodes")]
                    if host.get("name") in nodes_name_list:
                        mgr_ip = host.get("manageIp")
                        self.dp_clients[mgr_ip] = DataProtectDeployClient(
                            address=mgr_ip, op_admin_user=sys_admin_name, op_admin_pwd=sys_admin_pwd
                        )
                self.primary_fsm_client = self.dp_clients[primary_node_address]
                return
            else:
                token = self.pacific_client.token
            for host in self.config.get_nodes():
                mgr_ip = host.management_internal_ip
                self.dp_clients[mgr_ip] = DataProtectDeployClient(address=mgr_ip, token=token)

            self.primary_fsm_client = self.dp_clients[primary_node_address]
        elif device_type == "DataBackup":
            for node in self.config.nodes:
                self.dp_clients[node.ip] = DataProtectDeployClient(
                    self.config, node.ip, user=node.user, passwd=node.passwd
                )

            self.primary_fsm_client = self.dp_clients[self.config.preliminary_node.ip]

    def get_primary_fsm_client(self) -> DataProtectDeployClient:
        return self.primary_fsm_client

    def get_dataprotect_deployment_client(self, address: str) -> DataProtectDeployClient:
        return self.dp_clients[address]

    def get_pacific_client(self) -> PacificClient:
        return self.pacific_client


def run_in_parallel(client_mgr: ClientManager, nodes: List[PacificNode], func, *args, **kwargs):
    """
    Run func(args, kwargs) in parallel, return result list
    'func': must be method of class DataProtectDeployClient
    """
    with ThreadPoolExecutor() as executor:
        results = []
        for n in nodes:
            cli = client_mgr.get_dataprotect_deployment_client(n.management_internal_ip)
            r = executor.submit(func, cli, *args, **kwargs)
            results.append(r)
    return [(n.name, r.result()) for (n, r) in zip(nodes, results)]
