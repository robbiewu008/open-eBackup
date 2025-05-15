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
import os
import socket
import ssl

import urllib3
from fastapi import HTTPException

from app.base.consts import CERT_DIR, KEY_DIR, CNF_DIR, CA_DIR, CRL_DIR
from app.base.db_base import database
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exter_attack import exter_attack
from app.common.logger import get_logger
from app.common.util.cleaner import clear
from app.resource.client.open_storage_client import get_resource_ip
from app.resource.client.system_base import text_decrypt
from app.resource.models.resource_models import EnvironmentTable
from app.common.deploy_type import DeployType

log = get_logger(__name__)


def is_response_status_ok(response):
    if response:
        return response.status < 400
    return False


def get_key_password():
    with open(CNF_DIR, 'r') as file:
        text = file.read()
        return text_decrypt(text)


@exter_attack
def url_request_for_agent(method, agent_ip, suffix, headers=None, body=None):
    from app.resource.client.agent_client import query_proxy
    key_pass = get_key_password()
    port = get_agent_port(agent_ip)
    if ":" in agent_ip:
        agent_ip = f'[{agent_ip}]'
    http = get_http(agent_ip, key_pass, port)
    response = http.request(method, suffix, headers=headers, body=body)
    clear(key_pass)
    http.close()
    return response


def get_http(agent_ip, key_pass, port):
    ssl_context = get_ssl_context(key_pass)
    vrf_bind_option = (socket.SOL_SOCKET, socket.SO_BINDTODEVICE, "vrf-srv".encode('utf-8'))
    # assert_hostname 与 ssl_context.check_hostname含义相同，但是urllib3要求两个参数同时设置，否则会报错
    if DeployType().is_ocean_protect_type():
        http = urllib3.HTTPSConnectionPool(host=agent_ip, port=int(port), maxsize=1,
                                           ssl_context=ssl_context, assert_hostname=False,
                                           retries=False, timeout=60,
                                           source_address=(get_resource_ip(agent_ip, port), 0),
                                           socket_options=[vrf_bind_option])
    else:
        http = urllib3.HTTPSConnectionPool(host=agent_ip, port=int(port), maxsize=1,
                                           ssl_context=ssl_context, assert_hostname=False,
                                           retries=False, timeout=60)
    return http


@exter_attack
def url_request(method, ip, suffix, headers=None, body=None):
    key_pass = get_key_password()
    port = get_agent_port(ip)
    if ":" in ip:
        ip = f'[{ip}]'
    url = f'https://{ip}:{port}{suffix}'
    log.info(f'[Adapter url]: {url}')
    http = get_http(ip, key_pass, port)
    response = http.request(method, suffix, headers=headers, body=body)
    clear(key_pass)
    http.close()
    return response


@exter_attack
def get_ssl_context(key_pass):
    ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
    ssl_context.check_hostname = False
    ssl_context.verify_mode = ssl.CERT_REQUIRED
    ssl_context.load_cert_chain(certfile=CERT_DIR, keyfile=KEY_DIR, password=key_pass)
    ssl_context.load_verify_locations(cafile=CA_DIR)

    # 检查吊销证书列表
    if os.path.exists(CRL_DIR):
        ssl_context.load_verify_locations(cafile=CRL_DIR)
        ssl_context.verify_flags = ssl.VERIFY_CRL_CHECK_LEAF
    return ssl_context


def get_agent_port(ip):
    with database.session() as db:
        hosts = db.query(EnvironmentTable).filter(
            EnvironmentTable.endpoint == ip).all()
        host = hosts[0]
        if len(hosts) == 2:
            host = get_the_same_ip_host(hosts)
        return host.port


def get_the_same_ip_host(hosts):
    # 将集群ip更改为节点IP时，IP冲突导致无法访问。
    for host in hosts:
        if host.sub_type == ResourceSubTypeEnum.DBBackupAgent.value:
            if host.is_cluster is False:
                return host
    return hosts[0]


def query_databases(ip):
    try:
        response = url_request('GET', ip, '/agent/oracle/databases')
    except urllib3.exceptions.HTTPError as ex:
        log.exception(f"[Query database failed]: {ip}.")
        raise ex
    finally:
        pass

    if is_response_status_ok(response):
        return json.loads(response.data.decode('utf-8'))
    else:
        raise HTTPException(response.status)


def query_vm_agent_info(ip):
    # vmware agent 心跳机制
    try:
        response = url_request_for_agent('GET', ip, '/agent/host')
    except urllib3.exceptions.HTTPError as ex:
        log.exception(f"[Query vm status failed] ip: {ip}.")
        raise ex
    finally:
        pass

    if is_response_status_ok(response):
        return json.loads(response.data.decode('utf-8'))
    else:
        raise HTTPException(response.status)


def notice_vm_agent_update_service_links(ip):
    # 通知agent去更新links
    try:
        response = url_request_for_agent('GET', ip, '/agent/host/updatelinks')
    except urllib3.exceptions.HTTPError as ex:
        log.exception(f"[Notice vm update failed] ip: {ip}.")
        raise ex
    finally:
        pass
    if is_response_status_ok(response):
        log.info(f"[Notice vm update success] ip: {ip}.")
    else:
        raise HTTPException(response.status)


def query_database_tablespace(ip, db_name, inst_name, username, password):
    """
    os认证的情况传入任何参数均可取到
    数据库认证的情况，正确的用户名密码才能获取
    """
    headers = {
        'dbUserName': username,
        'dbPassword': password
    }
    response = url_request('GET',
                           ip,
                           f'/agent/oracle/tablespace?instName={inst_name}&dbName={db_name}',
                           headers=headers)
    if not is_response_status_ok(response):
        raise HTTPException(response.status)

    return json.loads(response.data.decode('utf-8'))


def query_asm_instance(ip):
    try:
        response = url_request('GET', ip, '/agent/oracle/asm')
    except urllib3.exceptions.HTTPError as ex:
        raise ex
    finally:
        pass

    if is_response_status_ok(response):
        return json.loads(response.data.decode('utf-8'))
    else:
        raise HTTPException(response.status)


def testconnection(ip, db_name, inst_name, oracle_home, username, password):
    params = {
        "instName": inst_name,
    }
    if db_name is not None:
        params['dbName'] = db_name
    if oracle_home is not None:
        params['oracleHome'] = oracle_home
    data = json.dumps(params)
    headers = {
        'dbUserName': username,
        'dbPassword': password
    }
    response = url_request('PUT', ip, '/agent/oracle/databases/action/testconnection',
                           headers=headers, body=data)
    if not is_response_status_ok(response):
        raise EmeiStorBizException(
            error=ResourceErrorCodes.CONNECT_HOST_FAILED, parameters=[])

    return json.loads(response.data.decode('utf-8'))


def query_oracle_clusterinfo(ip):
    try:
        response = url_request('GET', ip, '/agent/oracle/clusterinfo')
    except urllib3.exceptions.HTTPError as ex:
        log.exception(f"[Get cluster failed] ip: {ip}.")
        raise ex
    finally:
        pass

    if is_response_status_ok(response):
        return json.loads(response.data.decode('utf-8'))
    else:
        raise HTTPException(response.status)


def query_oracle_application(ip):
    try:
        response = url_request('GET', ip, '/agent/app/application')

    except urllib3.exceptions.HTTPError as ex:
        log.exception(f"get agent application info failed ip: {ip}.")
        raise ex
    finally:
        pass

    if is_response_status_ok(response):
        return json.loads(response.data.decode('utf-8'))
    else:
        raise HTTPException(response.status)


def update_oracle_snmp_trap(ip, data):
    try:
        data = json.dumps(data)
        response = url_request(
            'POST', ip, '/agent/host/trapserver/update', body=data)

    except urllib3.exceptions.HTTPError as ex:
        log.exception(f"POST agent snmp trap info failed ip : {ip}.")
        raise EmeiStorBizException(ResourceErrorCodes.RESOURCE_LINKSTATUS_OFFLINE,
                                   message=f"The linkstatus of resource is offline.") from ex
    finally:
        pass
    return is_response_status_ok(response)


def query_dws_host_agent(ip):
    try:
        response = url_request('GET', ip, '/agent/host')

    except urllib3.exceptions.HTTPError as ex:
        log.exception(f"get agent dws status info failed ip: {ip}.")
        raise ex
    finally:
        pass
    if is_response_status_ok(response):
        return json.loads(response.data.decode('utf-8'))
    else:
        raise HTTPException(response.status)
