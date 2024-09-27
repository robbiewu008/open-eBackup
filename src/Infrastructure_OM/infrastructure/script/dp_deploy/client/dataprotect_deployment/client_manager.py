from typing import Dict, List
from concurrent.futures import ThreadPoolExecutor
from dataprotect_deployment.dp_server_interface import DataProtectDeployClient
from dataprotect_deployment.pacific_interface import PacificClient

from config import Config, PacificNode, DataBackupConfig


class ClientManager:
    dp_clients: Dict[str, DataProtectDeployClient]
    pacific_client: PacificClient
    primary_fsm_client: DataProtectDeployClient
    expand_dp_clients: Dict[str, DataProtectDeployClient]
    config: [Config, DataBackupConfig]

    def __init__(self, config: [Config, DataBackupConfig]):
        self.config = config
        device_type = self.config.device_type
        self.dp_clients: Dict[str, DataProtectDeployClient] = dict()
        if device_type == "E6000":
            self.pacific_client = PacificClient(self.config)

            token = self.pacific_client.token
            for host in self.config.get_nodes():
                mgr_ip = host.management_internal_ip
                self.dp_clients[mgr_ip] = DataProtectDeployClient(address=mgr_ip, token=token)

            (primary_node_address, _) = self.pacific_client.get_primary_fsm()
            self.primary_fsm_client = self.dp_clients[primary_node_address]
        elif device_type == "DataBackup":
            for node in self.config.nodes:
                self.dp_clients[node.ip] = DataProtectDeployClient(
                    self.config, node.ip, user=node.user, passwd=node.passwd
                )

            self.primary_fsm_client = self.dp_clients[self.config.preliminary_node.ip]

    def get_primary_fsm_client(self) -> DataProtectDeployClient:
        return self.primary_fsm_client

    def get_dataprotect_deployment_client(
        self, address: str
    ) -> DataProtectDeployClient:
        return self.dp_clients[address]

    def get_pacific_client(self) -> PacificClient:
        return self.pacific_client


def run_in_parallel(
    client_mgr: ClientManager,
    nodes: List[PacificNode],
    func, *args, **kwargs
):
    """
    Run func(args, kwargs) in parallel, return result list
    'func': must be method of class DataProtectDeployClient
    """
    with ThreadPoolExecutor() as executor:
        results = []
        for n in nodes:
            cli = client_mgr.get_dataprotect_deployment_client(
                n.management_internal_ip)
            r = executor.submit(func, cli, *args, **kwargs)
            results.append(r)
    return [(n.name, r.result()) for (n, r) in zip(nodes, results)]