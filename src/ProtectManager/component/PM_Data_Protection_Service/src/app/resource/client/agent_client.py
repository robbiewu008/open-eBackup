# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
import json
import random
import socket
from enum import Enum
from http import HTTPStatus

import urllib3
from sqlalchemy.orm import aliased
from urllib3.util import parse_url

from app.base.consts import CERT_DIR, KEY_DIR, CA_DIR
from app.base.db_base import database
from app.common.clients.client_util import SystemBaseHttpsClient, parse_response_data
from app.common.enums.resource_enum import ResourceSubTypeEnum, ResourceTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import IllegalParamException
from app.common.exter_attack import exter_attack
from app.common.logger import get_logger
from app.common.util.cleaner import clear
from app.resource.client.open_storage_client import get_resource_ip
from app.resource.models.resource_models import EnvironmentTable, ResExtendInfoTable, ResourceTable
from app.resource.rpc.hw_agent_rpc import get_key_password
from app.archive.service.service import get_backup_cluster_esn
from app.common.deploy_type import DeployType
from app.replication.client.replication_client import ReplicationClient

log = get_logger(__name__)


# 将列表创建一个枚举类
def list_to_enum(enum_name, list1):
    dic = dict(zip(list(set(list1)), list(set(list1))))
    enum_class_name = Enum(enum_name, dic)
    return enum_class_name


# 查询可更新的客户端的信息(版本号和时间戳)
def query_can_update_agent_versions():
    """
    :param params: no params
    :return: list(dict)
    """
    update_res = None
    url = f"/v1/internal/query/agent/infos"
    response = SystemBaseHttpsClient().request("GET", url)
    if response.status == HTTPStatus.OK:
        return parse_response_data(response.data)
    else:
        log.info(
            f'invoke api to count agent update version info failed!, '
            f'response.status: {response.status},request url: {url}')
    return update_res


# 根据ip查询代理
def query_proxy(agent_ip):
    proxy = 'http://protectengine:8090'
    log.info(f'query dme_proxy ip :{agent_ip}')
    with database.session() as session:
        # 根据ip, type查询uuid
        environment: EnvironmentTable = session.query(EnvironmentTable).filter(
            EnvironmentTable.endpoint == agent_ip,
            ResourceTable.type == ResourceTypeEnum.Host.value).first()
        if environment is None:
            log.error(f'environment is none, query dme_proxy ip :{agent_ip}')
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["ip"])
        resource_id = environment.uuid
        log.info(f'query dme_proxy ip :{agent_ip}, resource_id:{resource_id}')
        # 根据uuid动态获取ip
        condition_u = {
            ResExtendInfoTable.key == "agent_domain_available_ip",
            ResExtendInfoTable.resource_id == resource_id
        }

        res_extend_info: ResExtendInfoTable = session.query(ResExtendInfoTable).filter(*condition_u).one_or_none()
        local_esn = get_backup_cluster_esn()
        if res_extend_info is not None:
            value = res_extend_info.value
            domain = json.loads(value).get(local_esn)
            log.info(f'query dme_proxy ip :{agent_ip}, domain:{domain}')
            domain_arr = domain.split(',')
            index = random.randrange(0, len(domain_arr))
            log.info(f'query dme_proxy index :{index}, domain:{domain_arr[index]}')
            proxy = 'http://' + domain_arr[index] + ":8090"
    log.info(f'query dme_proxy end ip :{agent_ip}')
    return proxy


@exter_attack
def url_request(method, ip, port, suffix, headers=None, body=None):
    key_pass = get_key_password()
    try:
        if DeployType().is_ocean_protect_type():
            vrf_bind_option = (socket.SOL_SOCKET, socket.SO_BINDTODEVICE, "vrf-srv".encode('utf-8'))
            http_method = urllib3.HTTPSConnectionPool(ip, int(port), maxsize=1, cert_file=CERT_DIR, key_file=KEY_DIR,
                                                      key_password=key_pass, cert_reqs='CERT_REQUIRED',
                                                      assert_hostname=False, ca_certs=CA_DIR, retries=3, timeout=60,
                                                      source_address=(get_resource_ip(ip, port), 0),
                                                      socket_options=[vrf_bind_option])
        else:
            http_method = urllib3.HTTPSConnectionPool(ip, int(port), maxsize=1, cert_file=CERT_DIR, key_file=KEY_DIR,
                                                      key_password=key_pass, cert_reqs='CERT_REQUIRED',
                                                      assert_hostname=False, ca_certs=CA_DIR, retries=3, timeout=60)
        response = http_method.request(method, suffix, headers=headers, body=body)
    finally:
        clear(key_pass)
    http_method.close()
    return response


# 查询自研agent接口
def query_agent_version_info(ip, port, suffix):
    query_agent_info_res = None
    response = url_request('GET', ip, port, True, suffix)

    if response.status == HTTPStatus.OK:
        value = json.loads(response.data.decode('utf-8'))
        return value
    else:
        log.info(f'agent_client: invoke api to query agent error!, response_data is: '
                 f'{json.loads(response.data.decode("utf-8"))}, response.status is: {response.status}')
    return query_agent_info_res


# 查询爱数agent接口将agent_version 和 agent_timestamp 字段存到environments表中
def query_agent_version_info_by_api(subtype, ip, port):
    """
    :param subtype: 自研或者爱数(爱数暂时不支持自动升级)
    :param ip: ip
    :param port: 端口
    :return:
    """
    query_agent_info_res = None
    suffix = f"/agent/host/action/version/check"
    url = f"https://{ip}:{port}{suffix}"
    if subtype != ResourceSubTypeEnum.ABBackupClient.value:
        return query_agent_version_info(ip, port, suffix)
    else:
        log.info(f'invoke agent api to query agent info url: {url}')
    return query_agent_info_res


def remove_protect_unmount_repo(ip, port, app_type, body):
    suffix = f"/v1/agent/{app_type}/remove-protect"
    log.info("Start unmounting repository when remove protection, ip: %s, port: %s, url: %s.", ip, port, suffix)
    response = url_request('POST', ip, port, True, suffix, body=body)
    if response.status != HTTPStatus.OK:
        log.error("Unmount repository error when remove protection, response status: %s.", response.status)
        return
    log.info("Unmount repository success when remove protection.")


# 生成可升级版本的枚举类
def get_can_update_agent_versions():
    with database.session() as session:
        environment = aliased(EnvironmentTable)
        agent_versions = session.query(environment.version).all()
        resource_list = list(set(i[0] for i in agent_versions))
        enum_name = list_to_enum("VersionEnum", resource_list)
    return enum_name
