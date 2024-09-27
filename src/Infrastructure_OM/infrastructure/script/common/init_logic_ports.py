import functools
import json
import logging
import os
import re
import sys
import time

from kubernetes import config as k8s_config
from kubernetes import client
from urllib import request

from kubernetes.client.rest import ApiException

k8s_config.load_incluster_config()
K8S_API_OBJ = client.CoreV1Api()
K8S_APP_OBJ = client.AppsV1Api()
logger = logging.getLogger()
LOG_PATH = f'/opt/OceanProtect/logs/{os.getenv("NODE_NAME", "node-0")}/infrastructure/init_logic_ports/logic_ports.log'
LOG_FORMATTER = "[%(asctime)s][%(levelname)s][%(message)s][%(funcName)s][%(process)d][%(thread)d]" \
                "[%(name)s][%(threadName)s][%(filename)s][%(lineno)s]"
POD_LOGIC_PORT_MAP = {
    'gaussdb': 'gaussdb_internal_communicate_net_plane',
    'sftp': 'sftp_net_plane',
    'inf': 'infrastructure_internal_communicate_net_plane'
}
# ETH 运行状态 0：未知 10：已连接 11：未连接 33：待恢复
ETH_CONNECTED_STATUS = '10'
# ETH健康状态 0：未知 1：正常 2：故障 7：有误码
ETH_HEALTH_STATUS = '1'
# 逻辑端口角色 1：管理 2：数据 3：管理+数据  4：复制 8：客户端 11：数据备份
LOGIC_PORT_DATA_BACKUP_ROLE = '11'
# 逻辑端口父端口类型 1：以太网端口 7：绑定 8：VLAN 25：VIP
ETH_PORT_TYPE = '1'
LOGIC_PORT_FAMILY_IPV4 = '0'
LOGIC_PORT_FAMILY_IPV6 = '1'
DEPLOY_TYPE = os.environ.get("DEPLOY_TYPE")
X9000_DEPLOY_TYPE = 'd6'
X9000_LOCATION_MAP = {
    'H': {
        'upper_half': 'ABCD',
        'lower_half': 'CDAB'
    },
    'L': {
        'upper_half': 'BCDA',
        'lower_half': 'DABC'
    }
}

class HOMEPORTTYPE:
    ETH = '1'
    BOND = '7'
    VLAN = '8'


def retry_when_exception(exceptions=Exception, retry_times=3, delay=1, logger=None):
    def decorator(func):
        @functools.wraps(func)
        def wrapper(*args, **kwargs):
            _times = retry_times
            while _times:
                try:
                    return func(*args, **kwargs)
                except exceptions as exception:
                    _times -= 1
                    if not _times:
                        logger.error(f'Retry failed')
                        raise exception
                    if logger is not None:
                        logger.error(f'Retrying in {delay} seconds...')
                    time.sleep(delay)

        return wrapper

    return decorator


class LifInfo:
    def __init__(self, ip_version, ip_address, mask, gateway, port_id_list, name, home_port_type, vlan, bond_port,
                 is_reuse, is_share_bond_port):
        self.address_family = ip_version
        self.ip_address = ip_address
        self.mask = mask
        self.gateway = gateway
        self.port_list = port_id_list
        self.name = name
        self.home_port_type = home_port_type
        self.vlan = vlan
        self.bond_port = bond_port
        self.is_reuse = is_reuse
        self.is_share_bond_port = is_share_bond_port


class DMClient:
    URL_WITHOUT_TOKEN = 'http://127.0.0.1:5555/deviceManager/rest/xxx'

    def get_x9000_controller_sequence(self, location):
        try:
            match = re.match(r"CTE0\.IOM\.([A-Z])(\d+)\.P(\d+)", location)
            if match:
                phy_loc = match.group(1)
                h_loc = match.group(2)
                p_loc = match.group(3)
            else:
                logger.error(f'Did not find the matched location for eth port. match: {match}')
                return ''
            if int(h_loc) <= 6:
                return X9000_LOCATION_MAP[phy_loc]['upper_half'][int(p_loc)]
            else:
                return X9000_LOCATION_MAP[phy_loc]['lower_half'][int(p_loc)]
        except Exception as e:
            logger.error(f'Failed to get the controller sequence, location:  {location}')
            return ''

    def get_x9000_controller_H_L(self, location):
        try:
            return location.split(".P")[0]
        except Exception as e:
            logger.error(f'Failed to get the controller sequence, location:  {location}')
            return ''

    def select_x9000_eth_port(self, lif_data, cur_controller):
        lif_eth_name = lif_data.get("LOCATION")
        controller = self.get_x9000_controller_sequence(lif_eth_name)
        if not controller:
            logger.error(f'Did not get the controller')
            return ''
        if f'0{controller}' == cur_controller and lif_data.get('HEALTHSTATUS') == ETH_HEALTH_STATUS and \
                lif_data.get('RUNNINGSTATUS') == ETH_CONNECTED_STATUS:
            logger.info(f'Find the controller: cur_controller: {cur_controller}, id: {lif_data.get("ID")}')
            return lif_data.get('ID')
        return ''

    def select_same_controller_port(self, port_list, cur_controller):
        selected_eth_port_id = ''
        for port in port_list:
            data = self.query_eth_port_id(port)
            if data == None:
                continue
            if data.get('HEALTHSTATUS') == ETH_HEALTH_STATUS and data.get('RUNNINGSTATUS') == ETH_CONNECTED_STATUS \
                    and data.get('OWNINGCONTROLLER') == cur_controller:
                selected_eth_port_id = data.get('ID')
        return selected_eth_port_id

    def create_bond(self, eth_id_list, pod_name, cur_controller, mtu=None):
        url = '/bond_port'
        body = {
            'NAME': f'cluster_{cur_controller}_{pod_name}',
            'PORTIDLIST': eth_id_list
        }
        res_create_bond = self.send_request(url, 'POST', body=body)[0]
        if not res_create_bond:
            logger.error(f'Failed to create the bond for eth_id_list: {eth_id_list}')
        if not mtu:
            return res_create_bond
        url = f'/bond_port/{res_create_bond.get("ID")}'
        body = {
            'MTU': mtu
        }
        res_update_bond = self.send_request(url, 'PUT', body=body)
        if res_update_bond[1] == 0:
            logger.info(f'Success to update MTU: {mtu}')
            return res_create_bond
        return ''

    def check_bond_exist(self, port_id_list):
        shared_port_id = []
        for port_id in port_id_list:
            port_data = self.query_eth_port_id(port_id)
            if port_data.get("BONDNAME") != "":
                shared_port_id.append(port_id)
        shared_bond_id = ""
        data = self.query_all_bond_port()
        for item in data:
            port_id_list = item['PORTIDLIST'].strip('[]').split(',')
            if any(port_id.strip('"') in shared_port_id for port_id in port_id_list):
                shared_bond_id = item['ID']
                break
        return shared_bond_id != ""

    def create_multi_bond(self, lif_info, cur_controller, pod_name):
        if lif_info.home_port_type == HOMEPORTTYPE.BOND:
            bond_port_id_list = lif_info.bond_port.get("port_list")
        else:
            bond_port_id_list = lif_info.vlan.get("port_list")
        controller_port_list = {}
        for port_id in bond_port_id_list:
            port_data = self.query_eth_port_id(port_id)
            if port_data is None:
                continue
            if DEPLOY_TYPE != X9000_DEPLOY_TYPE:
                owning_controller = port_data.get("OWNINGCONTROLLER")
            else:
                owning_controller = self.get_x9000_controller_H_L(port_data.get("LOCATION"))
            if not controller_port_list.get(owning_controller):
                controller_port_list[owning_controller] = [port_id]
            else:
                controller_port_list[owning_controller].append(port_id)
        controller_bond_id_list_map = {}
        for controller, port_id_list in controller_port_list.items():
            if lif_info.home_port_type == HOMEPORTTYPE.BOND:
                mtu = lif_info.bond_port.get("mtu")
            if lif_info.home_port_type == HOMEPORTTYPE.VLAN:
                mtu = lif_info.vlan.get("mtu")
            if lif_info.is_share_bond_port:
                if self.check_bond_exist(port_id_list):
                    logger.info(f'share_bond_port do not need create bond')
                    continue
            res_create_bond = self.create_bond(port_id_list, pod_name, controller, mtu)
            if not res_create_bond:
                logger.error(f'Failed to create bond, port_id_list: {port_id_list}')
                return ''
            controller_bond_id_list_map[controller] = res_create_bond.get('ID')
        logger.info(f'controller_bond_id_list_map: {controller_bond_id_list_map} ')
        return controller_bond_id_list_map

    def create_bond_port(self, lif_info, cur_controller, pod_name):
        if not lif_info.is_reuse:
            controller_bond_id_list_map = self.create_multi_bond(lif_info, cur_controller, pod_name)
            # 共享场景下gaussdb和inf在同一控时不需要再创建bond
            if lif_info.is_share_bond_port:
                bond_port_id_list = lif_info.bond_port.get("port_list")
                bond_info = self.query_all_bond_port()
                bond_id = {}
                for bond_data in bond_info:
                    port_id_list = [str(port_id) for port_id in json.loads(bond_data['PORTIDLIST'])]
                    for port_id in port_id_list:
                        if port_id not in bond_port_id_list:
                            break
                        controller = self.query_eth_port_id(port_id)['OWNINGCONTROLLER']
                        bond_id[controller] = bond_data['ID']
                share_port_id = controller_bond_id_list_map.get(cur_controller, bond_id[cur_controller])
                res_create_logic_port = self.create_logic_port(lif_info, cur_controller, port_id=share_port_id)
                return res_create_logic_port
            if not controller_bond_id_list_map:
                logger.error(f'Failed to create multi bond')
                return ''
            if DEPLOY_TYPE != X9000_DEPLOY_TYPE:
                res_create_logic_port = self.create_logic_port(lif_info, cur_controller,
                                                               port_id=controller_bond_id_list_map[cur_controller])
            else:
                for port_id in controller_bond_id_list_map.values():
                    res_create_logic_port = self.create_logic_port(lif_info, cur_controller, port_id=port_id)
                    break
            return res_create_logic_port
        else:
            if DEPLOY_TYPE == X9000_DEPLOY_TYPE:
                for port_id in lif_info.bond_port.get('port_list'):
                    res_create_logic_port = self.create_logic_port(lif_info, cur_controller, port_id=port_id)
                    break
                return res_create_logic_port
            for bond_id in lif_info.bond_port.get('port_list'):
                bond_info = self.query_bond_port_id(bond_id)
                port_list = json.loads(bond_info.get('PORTIDLIST'))
                port_id = port_list[0]
                port_data = self.query_eth_port_id(port_id)
                if port_data == None:
                    continue
                bond_controller = port_data.get('OWNINGCONTROLLER')
                if bond_controller == cur_controller:
                    res_create_logic_port = self.create_logic_port(lif_info, cur_controller, port_id=bond_id)
                    return res_create_logic_port
            return ''

    def query_vlan_port(self):
        url = '/vlan'
        return self.send_request(url, 'GET')[0]

    def query_vlan_port_id(self, port_id):
        url = f'/vlan/{port_id}'
        return self.send_request(url, 'GET')[0]

    def create_vlan(self, port_id, port_type, tag, mtu=None):
        url = '/vlan'
        body = {
            'TAG': tag,
            'PORTTYPE': port_type,
            'PORTID': port_id
        }
        res_create_vlan = self.send_request(url, 'POST', body=body)[0]
        if not res_create_vlan:
            logger.error(f'Failed to create the vlan for port id: {port_id}')
            return res_create_vlan
        if mtu:
            # 底座未提供创建的时候带入mtu最大传输单元参数，需要修改来适配
            url_update = f'/vlan/{res_create_vlan.get("ID")}'
            body_update = {
                'MTU': int(mtu)
            }
            res_update_vlan = self.send_request(url_update, 'PUT', body=body_update)
            if res_update_vlan[1] != 0:
                logger.error(f'Failed to update the vlan to MTU')
                return ''
        logger.info(f'Success to create vlan for port: {port_id}')
        return res_create_vlan

    def creat_vlan_port_eth(self, lif_info, cur_controller):
        eth_id_list = lif_info.port_list
        if not lif_info.is_reuse:
            if not eth_id_list:
                logger.error(f'Failed find the port for vlan, deploy type: {DEPLOY_TYPE}, '
                             f'lif_info ip: {lif_info.ip_address}')
                return ''
            for eth_id in eth_id_list:
                res_create_vlan = self.create_vlan(eth_id, HOMEPORTTYPE.ETH, lif_info.vlan.get("tag"))
                if not res_create_vlan:
                    return res_create_vlan
                res_query_eth_port = self.query_eth_port_id(eth_id)
                if res_query_eth_port is None:
                    continue
                if res_query_eth_port.get('OWNINGCONTROLLER') == cur_controller:
                    port_id = res_create_vlan.get('ID')
            if DEPLOY_TYPE == X9000_DEPLOY_TYPE:
                port_id = res_create_vlan.get('ID')
            res_create_port = self.create_logic_port(lif_info, cur_controller, port_id=port_id)
            if not res_create_port:
                logger.error(f'Failed to create port for vlan, lif_info ip: {lif_info.ip_address}')
            return res_create_port
        for vlan_id in eth_id_list:
            if DEPLOY_TYPE == X9000_DEPLOY_TYPE:
                res_create_logic_port = self.create_logic_port(lif_info, cur_controller, port_id=vlan_id)
                return res_create_logic_port
            vlan_info = self.query_vlan_port_id(vlan_id)
            port_id = json.loads(vlan_info.get('PORTID'))
            port_data = self.query_eth_port_id(port_id)
            if port_data == None:
                continue
            bond_controller = port_data.get('OWNINGCONTROLLER')
            if bond_controller == cur_controller:
                res_create_logic_port = self.create_logic_port(lif_info, cur_controller, port_id=vlan_id)
                return res_create_logic_port
        return ''

    def create_vlan_port_bond(self, lif_info, cur_controller, pod_name):
        if not lif_info.is_reuse:
            controller_bond_id_list_map = self.create_multi_bond(lif_info, cur_controller, pod_name)
            for controller, port_id in controller_bond_id_list_map.items():
                url = '/vlan'
                body = {
                    'TAG': lif_info.vlan.get('tag'),
                    'PORTTYPE': HOMEPORTTYPE.BOND,
                    'PORTID': port_id
                }
                res_create_vlan = self.send_request(url, 'POST', body=body)[0]
                if not res_create_vlan:
                    logger.error(f'Failed to create the vlan bond for port id: {port_id}')
                    return bool(res_create_vlan)
                # 底座未提供创建的时候带入mtu最大传输单元参数，需要修改来适配
                if lif_info.vlan.get("mtu"):
                    url_update = f'/vlan/{res_create_vlan.get("ID")}'
                    body_update = {
                        'MTU': int(lif_info.vlan.get("mtu"))
                    }
                    res_update_vlan = self.send_request(url_update, 'PUT', body=body_update)
                    if res_update_vlan[1] != 0:
                        logger.error(f'Failed to update the vlan to MTU')
                        return False

                if not lif_info.is_share_bond_port and DEPLOY_TYPE != X9000_DEPLOY_TYPE:
                    if cur_controller == controller:
                        self.create_logic_port(lif_info, cur_controller, res_create_vlan.get('ID'))

            if DEPLOY_TYPE == X9000_DEPLOY_TYPE:
                self.create_logic_port(lif_info, cur_controller, res_create_vlan.get('ID'))
                logger.info(f'Success to create vlan port bond')
                return True

            # 共享场景下gaussdb和inf在同一控时不需要再创建vlan
            if lif_info.is_share_bond_port:
                vlan_port_id_list = lif_info.vlan.get("port_list")
                vlan_info = self.query_vlan_port()
                vlan_id = {}
                for vlan_data in vlan_info:
                    bond_id = vlan_data['PORTID']
                    bond_data = self.query_bond_port_id(bond_id)
                    port_id_list = [str(port_id) for port_id in json.loads(bond_data['PORTIDLIST'])]
                    for port_id in port_id_list:
                        if port_id not in vlan_port_id_list:
                            break
                        controller = self.query_eth_port_id(port_id)['OWNINGCONTROLLER']
                        vlan_id[controller] = vlan_data['ID']
                self.create_logic_port(lif_info, cur_controller, vlan_id[cur_controller])
            logger.info(f'Success to create vlan port bond')
            return True

        for vlan_id in lif_info.vlan.get('port_list'):
            if DEPLOY_TYPE == X9000_DEPLOY_TYPE:
                res_create_logic_port = self.create_logic_port(lif_info, cur_controller, port_id=vlan_id)
                return res_create_logic_port
            vlan_info = self.query_vlan_port_id(vlan_id)
            bond_id = json.loads(vlan_info.get('PORTID'))
            bond_info = self.query_bond_port_id(bond_id)
            port_list = json.loads(bond_info.get('PORTIDLIST'))
            port_id = port_list[0]
            port_data = self.query_eth_port_id(port_id)
            if port_data == None:
                continue
            bond_controller = port_data.get('OWNINGCONTROLLER')
            if bond_controller == cur_controller:
                res_create_logic_port = self.create_logic_port(lif_info, cur_controller, port_id=vlan_id)
                return res_create_logic_port

    def create_vlan_port(self, lif_info, cur_controller, pod_name=None):
        if lif_info.vlan.get('port_type') == HOMEPORTTYPE.ETH:
            return self.creat_vlan_port_eth(lif_info, cur_controller)
        elif lif_info.vlan.get('port_type') == HOMEPORTTYPE.BOND:
            return self.create_vlan_port_bond(lif_info, cur_controller, pod_name)
        else:
            logger.error(f'Not a valid port type when choose vlan: {lif_info.vlan.get("port_type")}')
            return ''

    def create_logic_port(self, lif_info, cur_controller, port_id=None):
        selected_eth_port = None
        if not port_id:
            for lif_port in lif_info.port_list:
                data = self.query_eth_port_id(lif_port)
                if data == None:
                    continue
                # x9000的以太网口控制器划分和其他型号不同
                if DEPLOY_TYPE == X9000_DEPLOY_TYPE:
                    selected_eth_port = self.select_x9000_eth_port(data, cur_controller)
                    if selected_eth_port:
                        logger.info(f'Found the eth port for x9000')
                        break
                else:
                    if (data.get('HEALTHSTATUS') == ETH_HEALTH_STATUS and data.get('RUNNINGSTATUS') == ETH_CONNECTED_STATUS
                            and data.get('OWNINGCONTROLLER') == cur_controller):
                        selected_eth_port = data.get('ID')
            if not selected_eth_port and DEPLOY_TYPE != X9000_DEPLOY_TYPE:
                logger.error(f'Failed to find the corresponding eth port to build logic port.')
                return None
        else:
            selected_eth_port = port_id
        url = '/lif'
        body = {
            'NAME': lif_info.name,
            'HOMEPORTID': selected_eth_port,
            'ROLE': 11,
            'HOMEPORTTYPE': int(lif_info.home_port_type)
        }
        if lif_info.address_family == LOGIC_PORT_FAMILY_IPV4:
            body['ADDRESSFAMILY'] = int(LOGIC_PORT_FAMILY_IPV4)
            body['IPV4ADDR'] = lif_info.ip_address
            body['IPV4MASK'] = lif_info.mask
            body['IPV4GATEWAY'] = lif_info.gateway if lif_info.gateway else ''
        elif lif_info.address_family == LOGIC_PORT_FAMILY_IPV6:
            body['ADDRESSFAMILY'] = int(LOGIC_PORT_FAMILY_IPV6)
            body['IPV6ADDR'] = lif_info.ip_address
            body['IPV6MASK'] = lif_info.mask
            body['IPV6GATEWAY'] = lif_info.gateway if lif_info.gateway else ''
        else:
            return None
        if DEPLOY_TYPE == X9000_DEPLOY_TYPE:
            # x9000允许用户只选择一个以太口来创建逻辑端口，x9000情况下需要先创建逻辑端口，可能在任何控制器，然后再更新到pod所在控制器
            if lif_info.home_port_type == HOMEPORTTYPE.ETH:
                body["HOMEPORTID"] = lif_info.port_list[0]
            elif lif_info.home_port_type == HOMEPORTTYPE.VLAN:
                body["HOMEPORTID"] = port_id
            elif lif_info.home_port_type == HOMEPORTTYPE.BOND:
                body["HOMEPORTID"] = port_id
            logger.info(f'The home port id for x9000 written by pm base is {body["HOMEPORTID"]}')
            res_post = self.send_request(url, 'POST', body=body)[0]
            if not res_post:
                logger.error(f'Failed to create logic port for x9000, body: {body}')
            res_put = self.update_logic_port_on_controller(res_post["ID"], cur_controller)
            if res_put[1] != 0:
                logger.error(f'Failed to update the logic port for x9000, body: {body}')
            return res_put[1] == 0
        return self.send_request(url, 'POST', body=body)[0]

    def query_logic_port(self):
        url = '/lif'
        return self.send_request(url, 'GET')[0]

    def update_logic_port_on_controller(self, logic_id, controller):
        url = f'/lif/{logic_id}'
        body = {
            "HOMECONTROLLERID": controller
        }
        return self.send_request(url, 'PUT', body=body)

    def update_logic_port(self, lif_info, logic_id):
        url = f'/lif/{logic_id}'
        body = {
            'HOMEPORTID': lif_info.port_list[0],
            "HOMEPORTTYPE": lif_info.home_port_type
        }
        return self.send_request(url, 'PUT', body=body)

    def query_eth_port(self):
        url_service_port = f'/eth_port?LOGICTYPE=0'
        url_front_end_container_port = f'/eth_port?LOGICTYPE=13'
        data_service_port = self.send_request(url_service_port, 'GET')[0]
        data_front_end_container_port = self.send_request(url_front_end_container_port, 'GET')[0]
        data_service_port.extend(data_front_end_container_port)
        logger.info(f'The eth port for this env would be {data_service_port}')
        return data_service_port

    def query_eth_port_id(self, eth_id):
        url = f'/eth_port/{eth_id}'
        res = self.send_request(url, 'GET')
        if res == None:
            return None
        return res[0]

    def query_all_bond_port(self, name=None, cur_controller=None):
        url = f'/bond_port'
        if name:
            url = url + f'?filter=NAME:cluster_{cur_controller}_{name}'
        return self.send_request(url, 'GET')[0]

    def query_bond_port_id(self, bond_id):
        url = f'/bond_port/{bond_id}'
        return self.send_request(url, 'GET')[0]

    def select_bond_port(self, cur_controller, pod_name):
        all_bond_port_data = self.query_all_bond_port(pod_name, cur_controller)
        bond_data = all_bond_port_data[0] if len(all_bond_port_data) > 0 else {}
        return bond_data.get('ID')

    def select_vlan_eth_port(self, lif_info, cur_controller):
        vlan_data_list = self.query_vlan_port()
        for vlan in vlan_data_list:
            eth_port_id = vlan.get('PORTID')
            if eth_port_id in lif_info.port_list and vlan.get('PORTTYPE') == HOMEPORTTYPE.ETH:
                eth_data = self.query_eth_port_id(eth_port_id)
                if eth_data == None:
                    continue
                if eth_data.get('OWNINGCONTROLLER') == cur_controller and \
                   eth_data.get('HEALTHSTATUS') == ETH_HEALTH_STATUS and\
                   eth_data.get('RUNNINGSTATUS') == ETH_CONNECTED_STATUS:
                    return vlan.get('ID')
        return ''

    def select_bond_vlan_port(self, lif_info, cur_controller, pod_name):
        vlan_bond_name = f'cluster_{cur_controller}_{pod_name}.{lif_info.vlan.get("tag")}'
        vlan_data = self.query_vlan_port()
        for vlan in vlan_data:
            if vlan.get('NAME') == vlan_bond_name:
                return vlan.get('ID')
        return ''

    def update_eth_to_logic_port(self, lif_info, cur_controller, logic_id, pod_name):
        if DEPLOY_TYPE == X9000_DEPLOY_TYPE:
            if self.update_logic_port_on_controller(logic_id, cur_controller)[1] != 0:
                logger.error(f'Failed to update logic port controller on d6')
                return False
            logger.info(f'Success to update controller for d6')
            return True

        for lif_port in lif_info.port_list:
            # 复用下发的端口ID是vlan的
            if lif_info.is_reuse:
                vlan_port = self.query_vlan_port_id(lif_port)
                eth_port_id = vlan_port.get('PORTID')
                eth_port = self.query_eth_port_id(eth_port_id)
            # 非复用下发的端口ID是eth的
            else:
                eth_port = self.query_eth_port_id(lif_port)

            if eth_port == None:
                continue
            if eth_port.get('HEALTHSTATUS') == ETH_HEALTH_STATUS and \
               eth_port.get('RUNNINGSTATUS') == ETH_CONNECTED_STATUS and \
               eth_port.get('OWNINGCONTROLLER') == cur_controller:
                if lif_info.home_port_type == HOMEPORTTYPE.ETH:
                    lif_info.port_list = [eth_port.get("ID")]
                elif lif_info.home_port_type == HOMEPORTTYPE.VLAN:
                    if lif_info.is_reuse:
                        select_vlan_port = vlan_port.get('ID')
                    else:
                        select_vlan_port = self.select_vlan_eth_port(lif_info, cur_controller)
                    if not select_vlan_port:
                        logger.error(f'Failed to find the drifted vlan port')
                        return False
                    lif_info.port_list = [select_vlan_port]
                else:
                    logger.error(f'No corresponding home port type')
                    return False
                if self.update_logic_port(lif_info, logic_id)[1] != 0:
                    logger.error(f'Failed to update lif info')
                    return False
                logger.info(f'Succeed to update logic port: {lif_info.ip_address} for new eth por'
                            f't: {eth_port.get("ID")}')
                return True

        # vlan & bond 和 bond 的情况下，port_list为空
        if not lif_info.port_list:
            if lif_info.is_reuse:
                if lif_info.home_port_type == HOMEPORTTYPE.VLAN:
                    for lif_port in lif_info.vlan["port_list"]:
                        vlan_port = self.query_vlan_port_id(lif_port)
                        bond_port_id = vlan_port.get('PORTID')
                        bond_port = self.query_bond_port_id(bond_port_id)
                        eth_port_list_str = bond_port.get('PORTIDLIST')
                        # PORTIDLIST数据为字符串，需要转换
                        eth_port_list = [int(port_id) for port_id in json.loads(eth_port_list_str)]
                        for eth_port_id in eth_port_list:
                            eth_port = self.query_eth_port_id(eth_port_id)
                            if eth_port.get('HEALTHSTATUS') == ETH_HEALTH_STATUS and \
                               eth_port.get('RUNNINGSTATUS') == ETH_CONNECTED_STATUS and \
                               eth_port.get('OWNINGCONTROLLER') == cur_controller:
                                lif_info.port_list = [vlan_port.get('ID')]
                elif lif_info.home_port_type == HOMEPORTTYPE.BOND:
                    for lif_port in lif_info.bond_port["port_list"]:
                        bond_port = self.query_bond_port_id(lif_port)
                        eth_port_list_str = bond_port.get('PORTIDLIST')
                        # PORTIDLIST数据为字符串，需要转换
                        eth_port_list = [int(port_id) for port_id in json.loads(eth_port_list_str)]
                        for eth_port_id in eth_port_list:
                            eth_port = self.query_eth_port_id(eth_port_id)
                            if eth_port.get('HEALTHSTATUS') == ETH_HEALTH_STATUS and \
                               eth_port.get('RUNNINGSTATUS') == ETH_CONNECTED_STATUS and \
                               eth_port.get('OWNINGCONTROLLER') == cur_controller:
                                lif_info.port_list = [bond_port.get('ID')]
            else:
                if lif_info.home_port_type == HOMEPORTTYPE.BOND:
                    lif_info.port_list = [self.select_bond_port(cur_controller, pod_name)]
                elif lif_info.home_port_type == HOMEPORTTYPE.VLAN:
                    lif_info.port_list = [self.select_bond_vlan_port(lif_info, cur_controller, pod_name)]
                else:
                    logger.error(f'Unknown home port type writen in config map: {lif_info.home_port_type}')
            if self.update_logic_port(lif_info, logic_id)[1] != 0:
                logger.error(f'Failed to update lif info vlan and bond')
                return False
            logger.info(f'Succeed to update logic port: {lif_info.ip_address} for vlan and bond with new eth port')
            return True
        logger.error(f'Did not find the eth port for lif: {lif_info.ip_address}, current controller: {cur_controller}')
        return False

    @staticmethod
    def get_label_name_dict_to_node():
        try:
            api_response = K8S_API_OBJ.list_node()
            node_dict = {}
            for node_item in api_response.items:
                node_name = node_item.metadata.name
                node_label = DMClient.handle_by_env(node_item, node_name)
                node_dict[node_name] = node_label
            return node_dict
        except ApiException as e:
            raise ApiException(e) from e

    @staticmethod
    def check_log_path(path):
        # 将文件路径分割出来
        file_dir = os.path.split(path)[0]
        # 判断文件路径是否存在，如果不存在，则创建，此处是创建多级目录
        if not os.path.isdir(file_dir):
            os.makedirs(file_dir)
        # 然后再判断文件是否存在，如果不存在，则创建
        if not os.path.exists(path):
            os.mknod(path)

    @staticmethod
    def handle_logger(logger, log_path):
        logger.setLevel(logging.INFO)
        stream_handler = logging.StreamHandler()
        _formatter = logging.Formatter(LOG_FORMATTER)
        stream_handler.setFormatter(_formatter)
        DMClient.check_log_path(log_path)
        file_handler = logging.FileHandler(log_path, mode='a')
        file_handler.setFormatter(_formatter)
        logger.addHandler(stream_handler)
        logger.addHandler(file_handler)
        stream_handler.close()
        file_handler.close()

    @staticmethod
    def handle_by_env(node_item, node_name):
        # 安全一体机没有CTE label
        if DEPLOY_TYPE == 'd5':
            node_label = node_name
        # 不同环境不同处理逻辑的标签
        else:
            node_label = node_item.metadata.labels.get('controller')
            if node_label is not None:
                node_label = node_label.replace('CTE', '').replace('.', '')
        return node_label

    @staticmethod
    def get_net_plane_info_json(pod_name):
        net_plane_name = POD_LOGIC_PORT_MAP.get(pod_name)
        # 检查当前network-conf中逻辑端口是否已被创建
        try:
            network_conf = client.CoreV1Api().read_namespaced_config_map(name="network-conf", namespace="dpa")
        except Exception as e:
            logger.info(f'The config map network-conf does not exist.')
            os._exit(0)
        if not network_conf.data:
            logger.info(f'The network has not been set, do not need to create logic port')
            os._exit(0)
        return network_conf.data.get(net_plane_name)

    @staticmethod
    def build_net_plane_info(net_plane_info, cur_controller, pod_name):
        logic_ip = net_plane_info.get('ip')
        logic_port = net_plane_info.get('port_list')
        logic_mask = net_plane_info.get('mask')
        logic_gateway = net_plane_info.get('gateway')
        # 对于ipv6，为适配底座端口名称校验规则，将:改成.
        logic_ip_name = re.sub(r':', '.', logic_ip)
        logic_name = net_plane_info.get('name', f'cluster_{logic_ip_name}_{pod_name}')
        logic_ip_version = net_plane_info.get('ip_version')
        home_port_type = net_plane_info.get('home_port_type')
        vlan = net_plane_info.get('vlan')
        bond_port = net_plane_info.get('bond_port')
        is_reuse = True if net_plane_info.get('is_reuse', 'false') == 'true' else False
        is_share_bond_port = True if net_plane_info.get('is_share_bond_port', 'false') == 'true' else False

        lif_info = LifInfo(
            ip_version=logic_ip_version,
            gateway=logic_gateway,
            mask=logic_mask,
            ip_address=logic_ip,
            port_id_list=logic_port,
            name=logic_name,
            home_port_type=home_port_type,
            vlan=vlan,
            bond_port=bond_port,
            is_reuse=is_reuse,
            is_share_bond_port=is_share_bond_port
        )
        return lif_info

    @staticmethod
    @retry_when_exception(retry_times=3, delay=3)
    def send_request(url, method, body=None):
        headers = {"Content-Type": "application/json"}
        url = DMClient.URL_WITHOUT_TOKEN + url
        logger.info(f"{method}: {url}")
        if body:
            logger.info(f"body: {body}")
        body = json.dumps(body).encode()
        req = request.Request(url, data=body, method=method, headers=headers)
        rsp = request.urlopen(req)
        status, rsp_data = rsp.getcode(), json.loads(rsp.read().decode('utf-8'))
        if status != 200:
            logger.info(f"Send request error, status code: {status.status_code}.")
            return None
        if rsp_data.get("error", {}).get('code'):
            logger.error(f'Storage response error, error code: {rsp_data.get("error").get("code")}, '
                         f'description: {rsp_data.get("error").get("description")}.')
            return None
        return rsp_data.get('data', None), rsp_data.get('error', {}).get('code')


if __name__ == '__main__':
    if len(sys.argv) != 2:
        msg = f"Wrong number of parameters. It should be exactly 2, " \
              f"but given {len(sys.argv)}"
        logger.error(msg)
        os._exit(1)

    pod_name = sys.argv[1]
    dm_client = DMClient()
    dm_client.handle_logger(logger, LOG_PATH)
    logger.info(f'Start to create logic ports for {pod_name}')
    if pod_name not in POD_LOGIC_PORT_MAP.keys():
        logger.error(f'Did not find the corresponding net plane key, there must be some problem with the arg passed in.'
                     f' {pod_name}')
        os._exit(1)
    # 获取对应在config map--network-conf中的key
    net_plane_info_json = dm_client.get_net_plane_info_json(pod_name)
    if not net_plane_info_json:
        logger.info(f'The net plane has not been created by user yet, not need to init and create logic ports')
        os._exit(0)

    # 查询当前logic port是否存在, 如果存在则对比当前控制器和逻辑端口所属控制器，如果不一致则更新，一致则跳过；如果不存在则创建在本控
    logic_port_list = dm_client.query_logic_port()
    cur_node = os.getenv("NODE_NAME")
    cur_controller = dm_client.get_label_name_dict_to_node().get(cur_node)
    net_plane_info = json.loads(net_plane_info_json)
    lif_info = dm_client.build_net_plane_info(net_plane_info, cur_controller, pod_name)
    for logic_port in logic_port_list:
        if lif_info.address_family == LOGIC_PORT_FAMILY_IPV4:
            bool_same_logic_ip = lif_info.ip_address == logic_port.get('IPV4ADDR')
        elif lif_info.address_family == LOGIC_PORT_FAMILY_IPV6:
            bool_same_logic_ip = lif_info.ip_address == logic_port.get('IPV6ADDR')
        else:
            logger.error(f'Invalid ip address: {lif_info.ip_address}')
            os._exit(1)
        if bool_same_logic_ip and logic_port.get('ROLE') == LOGIC_PORT_DATA_BACKUP_ROLE:
            if logic_port.get('HOMECONTROLLERID') == cur_controller:
                logger.info(f'The logic port already exist in current controller, do not need to create.')
                os._exit(0)
            else:
                # 逻辑端口和当前控制器不在一控就轮询用户的选择configmap，找到第一个可用的以太网口
                res = dm_client.update_eth_to_logic_port(lif_info, cur_controller, logic_port.get('ID'), pod_name)
                if not res:
                    logger.error(f'Failed to update eth port to logic port')
                    os._exit(1)
                logger.info(f'Success to update the logic port')
                os._exit(0)
    logger.info(f'Did not find the logic port, need to create a new one.')

    # 创建新的逻辑端口
    # 1. 以太口 2.绑定端口 3. Vlan
    if lif_info.home_port_type == HOMEPORTTYPE.ETH:
        if not dm_client.create_logic_port(lif_info, cur_controller):
            logger.error(f'Failed to create the logic port: {lif_info.ip_address}')
            os._exit(1)
    elif lif_info.home_port_type == HOMEPORTTYPE.BOND:
        if not dm_client.create_bond_port(lif_info, cur_controller, pod_name):
            logger.error(f'Failed to create the bond port: {lif_info.ip_address}')
            os._exit(1)
    elif lif_info.home_port_type == HOMEPORTTYPE.VLAN:
        if not dm_client.create_vlan_port(lif_info, cur_controller, pod_name):
            logger.error(f'Failed to create the vlan port for {lif_info.ip_address}')
            os._exit(1)
    else:
        logger.error(f'Unknown home port type: {lif_info.home_port_type}')
        os._exit(1)
    logger.info(f'Succeed to create logic port')
    os._exit(0)
