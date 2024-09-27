import requests
import logging as log
from typing import List
import time
from threading import Thread

from config import Config, PacificNode, StoragePoolConfig
from utils import make_pacifc_address, retry
import consts


class PacificClient:
    def __init__(self, config: Config):
        self.address = make_pacifc_address(config.float_ip)
        self.session = requests.Session()
        self.session.verify = False
        self.token = self._login(config.user, config.password)
        self.session.headers.update({'X-AUTH-TOKEN': self.token})

    def _login(self, user_name: str, password: str) -> str:
        url = f'https://{self.address}/api/v2/aa/sessions'
        r = self.session.post(url, json={
            'user_name': user_name,
            'password': password
        })
        self._check_result(r, 'Failed to login. Check user name and password')
        token = r.json().get('data').get('x_auth_token')

        # 后台保活token
        backgroud_keepalive = Thread(target=self._token_keepalive)
        backgroud_keepalive.daemon = True
        backgroud_keepalive.start()
        return token

    def _token_keepalive(self):
        while True:
            url = f'https://{self.address}/dsware/service/v2/sec/keepAlive'
            self.session.post(url)
            time.sleep(consts.PACIFIC_TOKEN_KEEPALIVE_INTERNAL)

    @retry()
    def get_esn(self) -> str:
        url = f'https://{self.address}/api/v2/common/esn'
        r = self.session.get(url)
        self._check_result(r, 'Failed to get esn')
        esn = r.json().get('data').get('esn')
        return esn

    @retry()
    def get_secondary_float_ip(self) -> str:
        url = f"https://{self.address}/api/v2/console/secondary_float_ip"
        r = self.session.get(url)
        self._check_result(r, 'Failed to get secondary float ip')
        return r.json().get('data').get('secondary_external_service_float_ip')

    @retry()
    def get_primary_fsm(self) -> (str, str):
        url = f'https://{self.address}/dsware/service/upgrade/getFsmNodeInfo'
        r = self.session.get(url)
        self._check_result(r, 'get primary fsm address failed')
        return r.json()['nodeIpPrimaryExter'], r.json()['nodeNamePrimary']

    def get_token(self) -> str:
        return self.token

    @retry()
    def get_servers(self) -> List[PacificNode]:
        url = f'https://{self.address}/api/v2/network_service/servers'
        r = self.session.post(url, json={})
        self._check_result(r, 'Failed to get servers info')

        results = []

        cluster_nodes = r.json().get('data')
        for n in cluster_nodes:
            node = PacificNode()
            node.name = n.get('name')
            node.role = n.get('role')
            for ip in n.get('ip_address_list'):
                for usage in ip.get('ip_usage'):
                    if usage == 'storage_ctrl_network':
                        node.storage_ctrl_network = ip.get('ip_address')
                        node.storage_ctrl_network_port_name = ip.get('port_name')
                    if usage == 'storage_frontend':
                        node.storage_frontend_ip = ip.get('ip_address')
                        node.storage_frontend_port_name = ip.get('port_name')
                    if usage == 'management_internal':
                        node.management_internal_ip = ip.get('ip_address')

            results.append(node)

        return results

    @retry()
    def scan_server_media(self, nodes: [PacificNode]):
        url = f"https://{self.address}/dsware/service/vsan/scanServerMedia"
        nodeIpList = list(map(
            lambda server: server.management_internal_ip,
            nodes
        ))
        r = self.session.post(url, json={
            "nodeIpList": nodeIpList
        })
        self._check_result(r, 'Failed to scan storage media')
        return r.json().get('servers')

    @retry()
    # query storage pool id using storage pool name
    def query_storage_pool(self, pool_name) -> int:
        url = f"https://{self.address}/dsware/service/cluster/storagepool/queryStoragePool"
        r = self.session.get(url)
        self._check_result(r, 'Failed to get storage pool')
        for storage_pool in r.json().get('storagePools'):
            if storage_pool.get('poolName') == pool_name:
                return storage_pool.get('poolId')
        return None

    # create storage pool, return task id
    def create_storage_pool(
        self,
        pool_name: str,
        storage_pool_config: StoragePoolConfig,
        nodes: [PacificNode]
    ) -> int:
        servers_medis = self.scan_server_media(nodes)
        server_list = []

        def get_media_role(media_type):
            if media_type == consts.STORAGE_POOL_MAIN_STORAGE_DISK_TYPE:
                return 'main_storage'
            elif media_type == consts.STORAGE_POOL_CACHE_DISK_TYPE:
                return 'osd_cache'
            return None

        for server_ip in servers_medis:
            media_list = []
            for disk in servers_medis[server_ip]:
                media_role = get_media_role(disk['mediaType'])
                if media_role is None or disk['mediaRole'] != 'no_use':
                    continue
                media_list.append({
                    "mediaRole": media_role,
                    "mediaType": disk['mediaType'],
                    "phyDevEsn": disk['phyDevEsn'],
                    "phySlotId": disk['phySlotId']
                })
            server_list.append({
                "nodeMgrIp": server_ip,
                "mediaList": media_list
            })

        url = f'https://{self.address}/dsware/service/cluster/storagepool/createStoragePool'
        create_storage_pool_body = {
            "poolPara": {
                "poolName": pool_name,
                "storageMediaType": "sata_disk",
                "cacheMediaType": "ssd_card",
                "securityLevel": "server",
                "serviceType": 5
            },
            "serverList": server_list
        }
        if storage_pool_config.replica_num is not None:
            create_storage_pool_body['poolPara']['redundancyPolicy'] = 'replication'
            create_storage_pool_body['poolPara']['replicaNum'] = storage_pool_config.replica_num
        else:
            create_storage_pool_body['poolPara']['redundancyPolicy'] = 'ec'
            create_storage_pool_body['poolPara']['numParityUnits'] = storage_pool_config.parity_units
            create_storage_pool_body['poolPara']['numFaultTolerance'] = storage_pool_config.fault_tolerance

        r = self.session.post(url, json=create_storage_pool_body)
        self._check_result(r, 'Failed to create storage_pool')
        return r.json().get('taskId')

    def delete_storage_pool(self, pool_id: str) -> int:
        url = f'https://{self.address}/dsware/service/cluster/storagepool/deleteStoragePool'
        r = self.session.post(url, json={
            'poolId': pool_id
        })
        self._check_result(r, 'Failed to delete storage pool')
        return r.json().get('taskId')

    @retry()
    def query_converged_eds_file_service(self, storage_pool_id: int):
        url = f'https://{self.address}/api/v2/cluster_service/converged_eds_service'
        r = self.session.get(url)
        self._check_result(r, 'Failed to query converged eds file service')
        return r.json().get('data')

    def delete_converged_eds_file_service(self, storage_pool_id: int):
        url = f'https://{self.address}/api/v2/cluster_service/converged_eds_service'
        r = self.session.delete(url, json={
            'storage_pool_id': storage_pool_id
        })
        self._check_result(r, "Failed to delete converged eds file service")
        return r.json().get('data').get('taskId')

    def create_converged_eds_file_service(self, storage_pool_id: int):
        url = f'https://{self.address}/api/v2/cluster_service/converged_eds_service'
        r = self.session.post(url, json={
            'action': 0,
            'storage_pool_id': storage_pool_id,
            'service_type_list': [2]
        })
        self._check_result(r, "Failed to create converged eds file service")
        return r.json().get('data').get('taskId')

    def expand_converged_eds_file_service(self, storage_pool_id: int, ip_list: [str]):
        url = f'https://{self.address}/api/v2/cluster_service/converged_eds_node'
        r = self.session.post(url, json={
            'action': 2,
            'storage_pool_id': storage_pool_id,
            'server_ip_list': ip_list,
        })
        self._check_result(r, "Failed to expand converged eds file service")
        return r.json().get('data').get('taskId')

    def scale_down_converged_eds_file_service(self, storage_pool_id: int, ip_list: [str]):
        url = f'https://{self.address}/api/v2/cluster_service/converged_eds_node'
        r = self.session.post(url, json={
            'action': 3,
            'storage_pool_id': storage_pool_id,
            'server_ip_list': ip_list,
        })
        self._check_result(r, "Failed to scale down converged eds file service")
        return r.json().get('data').get('taskId')

    @retry(tries=6, delay=10)
    def query_account(self, name: str) -> str:
        url = f'https://{self.address}/api/v2/account/accounts?name={name}'
        r = self.session.get(url)
        if r.status_code == requests.codes.ok and \
           r.json()['result']['code'] == 1800000404:  # user not found
            return None

        self._check_result(r, 'Failed to get account')
        return r.json()['data']['id']

    def create_account(self, name: str, encrypt_option: int) -> (str, str):
        url = f'https://{self.address}/api/v2/account/accounts'
        r = self.session.post(url, json={
            'name': name,
            'encrypt_option': encrypt_option
        })
        self._check_result(r, 'Failed to create account')
        return r.json().get('name'), r.json().get('id')

    def wait_for_task(self, task_id: int):
        last_progress = 0
        while True:
            taskInfo = self.query_task_info(task_id)
            taskStatus, progress = taskInfo['taskStatus'], taskInfo['progress']
            if taskStatus in ['running', 'waiting']:
                if progress != last_progress:
                    log.info(f'status: {taskStatus}, progress: {progress}%')
                    last_progress = progress
                time.sleep(10)
            else:
                return taskStatus

    @retry()
    def query_task_info(self, task_id: int):
        url = f"https://{self.address}/dsware/service/task/querySingleTaskInfo"
        r = self.session.get(url, params={
            'taskId': task_id
        })
        self._check_result(r, 'Failed to query task info')
        return r.json().get('taskInfo')

    def _check_result(self, r: requests.Response, msg: str):
        result = r.json().get('result')
        if r.status_code == requests.codes.ok and result is None:
            return
        if isinstance(result, int) and result == 0:
            return
        if isinstance(result, dict) and result.get('code') == 0:
            return
        raise Exception(f'{msg}, {r.text}')
