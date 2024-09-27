from dataclasses import dataclass

from dataprotect_deployment.dp_server_interface import DataProtectDeployClient


class Node:
    def __init__(self, management_ip, internal_ip, simbaos_role,  **kwargs):
        management_ip: str
        internal_ip: str
        simbaos_role: str
        for key, value in kwargs:
            setattr(self, key, value)

    def get_priminary_node(self):
        pass

    def init_depolyclient(self):
        pass


