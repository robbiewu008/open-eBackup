import base64
import mock
import sys
from urllib.parse import urlparse
from click.testing import CliRunner
from typing import Dict, List
import json
import logging as log
import unittest
import os
import shutil

sys.path.append('./client')
import consts
from dpclient import main as main_prog
from config import SimbaOSInstallConfig, SimbaOSConfigGenerator, SimbaOSExpandConfig

from mock_config import xlsx_e6000_expected_content, mock_password,\
                        mock_dataprotect_image_name, mock_simbaos_package_name,\
                        mock_dataprotect_chart_name, mock_config_file_name

mock_primary_fsm = '1.1.1.1'
mock_get_primary_fsm_response = {
    'result': 0,
    'nodeIpPrimaryExter': mock_primary_fsm,
    'nodeNamePrimary': 'node1',
}
mock_simbaos_package_name = 'SimbaOS-v1.3.0-release-aarch64.tar.gz'
mock_dataprotect_chart_name = 'OceanProtect_DataProtect_1.6.RC1_chart_ARM_64.tgz'
mock_dataprotect_image_name = 'OceanProtect_DataProtect_1.6.RC1_image_ARM_64.tgz'
mock_upgrade_dpserver_package = 'dpserver-version2.tgz'

mock_float_ip = '1.2.3.7'
mock_float_ipv6 = '44:00:4d:65:37:5c'
mock_esn = 'mock_esn'

mock_dpserver_version1 = 'version1'
mock_dpserver_version2 = 'version2'
mock_upgrade_dpserver_package = 'dpserver-version2.tgz'

mock_config = """
float_ip: 1.2.3.7
user: admin
storage_pool:
  parity_units: 4
  fault_tolerance: 1
simbaos:
  package: {}
  control_plane_endpoint: 1.2.3.8
  kube_pods_cidr: 10.233.64.0/18
  kube_service_cidr: 10.233.0.0/18
dataprotect:
  chart: {}
  image: {}
"""

class MockStoragePool:
    def __init__(self, id, poolPara, server_list):
        self.id = id
        self.poolPara = poolPara
        self.server_list = server_list


class MockPacificNode:
    def __init__(self, name, role, ip_address_list, media):
        self.name = name
        self.role = role
        self.ip_address_list = ip_address_list
        self.media = media
        self.file_service = -1

    def get_management_ip(self):
        for ip in self.ip_address_list:
            if 'management_internal' in ip['ip_usage']:
                return ip['ip_address']


def generate_mock_pacific_node(ids, role=['storage']):
    servers = []
    for id in ids:
        servers.append(MockPacificNode(
            name=f'node{id}',
            role=role,
            ip_address_list=[
                {'ip_address': f'1.1.1.{id}', 'ip_usage': [
                    'management_internal'], 'port_name': 'eth0'},
                {'ip_address': f'2.1.1.{id}', 'ip_usage': [
                    'storage_frontend'], 'port_name': 'eth1'},
            ],
            media=[
                {"phyDevEsn": "1", "phySlotId": 1, "mediaType": "ssd_card",
                    'mediaRole': 'no_use'},
                {"phyDevEsn": "3", "phySlotId": 2, "mediaType": "ssd_card",
                    'mediaRole': 'system_disk'},
                {"phyDevEsn": "2", "phySlotId": 2, "mediaType": "sata_disk",
                    'mediaRole': 'no_use'},
            ]
        ))
    return servers


global_id = 0


def next_id():
    global global_id
    global_id += 1
    return global_id


class MockNamespace:
    def __init__(self, id, name):
        self.id = id
        self.name = name
        self.nfs_share_id = None
        self.nfs_share_client_id = None


class MockSimbaOSCluster:

    class MockSimbaOSNode:
        def __init__(self, name):
            self.name = name
            self.preinstalled = False
            self.installed = False
            self.role = None

        def __str__(self):
            return f'MockSimbaOSNode(name={self.name}, ' \
                f'preinstalled={self.preinstalled}, ' \
                f'installed={self.installed}, ' \
                f'role={self.role})'

        def __repr__(self):
            return self.__str__()

    def __init__(self):
        self.nodes: Dict[str, self.MockSimbaOSNode] = dict()
        self.deployed = False
        self.version = False

    def register(self, name):
        assert self.nodes.get(name) is None
        self.nodes[name] = self.MockSimbaOSNode(name)

    def unregister(self, name):
        assert self.nodes.get(name) is not None
        self.nodes.pop(name)

    def preinstall(self, name):
        assert not self.nodes.get(name).installed
        self.nodes[name].preinstalled = True

    def reset(self, name):
        self.nodes[name].installed = False
        self.nodes[name].preinstalled = False

    def install(self, version, master0, masters, workers):
        assert self.nodes[master0].preinstalled
        self.nodes[master0].installed = True
        self.nodes[master0].role = 'master'
        if masters is not None:
            for m in masters:
                assert self.nodes[m].preinstalled
                self.nodes[m].installed = True
                self.nodes[m].role = 'master'
        if workers is not None:
            for w in workers:
                assert self.nodes[m].preinstalled
                self.nodes[m].installed = True
                self.nodes[w].role = 'worker'
        self.deployed = True
        self.version = version

    def expand(self, masters, workers):
        for n in masters:
            node = self.nodes.get(n)
            assert node.preinstalled
            node.installed = True
            node.role = 'master'
        for n in workers:
            node = self.nodes.get(n)
            assert node.preinstalled
            node.installed = True
            node.role = 'worker'

    def delete_node(self, node):
        assert not self.nodes.get(node).preinstalled
        assert not self.nodes.get(node).installed


class MockDataProtectCluster:

    class MockDataProtectNode:
        def __init__(self):
            self.image_loaded = False

    def __init__(self):
        self.nodes: Dict[str, self.MockDataProtectNode] = dict()
        self.deployed = False
        self.pm_replicas = 0
        self.pe_replicas = 0

    def register(self, name):
        assert self.nodes.get(name) is None
        self.nodes[name] = self.MockDataProtectNode()

    def unregister(self, name):
        assert self.nodes.get(name) is not None
        self.nodes.pop(name)

    def preinstall(self, name):
        self.nodes[name].image_loaded = True

    def reset(self):
        self.deployed = False
        self.pm_replicas = 0
        self.pe_replicas = 0

    def install(self, pm_replicas, pe_replicas):
        assert all(node.image_loaded for node in self.nodes.values())
        assert pe_replicas == len(self.nodes)
        self.pm_replicas = pm_replicas
        self.pe_replicas = pe_replicas
        self.deployed = True

    def expand(self, pm_replicas, pe_replicas):
        self.pm_replicas = pm_replicas
        self.pe_replicas = pe_replicas


class MockDpServer:

    simbaos_cluster = MockSimbaOSCluster()
    dataprotect_cluster = MockDataProtectCluster()
    uploaded_packages = []

    def __init__(self, hostname):
        self.hostname = hostname
        self.namespaces: Dict[int, MockNamespace] = dict()
        self.nfs_umounted = False
        self.simbaos_cluster.register(hostname)
        self.dataprotect_cluster.register(hostname)
        self.dpserver_version = mock_dpserver_version1

    def unregister(self):
        self.simbaos_cluster.unregister(self.hostname)
        self.dataprotect_cluster.unregister(self.hostname)

    def handle_requests(self, method: str, path: str, json, **kwarg):

        if json is not None:
            ns = json.get('namespace_name')
            nid = json.get('namespace_id')
            aid = json.get('account_id')
            pid = json.get('pool_id')
            nfs_id = json.get('nfs_id')

        if path == '/v1/dp_deploy/converaged_service/hostname':
            return self.gethostname()
        if path == '/v1/dp_deploy/converaged_service/namespace':
            if method == 'GET':
                return self.get_namespace(ns, aid)
            elif method == 'POST':
                return self.create_namespace(ns, aid, pid)
            elif method == 'DELETE':
                return self.delete_namespace(ns, aid)
        if path == "/v1/dp_deploy/converaged_service/nfs_share":
            if method == 'GET':
                return self.get_nfs_share(nid, aid)
            elif method == 'POST':
                return self.create_nfs_share(aid, ns, nid)
            elif method == 'DELETE':
                return self.delete_nfs_share(nfs_id, aid)
        if path == '/v1/dp_deploy/converaged_service/umount':
            return self.umount_nfs_share()
        if path == '/v1/dp_deploy/converaged_service/client':
            if method == 'GET':
                return self.get_nfs_client(nfs_id, aid)
            elif method == 'POST':
                return self.create_nfs_client(json['client_name'], nfs_id, aid)
            elif method == 'DELETE':
                return self.delete_nfs_client(aid, json['client_id'])
        if path == '/v1/dp_deploy/package/upload':
            filename = kwarg['data'].fields['file'][0]
            return self.upload_package(filename)
        if path == '/v1/dp_deploy/package/unpack_package':
            filename = json['package_name']
            return self.unpack_package(filename)
        if path.startswith('/v1/dp_deploy/package/delete/'):
            return self.delete_package(path.split('/')[-1])
        if path == '/v1/dp_deploy/dpserver/version':
            return {'version': self.dpserver_version}
        if path == '/v1/dp_deploy/dpserver/upgrade':
            return self.dpserver_upgrade(json['upgrade_package_name'])
        if path == '/v1/dp_deploy/app/simbaos/status':
            return self.simbaos_get_status()
        if path == '/v1/dp_deploy/app/simbaos/preinstall':
            return self.simbaos_preinstall(json['package_name'])
        if path == '/v1/dp_deploy/app/simbaos/install':
            install_config_content = base64.b64decode(
                json['base64_encoded_config'])
            install_config = SimbaOSConfigGenerator.load_install_config(
                install_config_content)
            return self.simbaos_install(install_config)
        if path == '/v1/dp_deploy/app/simbaos/expand':
            expand_config_content = base64.b64decode(
                json['base64_encoded_config'])
            expand_config = SimbaOSConfigGenerator.load_expand_config(
                expand_config_content)
            return self.simbaos_expand(expand_config)
        if path == '/v1/dp_deploy/app/simbaos/reset':
            return self.simbaos_reset()
        if path == '/v1/dp_deploy/app/simbaos/delete_node':
            return self.simbaos_cluster.delete_node(json['node_name'])
        if path == '/v1/dp_deploy/app/dataprotect/status':
            assert False
        if path == '/v1/dp_deploy/app/dataprotect/preinstall':
            return self.dataprotect_cluster.preinstall(self.hostname)
        if path == '/v1/dp_deploy/app/dataprotect/install':
            return self.dataprotect_cluster.install(
                json['pm_replicas'], json['pe_replicas'])
        if path == '/v1/dp_deploy/app/dataprotect/reset':
            return self.dataprotect_cluster.reset()
        if path == '/v1/dp_deploy/app/dataprotect/expand':
            return self.dataprotect_cluster.expand(
                json['master_replicas'], json['worker_replicas'])

        raise Exception(f'Unknow path {path}')

    def gethostname(self):
        return {'hostname': self.hostname}

    def create_namespace(self, namespace_name, account_id, pool_id):
        for ns in self.namespaces.values():
            assert ns.name != namespace_name
        id = next_id()
        self.namespaces[id] = MockNamespace(id, namespace_name)
        return {"namespace_id": id}

    def get_namespace(self, namespace_name, account_id):
        for ns in self.namespaces.values():
            if ns.name == namespace_name:
                return {'namespace_id': ns.id}
        return {'namespace_id': None}

    def delete_namespace(self, namespace_name, account_id):
        for ns in self.namespaces.values():
            if ns.name == namespace_name:
                self.namespaces.pop(ns.id)
                return
        assert False

    def create_nfs_share(self, account_id, namespace_name, namespace_id):
        ns = self.namespaces.get(namespace_id)
        assert ns is not None
        assert ns.name == namespace_name
        if ns.nfs_share_id is None:
            ns.nfs_share_id = next_id()
        return {'nfs_id': ns.nfs_share_id}

    def get_nfs_share(self, namespace_id, account_id):
        assert self.namespaces.get(namespace_id) is not None
        return {'nfs_id': self.namespaces[namespace_id].nfs_share_id}

    def delete_nfs_share(self, nfs_id, account_id):
        for ns in self.namespaces.values():
            if ns.nfs_share_id == nfs_id:
                self.namespaces[ns.id].nfs_share_id = None
                return

    def umount_nfs_share(self):
        self.nfs_umounted = True

    def create_nfs_client(self, client_name, nfs_id, account_id):
        assert client_name == '127.0.0.1'
        for ns in self.namespaces.values():
            if ns.nfs_share_id == nfs_id:
                assert ns.nfs_share_client_id is None
                id = next_id()
                self.namespaces[ns.id].nfs_share_client_id = id
                return {'nfs_client_id': id}
        assert False  # unreachable

    def get_nfs_client(self, nfs_id, account_id):
        for ns in self.namespaces.values():
            if ns.nfs_share_id == nfs_id:
                return {'nfs_client_id': ns.nfs_share_client_id}
        assert False  # unreachable

    def delete_nfs_client(self, account_id, client_id):
        for ns in self.namespaces.values():
            if ns.nfs_share_client_id == client_id:
                self.namespaces[ns.id].nfs_share_client_id = None

    def upload_package(self, filename):
        self.uploaded_packages.append(filename)

    def unpack_package(self, filename):
        if filename not in self.uploaded_packages:
            return {'status': 'not exist'}
        else:
            return {'status': 'exist'}

    def delete_package(self, filename):
        self.uploaded_packages.remove(filename)

    def simbaos_get_status(self):
        if self.simbaos_cluster.deployed:
            nodes = [n for n in self.simbaos_cluster.nodes.values()]
            return {
                'status': 'deployed',
                'version': self.simbaos_cluster.version,
                'nodes': [{'name': n.name, 'role': n.role}
                          for n in nodes if n.installed]}
        return {'status': 'not found'}

    def simbaos_preinstall(self, package_name):
        self.simbaos_cluster.preinstall(self.hostname)

    def simbaos_install(self, cfg: SimbaOSInstallConfig):
        assert cfg.master0 == self.hostname
        self.simbaos_cluster.install(
            "mock_version", cfg.master0, cfg.masters, cfg.workers
        )

    def simbaos_expand(self, cfg: SimbaOSExpandConfig):
        assert cfg.master.name == self.hostname
        masters, workers = [], []
        for host in cfg.hosts:
            if host.role == 'master':
                masters.append(host.name)
            else:
                workers.append(host.name)
        self.simbaos_cluster.expand(masters, workers)

    def simbaos_reset(self):
        self.simbaos_cluster.reset(self.hostname)
        for n in self.simbaos_cluster.nodes.values():
            if n.installed:
                return
        self.simbaos_cluster.deployed = False

    def dpserver_upgrade(self, package_name):
        assert mock_upgrade_dpserver_package in self.uploaded_packages
        assert package_name == mock_upgrade_dpserver_package
        self.dpserver_version = mock_dpserver_version2


class MockPacificService:
    def __init__(self, nodes: List[MockPacificNode], pool: MockStoragePool = None):
        self.pool = pool
        self.account_id = None
        self.tasks = []
        self.nodes = nodes

    def _new_task(self):
        if len(self.tasks) > 0 and not self.tasks[-1]:
            raise Exception("Last pacific task is running")
        self.tasks.append(False)
        return len(self.tasks)-1

    def _expand_nodes(self, nodes: List[MockPacificNode]):
        self.nodes.extend(nodes)

    def _delete_node(self, node):
        for (i, n) in enumerate(self.nodes):
            if n.name == node:
                self.nodes.pop(i)
                return
        assert False

    def handle_requests(self, method, path, data, **kwarg):
        if path == '/api/v2/aa/sessions':
            return self.login(data['user_name'], data['password'])
        if path == '/api/v2/network_service/servers':
            return self.get_servers()
        if path == '/dsware/service/v2/sec/keepAlive':
            return
        if path == '/api/v2/common/esn':
            return self.ok(esn=mock_esn)
        if path == '/api/v2/console/secondary_float_ip':
            return self.ok(secondary_external_service_float_ip=mock_float_ipv6)
        if path == '/dsware/service/upgrade/getFsmNodeInfo':
            return self.get_primary_fsm()
        if path == '/dsware/service/vsan/scanServerMedia':
            return self.scan_server_media()
        if path == '/dsware/service/cluster/storagepool/queryStoragePool':
            return self.query_storage_pool()
        if path == '/dsware/service/cluster/storagepool/createStoragePool':
            return self.create_storage_pool(data['poolPara'], data['serverList'])
        if path == '/dsware/service/cluster/storagepool/deleteStoragePool':
            return self.delete_storage_pool(data['poolId'])
        if path == '/api/v2/cluster_service/converged_eds_service':
            if method == 'GET':
                return self.query_converged_eds_file_service()
            elif method == 'POST':
                if data['action'] == 0:
                    return self.create_converged_eds_file_service(
                        action=data['action'],
                        storage_pool_id=data['storage_pool_id'],
                        service_type_list=data['service_type_list']
                    )
                elif data['action'] == 2:
                    return self.expand_converged_eds_file_service(
                        action=data['action'],
                        storage_pool_id=data['storage_pool_id'],
                        server_ip_list=data['server_ip_list']
                    )
                elif data['action'] == 3:
                    self.scale_down_converged_eds_file_service(
                        action=data['action'],
                        storage_pool_id=data['storage_pool_id'],
                        server_ip_list=data['server_ip_list']
                    )
            elif method == 'DELETE':
                return self.delete_converged_eds_file_service(
                    data['storage_pool_id'])
        if path == '/api/v2/cluster_service/converged_eds_node':
            if data['action'] == 2:
                return self.expand_converged_eds_file_service(
                    action=data['action'],
                    storage_pool_id=data['storage_pool_id'],
                    server_ip_list=data['server_ip_list']
                )
            elif data['action'] == 3:
                return self.scale_down_converged_eds_file_service(
                    action=data['action'],
                    storage_pool_id=data['storage_pool_id'],
                    server_ip_list=data['server_ip_list']
                )
        if path.startswith('/api/v2/account/accounts'):
            if method == 'GET':
                return self.query_account(consts.STORAGE_ACCOUNT_NAME)
            elif method == 'POST':
                return self.create_account(data['name'])
        if path == '/dsware/service/task/querySingleTaskInfo':
            return self.query_task_info(kwarg['params']['taskId'])
        raise Exception(f'Unknow path {path}')

    def login(self, user_name, password):
        assert user_name == 'admin'
        assert password == mock_password
        return self.ok(x_auth_token='mock_token')

    def get_servers(self):
        data = [node.__dict__ for node in self.nodes]
        return self.ok(data=data)

    def get_primary_fsm(self):
        return mock_get_primary_fsm_response

    def get_esn(self):
        return self.ok(esn=mock_esn)

    def get_secondary_float_ip(self) -> str:
        return self.ok(secondary_external_service_float_ip=mock_float_ipv6)

    def scan_server_media(self):
        media = {}
        for node in self.nodes:
            media[node.get_management_ip()] = node.media
        return {'result': 0, 'servers': media}

    def query_storage_pool(self):
        pools = [{
            'poolName': self.pool.poolPara['poolName'],
            'poolId': self.pool.id,
        }] if self.pool is not None else []
        return {'result': 0, 'storagePools': pools}

    def create_storage_pool(self, poolPara, server_list):
        assert self.pool is None
        self.pool = MockStoragePool(0, poolPara, server_list)
        return self.ok_v2(taskId=self._new_task())

    def delete_storage_pool(self, pool_id: str):
        assert self.pool is not None
        assert self.pool.id == pool_id
        self.pool = None
        return self.ok_v2(taskId=self._new_task())

    def query_converged_eds_file_service(self):
        services_on_pool_nodes = []
        for node in self.nodes:
            services_on_pool_nodes.append({
                node.get_management_ip(): {
                    'file_service': node.file_service
                }
            })
        return self.ok(service_of_pools=[{
            'services_on_pool_nodes': services_on_pool_nodes
        }])

    def delete_converged_eds_file_service(self, storage_pool_id):
        assert self.pool is not None
        assert storage_pool_id == self.pool.id
        for node in self.nodes:
            node.file_service = -1
        return self.ok(taskId=self._new_task())

    def create_converged_eds_file_service(self, action, storage_pool_id, service_type_list):
        assert action == 0
        assert service_type_list == [2]
        assert self.pool is not None and storage_pool_id == self.pool.id
        for node in self.nodes:
            assert node.file_service == -1
            node.file_service = 0
        return self.ok(taskId=self._new_task())

    def expand_converged_eds_file_service(self, action, storage_pool_id, server_ip_list):
        assert action == 2
        assert self.pool is not None and storage_pool_id == self.pool.id
        cnt = 0
        for node in self.nodes:
            if node.get_management_ip() in server_ip_list:
                assert node.file_service == -1
                node.file_service = 0
                cnt += 1
        assert cnt == len(server_ip_list)
        return self.ok(taskId=self._new_task())

    def scale_down_converged_eds_file_service(self, action, storage_pool_id, server_ip_list):
        assert action == 3
        assert self.pool is not None and storage_pool_id == self.pool.id
        cnt = 0
        for node in self.nodes:
            if node.get_management_ip() in server_ip_list:
                assert node.file_service == 0
                node.file_service = -1
                cnt += 1
        assert cnt == len(server_ip_list)
        return self.ok(taskId=self._new_task())

    def query_account(self, name):
        assert name == consts.STORAGE_ACCOUNT_NAME
        if self.account_id is None:
            return self.ok(code=1800000404)
        return self.ok(id=self.account_id)

    def create_account(self, name):
        assert name == consts.STORAGE_ACCOUNT_NAME
        assert self.account_id is None
        self.account_id = 0
        return self.ok(id=self.account_id)

    def query_task_info(self, task_id):
        assert task_id == len(self.tasks) - 1
        assert len(self.tasks) <= 1 or self.tasks[task_id-1]
        self.tasks[task_id] = True
        return self.ok_v2(taskInfo={'taskStatus': 'success', 'progress': 100})

    def ok(self, code=0, data=None, **kwargs):
        if data is None:
            return {'result': {'code': code}, 'data': {**kwargs}}
        return {'result': {'code': code}, 'data': data}

    def ok_v2(self, code=0, **kwarg):
        return {'result': {'code': code}, **kwarg}


class MockCluster:
    def __init__(self, pacific_nodes: List[MockPacificNode], float_ip):
        self.float_ip = float_ip
        self.dpserves: Dict[str, MockDpServer] = {
            node.get_management_ip(): MockDpServer(node.name)
            for node in pacific_nodes
        }
        self.pacific = MockPacificService(pacific_nodes)

    def handle_requests(self, address, method, path, json, **kwarg):
        if address == self.float_ip:
            return self.pacific.handle_requests(method, path, json, **kwarg)
        return self.dpserves[address]\
            .handle_requests(method, path, json, **kwarg)

    def expand_pacific_nodes(self, nodes: List[MockPacificNode]):
        for n in nodes:
            self.dpserves[n.get_management_ip()] = MockDpServer(n.name)
        self.pacific._expand_nodes(nodes)


mock_pacific_cluster = []
mock_pacific_cluster.extend(generate_mock_pacific_node(
    ids=[0, 1],
    role=['management', 'storage']
))
mock_pacific_cluster.extend(generate_mock_pacific_node(
    ids=[2],
    role=['storage']
))
mock_cluster = MockCluster(mock_pacific_cluster, mock_float_ip)


class MockRequestSession:

    class MockResponse:
        def __init__(self, json_data, status_code=200):
            self.text = json.dumps(json_data)
            self.json_data = json_data
            self.status_code = status_code

        def json(self):
            return self.json_data

    def __init__(self):
        self.cluster = mock_cluster
        self.headers = {}
        self.verify = True

    def _handle(self, method, url, json, **kwarg):
        url = urlparse(url)
        assert url.scheme == 'https'
        resp = self.cluster.handle_requests(
            url.hostname, method, url.path, json=json, **kwarg)
        return self.MockResponse(resp)

    def get(self, url, json=None, **kwarg):
        return self._handle('GET', url, json, **kwarg)

    def post(self, url, json=None, **kwarg):
        return self._handle('POST', url, json, **kwarg)

    def delete(self, url, json=None, **kwarg):
        return self._handle('DELETE', url, json, **kwarg)


def mock_getpass(*args, **kwarg):
    return mock_password

def mock_e6000_read_xlsx(path, simbaos_package=None, \
                         chart_package=None, image_package=None):
    return xlsx_e6000_expected_content, mock_password

class DataprotectDeploymentClientTest(unittest.TestCase):

    def setup_temp_filesystem(self, tempdir="./tmp"):
        if os.path.exists(tempdir):
            shutil.rmtree(tempdir)  # Remove it if it already exists (clean start)
        os.makedirs(tempdir, exist_ok=True)
        shutil.copytree('./client/template', os.path.join(tempdir, 'template'))

        self._new_empty_files(
            os.path.join(tempdir, mock_simbaos_package_name),
            os.path.join(tempdir, mock_dataprotect_chart_name),
            os.path.join(tempdir, mock_dataprotect_image_name),
            os.path.join(tempdir, mock_upgrade_dpserver_package)
        )
        with open(os.path.join(tempdir, 'config.yaml'), 'w') as config_file:
            config_file.write(mock_config.format(
                os.path.join(tempdir, mock_simbaos_package_name),
                os.path.join(tempdir, mock_dataprotect_chart_name),
                os.path.join(tempdir, mock_dataprotect_image_name)
            ))
        os.chdir(tempdir)

    def _new_empty_files(self, *paths):
        for path in paths:
            with open(path, 'w'):
                pass

    def setUp(self):
        self.nodes_cnt = 3
        self.setup_temp_filesystem()

    def check_cluster_ok(self, mock_cluster):
        # check storage pool created
        self.assertIsNotNone(mock_cluster.pacific.pool)

        # check eds file service created
        for node in mock_cluster.pacific.nodes:
            self.assertEqual(node.file_service, 0)

        primary_dp = mock_cluster.dpserves[mock_primary_fsm]
        dp_cluster = MockDpServer.dataprotect_cluster
        simbaos_cluster = MockDpServer.simbaos_cluster

        # check all namespaces created
        all_ns = consts.BASE_NFS_LIST + consts.OP_NFS_LIST
        created_ns = [ns.name for ns in primary_dp.namespaces.values()]
        assert sorted(all_ns) == sorted(created_ns)

        # check packages uploaded
        all_packaged = [
            mock_simbaos_package_name,
            mock_dataprotect_chart_name,
            mock_dataprotect_image_name
        ]
        assert sorted(primary_dp.uploaded_packages) == sorted(all_packaged)

        assert mock_simbaos_package_name in primary_dp.uploaded_packages
        assert mock_dataprotect_chart_name in primary_dp.uploaded_packages

        # check simbaos installed
        for node in simbaos_cluster.nodes.values():
            self.assertTrue(node.installed)

        # check dataprotect installed
        for node in mock_cluster.dpserves.values():
            assert node.dataprotect_cluster.deployed

        # check dataprotect pm_replicas and pe_replicas are correct
        node_cnt = len(mock_pacific_cluster)
        master_cnt = dp_cluster.pm_replicas
        worker_cnt = dp_cluster.pe_replicas
        self.assertTrue(node_cnt >= 3)
        self.assertTrue(node_cnt <= 32)
        if node_cnt <= 3:
            self.assertTrue(master_cnt == 3)
        elif node_cnt <= 16:
            self.assertTrue(master_cnt == 4)
        elif node_cnt <= 32:
            self.assertTrue(master_cnt == 5)
        self.assertTrue(worker_cnt == node_cnt)

    def check_cluster_rest(self, mock_cluster):
        primary_dp = mock_cluster.dpserves[mock_primary_fsm]
        dp_cluster = MockDpServer.dataprotect_cluster
        simbaos_cluster = MockDpServer.simbaos_cluster

        # check dataprotect reset
        assert not dp_cluster.deployed
        # check simbaos reset
        for node in simbaos_cluster.nodes.values():
            self.assertFalse(node.installed)
            self.assertFalse(node.preinstalled)
        self.assertFalse(simbaos_cluster.deployed)
        # check packages deleted
        assert len(primary_dp.uploaded_packages) == 0
        # check namespaces deleted
        assert len(primary_dp.namespaces) == 0
        # check eds file services deleted
        for node in mock_cluster.pacific.nodes:
            assert node.file_service == -1
        # check nfs umounted
        assert primary_dp.nfs_umounted
        # check storage pool deleted
        assert mock_cluster.pacific.pool is None

    def install_cluster(self):
        runner = CliRunner()
        args = ['e6000', '-f', 'config.yaml', 'install', '--no_rollback']
        result = runner.invoke(main_prog, args)
        assert result.exit_code == 0
        self.check_cluster_ok(mock_cluster)
    
    def install_cluster_xlsx(self):
        runner = CliRunner()
        args = ['e6000', '-f', mock_config_file_name, "-P1", mock_simbaos_package_name, \
                "-P2", mock_dataprotect_chart_name, "-P3", mock_dataprotect_image_name, 'install', '--no_rollback']
        result = runner.invoke(main_prog, args)
        assert result.exit_code == 0
        self.check_cluster_ok(mock_cluster)


    def reset_cluster(self):
        runner = CliRunner()
        args = ["e6000"]
        args += ['-f', 'config.yaml', 'reset']
        result = runner.invoke(main_prog, args)
        assert result.exit_code == 0
        self.check_cluster_rest(mock_cluster)

    def expand_cluster(self, total_num):
        assert total_num > self.nodes_cnt
        nodes = generate_mock_pacific_node(
            ids=[i + self.nodes_cnt for i in range(0, total_num-self.nodes_cnt)])
        self.nodes_cnt = total_num
        mock_cluster.expand_pacific_nodes(nodes)
        runner = CliRunner()
        node_names = ','.join([n.name for n in nodes])
        args = ["e6000"]
        args += ['-f', 'config.yaml', 'expand', '--nodes', node_names]
        result = runner.invoke(main_prog, args)
        assert result.exit_code == 0
        self.check_cluster_ok(mock_cluster)

    def expand_cluster_xlsx(self, total_num):
        assert total_num > self.nodes_cnt
        nodes = generate_mock_pacific_node(
            ids=[i + self.nodes_cnt for i in range(0, total_num-self.nodes_cnt)])
        self.nodes_cnt = total_num
        mock_cluster.expand_pacific_nodes(nodes)
        xlsx_e6000_expected_content["expanded_node_names"] = [n.name for n in nodes]
        runner = CliRunner()
        node_names = ','.join([n.name for n in nodes])
        args = ['e6000', '-f', mock_config_file_name, "-P1", mock_simbaos_package_name, \
                "-P2", mock_dataprotect_chart_name, "-P3", mock_dataprotect_image_name, 'expand']
        result = runner.invoke(main_prog, args)
        assert result.exit_code == 0
        self.check_cluster_ok(mock_cluster)

    def delete_node(self, node):
        runner = CliRunner()
        args = ["e6000"]
        args += ['-f', 'config.yaml', 'delete_node', '--node_name', node]
        result = runner.invoke(main_prog, args)
        assert result.exit_code == 0

    def namespace_get_create_and_delete(self, pool_id, account_id):
        namespace_name = 'mock_namespace'
        args = ['e6000']
        args += ['-f', 'config.yaml', 'namespace', 'create', namespace_name,
                 f'--pool_id={pool_id}', f'--account_id={account_id}']
        runner = CliRunner()
        result = runner.invoke(main_prog, args)
        assert result.exit_code == 0

        args = ['e6000']
        args += ['-f', 'config.yaml', 'namespace', 'get', namespace_name,
                 f'--account_id={account_id}']
        runner = CliRunner()
        result = runner.invoke(main_prog, args)
        assert result.exit_code == 0

        args = ['e6000']
        args += ['-f', 'config.yaml', 'namespace', 'delete', namespace_name,
                 f'--account_id={account_id}']
        runner = CliRunner()
        result = runner.invoke(main_prog, args)
        assert result.exit_code == 0

    def simbaos_reset_and_install(self):
        args = ["e6000"]
        args += ['-f', 'config.yaml', 'simbaos', 'reset']
        runner = CliRunner()
        result = runner.invoke(main_prog, args)
        assert result.exit_code == 0

        args = ["e6000"]
        args += ['-f', 'config.yaml', 'simbaos', 'install']
        runner = CliRunner()
        result = runner.invoke(main_prog, args)
        assert result.exit_code == 0

    def dataprotect_reset_and_install(self):
        args = ['e6000']
        args += ['-f', 'config.yaml', 'dataprotect', 'reset']
        runner = CliRunner()
        result = runner.invoke(main_prog, args)
        assert result.exit_code == 0

        args = ['e6000']
        args += ['-f', 'config.yaml', 'dataprotect', 'install']
        runner = CliRunner()
        result = runner.invoke(main_prog, args)
        assert result.exit_code == 0

    def upgrade_dpserver(self):
        args = ['e6000']
        args += ['-f', 'config.yaml', 'upgrade_dpserver', '--package',
                 mock_upgrade_dpserver_package]
        runner = CliRunner()
        result = runner.invoke(main_prog, args)
        assert result.exit_code == 0



    @mock.patch('requests.Session', MockRequestSession)
    @mock.patch('getpass.getpass', mock_getpass)
    @mock.patch('dpclient.read_e6000_xlsx', mock_e6000_read_xlsx)
    @mock.patch('subcommands.dpserver.extract_version_from_package', )
    def test_dpclient_xlsx(self, mock_extract_version):
        self.install_cluster_xlsx()
        self.namespace_get_create_and_delete(mock_cluster.pacific.pool.id, 0)
        self.dataprotect_reset_and_install()
        self.simbaos_reset_and_install()
        self.expand_cluster(5)
        self.expand_cluster(16)
        self.expand_cluster(32)
        for i in range(0, 29):
            self.delete_node(f'node{i+3}')  # 删除node3 ~ node31
        self.reset_cluster()

        mock_extract_version.return_value = mock_dpserver_version2
        self.upgrade_dpserver()
