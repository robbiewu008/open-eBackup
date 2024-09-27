from jinja2 import Template
from typing import List, Union, Dict
from dataclasses import dataclass, asdict
from enum import Enum
import ipaddress
import yaml
import getpass

from dataprotect_deployment.dp_server_interface import DataProtectDeployClient


class StoragePoolConfig:
    def __init__(self, **kwargs):
        self.parity_units = kwargs.get('parity_units')
        self.fault_tolerance = kwargs.get('fault_tolerance')
        self.replica_num = kwargs.get('replica_num')


@dataclass
class SimbaOSConfig:
    package: str
    control_plane_endpoint: str
    kube_pods_cidr: str
    kube_service_cidr: str


@dataclass
class DataProtectConfig:
    chart: str
    image: str


class PacificNode:
    def __init__(self):
        self.name = None
        self.role: [str] = None
        self.management_internal_ip = None
        self.storage_frontend_ip = None
        self.storage_frontend_port_name = None
        self.storage_ctrl_network = None
        self.storage_frontend_port_name = None

    def is_management(self) -> bool:
        return 'management' in self.role

    def is_storage(self) -> bool:
        return 'storage' in self.role


@dataclass
class CommonNodeConfig:
    node_name: str
    ip: str
    simbaos_ip: str
    internal_address: str
    user: str
    passwd: str
    management_interface: str
    dpclient: DataProtectDeployClient
    kadmin: str
    kadmin_passwd: str
    role: str = "worker"


def is_valid_ip(*args) -> bool:
    try:
        for ip in args:
            ipaddress.ip_address(ip)
        return True
    except Exception:
        return False


@dataclass
class DataBackupConfig:
    preliminary_node: CommonNodeConfig
    k8sVIP: str
    k8sServiceVIP: str
    simbaos_package_path: str
    chart_package_path: str
    image_package_path: str
    kube_pods_cidr: str
    kube_service_cidr: str
    expand_nodes: List[CommonNodeConfig] = None
    device_type: str = "DataBackup"
    nodes: Dict[str, CommonNodeConfig] = None

    @staticmethod
    def _gen_common_node_config(node):
        ip = node["ip"]
        password = node["passwd"]
        dpclient = DataProtectDeployClient(ip, user=node["user"], passwd=password)
        node_name = dpclient.gethostname()
        internal_ip = node["internal_address"]
        management_interface = dpclient.get_interface_from_ip(internal_ip)

        return CommonNodeConfig(**node,
                                dpclient=dpclient,
                                node_name=node_name,
                                management_interface=management_interface
                                )

    def __post_init__(self):
        self.nodes = {}
        if self.preliminary_node is not None:
            self.preliminary_node = self._gen_common_node_config(self.preliminary_node)
            self.nodes[self.preliminary_node.ip] = self.preliminary_node

        if self.expand_nodes is not None:
            for i, node in enumerate(self.expand_nodes):
                self.expand_nodes[i] = self._gen_common_node_config(self.expand_nodes[i])
                self.nodes[self.expand_nodes[i].ip] = self.expand_nodes[i]

    def get_all_node_list(self):
        return list(self.nodes.values())

    def get_expand_node_list(self):
        return self.expand_nodes

    def check_ip(self):
        if not is_valid_ip(self.preliminary_node.ip):
            raise Exception(f"Invalid preliminary node ip: {self.preliminary_node.ip}")
        if not is_valid_ip(self.preliminary_node.internal_address):
            raise Exception(f"Invalid preliminary node internal ip: {self.preliminary_node.internal_address}")
        if not is_valid_ip(self.k8sVIP):
            raise Exception(f"Invalid k8s vip ip: {self.k8sVIP}")
        if not is_valid_ip(self.k8sServiceVIP):
            raise Exception(f"Invalid k8s service ip: {self.k8sServiceVIP}")
        if self.expand_nodes is not None:
            for node in self.expand_nodes:
                if not is_valid_ip(node.ip):
                    raise Exception(f"Invalid node ip: {node.ip}")
                if not is_valid_ip(node.internal_address):
                    raise Exception(f"Invalid node internal ip: {node.internal_address}")

    def validate(self):
        self.check_ip()


@dataclass
class Config:
    float_ip: str
    user: str
    simbaos: SimbaOSConfig = None
    dataprotect: DataProtectConfig = None
    storage_pool: StoragePoolConfig = None
    password: str = None
    device_type = str = "E6000"

    def __post_init__(self):
        self.password = getpass.getpass(prompt="Please enter device manager password: ")
        if self.storage_pool is not None:
            self.storage_pool = StoragePoolConfig(**self.storage_pool)
        if self.simbaos is not None:
            self.simbaos = SimbaOSConfig(**self.simbaos)
        if self.dataprotect is not None:
            self.dataprotect = DataProtectConfig(**self.dataprotect)

    def validate(self):
        if not is_valid_ip(self.float_ip):
            raise Exception('Invalid float_ip')
        if self.simbaos is not None and \
                not is_valid_ip(self.simbaos.control_plane_endpoint):
            raise Exception('Invalid simbaos control_plane_endpoint ip')
        if self.storage_pool.replica_num is not None and \
                self.storage_pool.fault_tolerance is not None and \
                self.storage_pool.parity_units is not None:
            raise Exception('Invalid storage pool configuration. '
                            'If redundancy policy is replication, set replica_num parameter. '
                            'If redundancy policy is EC, set parity_units and fault_tolerance  parameter.')

    def set_nodes(self, nodes: [PacificNode]):
        self.nodes = nodes

    def get_nodes(self) -> [PacificNode]:
        return self.nodes

    def get_node(self, name) -> PacificNode:
        node = [n for n in self.nodes if n.name == name]
        if len(node) == 0:
            return None
        return node[0]

    def get_nodes_name(self) -> [str]:
        return [n.name for n in self.nodes]

    def check_for_completeness(
            self,
            check_all=False,
            check_simbaos=False,
            check_dataprotect=False,
            check_storage_pool=False,
    ):
        if (check_all or check_simbaos) and self.simbaos is None:
            raise Exception('SimbaOS configuration is missing')
        if (check_all or check_dataprotect) and self.dataprotect is None:
            raise Exception('DataProtect configuration is missing')
        if (check_all or check_storage_pool) and self.storage_pool is None:
            raise Exception('Storage pool configuration is missing')


class SimbaOSRole(Enum):
    Master = 1
    Worker = 2


@dataclass
class SimbaOSNode:
    name: str
    address: str
    internal_address: str
    service_address: str
    interface: str
    management_address: str
    role: str

    @staticmethod
    def from_pacific_node(pnode: PacificNode, role: SimbaOSRole):
        if pnode.storage_ctrl_network is not None:
            address, interface = (
                pnode.storage_ctrl_network,
                pnode.storage_ctrl_network_port_name
            )
        else:
            address, interface = (
                pnode.storage_frontend_ip,
                pnode.storage_frontend_port_name
            )

        return SimbaOSNode(
            name=pnode.name,
            address=address,
            internal_address=address,
            service_address=address,
            interface=interface,
            management_address=pnode.management_internal_ip,
            role='master' if role == SimbaOSRole.Master else 'worker'
        )

    def set_role(self, role: SimbaOSRole):
        self.role = 'master' if role == SimbaOSRole.Master else 'worker'


@dataclass
class SimbaOSInstallConfig:
    hosts: [SimbaOSNode]
    master0: str
    masters: [str]
    workers: [str]
    control_plane_endpoint: str
    service_plane_endpoint: str
    service_plane_endpoint_v6: Union[str, None]
    kube_pods_cidr: str
    kube_service_cidr: str


@dataclass
class SimbaOSExpandConfig:
    master: SimbaOSNode
    hosts: List[SimbaOSNode]


class SimbaOSConfigGenerator:
    INSTALL_TEMPLATE_FILE = 'template/simbaos/install.yaml.template'
    EXPAND_TEMPLATE_FILE = 'template/simbaos/expand.yaml.template'

    @staticmethod
    def _read_file(file_name: str):
        with open(file_name, 'r') as f:
            return f.read()

    @staticmethod
    def _validate_install_config(config: SimbaOSInstallConfig):
        for host in config.hosts:
            if host.address == config.control_plane_endpoint or \
                    host.management_address == config.control_plane_endpoint:
                raise Exception(f"simbaos's control_plane_endpoint \
                        {config.control_plane_endpoint} already in use. \
                        please choose another ip")

    @classmethod
    def generate_expand_config(
            cls,
            master: SimbaOSNode,
            added_nodes: List[SimbaOSNode]
    ) -> str:
        content = cls._read_file(cls.EXPAND_TEMPLATE_FILE)
        t = Template(content)
        return t.render(master=master, hosts=added_nodes)

    @classmethod
    def generate_install_config(
            cls,
            float_ip: str,
            float_ipv6: Union[str, None],
            masters: [SimbaOSNode],
            workers: [SimbaOSNode],
            master0: str,
            simbaos_config: SimbaOSConfig
    ) -> str:
        install_config = SimbaOSInstallConfig(
            hosts=masters + workers,
            master0=master0,
            masters=[n.name for n in masters if n.name != master0],
            workers=[n.name for n in workers],
            control_plane_endpoint=simbaos_config.control_plane_endpoint,
            service_plane_endpoint=float_ip,
            service_plane_endpoint_v6=float_ipv6,
            kube_pods_cidr=simbaos_config.kube_pods_cidr,
            kube_service_cidr=simbaos_config.kube_service_cidr
        )

        cls._validate_install_config(install_config)
        content = cls._read_file(cls.INSTALL_TEMPLATE_FILE)
        t = Template(content)
        return t.render(config=install_config)

    @classmethod
    def load_install_config(cls, install_config_content):
        cfg = yaml.safe_load(install_config_content)
        spec = cfg['spec']
        simbaos_install_config = SimbaOSInstallConfig(
            hosts=[
                SimbaOSNode(
                    name=host['name'],
                    address=host['address'],
                    internal_address=host['internalAddress'],
                    service_address=host['serviceAddress'],
                    interface=host['interface'],
                    management_address=host['managementAddress'],
                    role='worker'
                )
                for host in cfg['spec']['hosts']
            ],
            master0=spec['roleGroups']['master0'],
            masters=spec['roleGroups']['master'],
            workers=spec['roleGroups']['worker'],
            control_plane_endpoint=spec['controlPlaneEndpoint']['address'],
            service_plane_endpoint=spec['servicePlaneEndpoint']['address'],
            service_plane_endpoint_v6=spec['servicePlaneEndpoint'].get('addressV6'),
            kube_pods_cidr=spec['network']['kubePodsCIDR'],
            kube_service_cidr=spec['network']['kubeServiceCIDR'],
        )
        return simbaos_install_config

    @classmethod
    def load_expand_config(cls, expand_config_content):
        cfg = yaml.safe_load(expand_config_content)
        master = cfg['spec']['master']
        simbaos_expand_config = SimbaOSExpandConfig(
            master=SimbaOSNode(
                name=master['name'],
                address=master['address'],
                internal_address=master['internalAddress'],
                service_address=None,
                interface=None,
                management_address=None,
                role='master'
            ),
            hosts=[
                SimbaOSNode(
                    name=host['name'],
                    address=host['address'],
                    internal_address=host['internalAddress'],
                    service_address=host['serviceAddress'],
                    interface=host['interface'],
                    management_address=host['managementAddress'],
                    role=host['role']
                )
                for host in cfg['spec']['hosts']
            ],
        )
        return simbaos_expand_config
