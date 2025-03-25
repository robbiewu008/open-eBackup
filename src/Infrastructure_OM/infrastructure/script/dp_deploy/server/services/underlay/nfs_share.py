import os
import json
import subprocess
import shlex
import requests
import platform
from server.common.exec_cmd import exec_cmd, exec_cmds_with_pipe
from server.common.logger.logger import logger
from server.common.consts import LOG_PATH, COMMAND_SUCCESS, COMMAND_FAILED, COMMAND_WRONG, GET_NAMESPACE_NOT_EXIST, \
    SCRIPTS_PATH
from server.schemas.request import AuthBase, CreateNamespaceRequest, CreateNFSShareRequest, CreatNFSClientRequest, \
    GetNamespaceResponse, GetNFSShareRequest, GetNamespaceRequest, GetNFSShareResponse, GetNFSClientRequest, \
    GetNFSClientResponse, DeleteNFSShareRequest, DeleteNFSClientRequest, DeleteNamespaceRequest
from server.common.exter import exter_attack
from server.common import mount, consts
from server.common.get_auth_token import get_auth_token
from fastapi import Header, HTTPException


def get_service_float_ip() -> str:
    with open(consts.PACIFIC_NETWORK_CONFIG, 'r') as f:
        for line in f:
            if line.startswith('service_float_ip'):
                return line.split('=')[1].strip()


def verify_token(x_auth_token: str = Header(default=None), ip: str = Header(default=None),
                 op_admin_user: str = Header(default=None), op_admin_pwd: str = Header(default=None)):
    logger.info("Begin to verify token in E6000 dpserver")
    service_float_ip = get_service_float_ip()
    if x_auth_token is None:
        re, x_auth_token = get_auth_token(service_float_ip, op_admin_user, op_admin_pwd)
        if re:
            logger.info(f"Login through sysadmin user, {op_admin_user}")
            return
        raise HTTPException(status_code=400)
    url = f"https://{service_float_ip}:8088/api/v2/aa/current_session"
    headers = {
        "X-Auth-Token": x_auth_token
    }
    r = requests.get(url=url, headers=headers, verify=False)
    sessions = r.json().get('data')
    if sessions is None or len(sessions) == 0:
        raise HTTPException(status_code=400)
    session = sessions[0]
    client_ip = session.get('client_ip')
    user_name = session.get('user_name')
    if session.get('status') == 0:
        raise HTTPException(status_code=400, detail="The user is offline")
    logger.info(f"Client ip: {client_ip}, user name: {user_name}")


@exter_attack
def get_token(pre: AuthBase):
    url = "https://127.0.0.1:8088/api/v2/aa/sessions"
    headers = {
        "Content-type": "application/json;charset=UTF-8",
        "X-Real-IP": "9.8.7.6"
    }
    data = {
        "scope": 0,
        "user_name": pre.user_name,
        "password": pre.password
    }
    r = requests.post(url=url, headers=headers, json=data, verify=False)
    if r.status_code != requests.codes.ok or r.json().get('result').get('code') != 0:
        return None, r.json().get('result', {}).get('description', None)
    x_auth_token = r.json().get('data').get('x_auth_token')
    return x_auth_token, f"successfully get token:{x_auth_token}."


@exter_attack
def create_pms_namespace(token, pre: CreateNamespaceRequest):
    url = "http://127.0.0.1:8778/cfgapi/v2/converged_service/pms_namespace"
    headers = {
        "X-Auth-Token": token
    }
    data = {
        "account_id": pre.account_id,
        "name": pre.namespace_name,
        "storage_pool_id": pre.pool_id,
        "fsusage_type": 4
    }
    r = requests.post(url=url, headers=headers, json=data, verify=False)
    if r.status_code != requests.codes.ok or r.json().get('result').get('code') != 0:
        logger.error(f'Failed to create pms namespace, return error: {r.text}.')
        return None, r.json().get('result', {}).get('description', None)

    namespace_id = r.json().get('data').get('id')
    logger.info(f'Successfully create pms namespace, namespace id is {namespace_id}.')
    return namespace_id, f'Successfully create pms namespace, namespace id is {namespace_id}.'


@exter_attack
def get_pms_namespace(token, pre: GetNamespaceRequest):
    url = "http://127.0.0.1:8778/api/v2/converged_service/namespaces"
    headers = {
        "X-Auth-Token": token
    }
    data = {
        "name": pre.namespace_name,
        "account_id": pre.account_id
    }
    r = requests.get(url=url, headers=headers, json=data, verify=False)
    logger.info(f'get pms namespace*******  {r.json()}')
    if r.status_code == requests.codes.ok:
        if r.json().get('result').get('code') == 0:
            namespace_id = r.json().get('data').get('id')
            logger.info(f'Successfully get pms namespace, namespace id is {namespace_id}.')
            return (COMMAND_SUCCESS,
                    str(namespace_id), f'Successfully get pms namespace, namespace id is {namespace_id}.')
        if r.json().get('result').get('code') == GET_NAMESPACE_NOT_EXIST:
            logger.info(f'Namespace {pre.namespace_name} not exist.')
            return COMMAND_FAILED, None, f'Namespace {pre.namespace_name} not exist.'
    logger.error(f'Failed to get pms namespace, return error: {r.text}')
    return COMMAND_WRONG, None, r.json().get('result', {}).get('description', None)


@exter_attack
def delete_pms_namespace(token, pre: DeleteNamespaceRequest):
    url = "http://127.0.0.1:8778/cfgapi/v2/converged_service/pms_namespace"
    headers = {
        "X-Auth-Token": token
    }
    data = {
        "name": pre.namespace_name,
        "account_id": pre.account_id
    }
    r = requests.delete(url=url, headers=headers, json=data, verify=False)
    if r.status_code != requests.codes.ok or r.json().get('result').get('code') != 0:
        logger.error(f'Failed to delete pms namespace, return error: {r.text}.')
        return COMMAND_FAILED, r.json().get('result', {}).get('description', None)
    logger.info(f'Successfully delete pms namespace, namespace name is {pre.namespace_name}.')
    return COMMAND_SUCCESS, f'Successfully delete pms namespace, namespace name is {pre.namespace_name}.'


@exter_attack
def create_pms_nfs(token, pre: CreateNFSShareRequest):
    url = "http://127.0.0.1:8778/cfgapi/v2/nas_protocol/pms_nfs_share"
    headers = {
        "X-Auth-Token": token,
        "Content-Type": "application/json"
    }
    data = {
        "account_id": pre.account_id,
        "file_system_id": pre.namespace_id,
        "share_path": "/" + pre.namespace_name
    }
    r = requests.post(url=url, headers=headers, json=data, verify=False)
    logger.info(f'create pms nfs share  {r.json()}, data share path is {data.get("share_path")}')
    if r.status_code != requests.codes.ok or r.json().get('result').get('code') != 0:
        logger.error(f'Failed to create pms nfs share, return error: {r.text}.')
        return None, r.json().get('result', {}).get('description', None)
    nfs_id = r.json().get('data').get('id')
    logger.info(f'Successfully create pms nfs, nfs id is {nfs_id}.')
    return nfs_id, f'Successfully create pms nfs, nfs id is {nfs_id}.'


@exter_attack
def get_pms_nfs(token, pre: GetNFSShareRequest):
    url = "http://127.0.0.1:8778/api/v2/nas_protocol/nfs_share_list"
    headers = {
        "X-Auth-Token": token
    }
    data = {
        "filter": [{"fs_id": pre.namespace_id}],
        "account_id": pre.account_id
    }
    r = requests.get(url=url, headers=headers, json=data, verify=False)
    logger.info(f'get pms nfs share  {r.json()}')
    if r.status_code != requests.codes.ok or r.json().get('result').get('code') != 0:
        logger.error(f'Failed to get pms nfs share, return error: {r.text}.')
        return COMMAND_WRONG, None, r.json().get('result', {}).get('description', None)
    # namespace id : file_system_id
    # fsm 即使不存在nfs，返回结果仍然是 status code = requests.codes.ok, result code = 0，所以只能再下面再加判断，判断data是否为空。
    # fsm get pms nfs 返回data为 list 类型。
    if len(r.json().get('data')) == 0:
        logger.info(f'Failed to get pms nfs share, return error: {r.text}.')
        return COMMAND_FAILED, None, r.json().get('result', {}).get('description', None)
    nfs_id = r.json().get('data')[0].get('id')
    logger.info(f'Successfully get pms nfs, nfs id is {nfs_id}.')
    return COMMAND_SUCCESS, str(nfs_id), f'Successfully get pms nfs, nfs id is {nfs_id}.'


@exter_attack
def delete_pms_nfs(token, pre: DeleteNFSShareRequest):
    url = "http://127.0.0.1:8778/cfgapi/v2/nas_protocol/pms_nfs_share"
    headers = {
        "X-Auth-Token": token
    }
    data = {
        "id": pre.nfs_id,
        "account_id": pre.account_id
    }
    r = requests.delete(url=url, headers=headers, json=data, verify=False)
    if r.status_code != requests.codes.ok or r.json().get('result').get('code') != 0:
        logger.error(f'Failed to delete pms nfs share, return error: {r.text}.')
        return COMMAND_FAILED, r.json().get('result', {}).get('description', None)
    logger.info(f'Successfully delete pms nfs, nfs id is {pre.nfs_id}.')
    return COMMAND_SUCCESS, f'Successfully delete pms nfs, nfs id is {pre.nfs_id}.'


@exter_attack
def create_pms_client(token, pre: CreatNFSClientRequest):
    url = "http://127.0.0.1:8778/cfgapi/v2/nas_protocol/pms_nfs_share_auth_client"
    headers = {
        "X-Auth-Token": token
    }
    data = {
        "access_name": pre.client_name,
        "share_id": pre.nfs_id,
        "account_id": pre.account_id,
        "all_squash": "1",
        "root_squash": "1",
        "sync": "0",
        "access_value": "1"
    }
    logger.info(f"create pms client, data is {data}")
    r = requests.post(url=url, headers=headers, json=data, verify=False)
    logger.info(f'create_pms_client*******  {r.json()}')
    if r.status_code != requests.codes.ok or r.json().get('result').get('code') != 0:
        logger.error(f'Failed to create pms client, return error: {r.text}')
        return None, r.json().get('result', {}).get('description', None)
    url = "http://127.0.0.1:8778/api/v2/nas_protocol/nfs_share_auth_client_list"
    r = requests.get(url, headers=headers, json={
        'filter': [{'share_id': pre.nfs_id}],
        'account_id': pre.account_id
    }, verify=False)
    logger.info(f'get pms client list*******  {r.json()}')

    if r.status_code != requests.codes.ok or r.json().get('result').get('code') != 0:
        logger.error(f'Failed to get nfs auth client list, return:{r.text}')
        return None, r.json().get('result', {}).get('description', None)
    infos = r.json().get('data')
    for info in infos:
        if info['share_id'] == pre.nfs_id:
            logger.info(f"Successfully create pms client, client id is {info['id']}.")
            return info['id'], f"Successfully create pms client, client id is {info['id']}."
    logger.error(
        f'Failed to create pms client, not found pms client id, nfs id is {pre.nfs_id}, return error: {r.text}')
    return None, f'Failed to create pms client, not found pms client id, nfs id is {pre.nfs_id}.'


@exter_attack
def get_pms_client(token, pre: GetNFSClientRequest):
    headers = {
        "X-Auth-Token": token
    }
    url = "http://127.0.0.1:8778/api/v2/nas_protocol/nfs_share_auth_client_list"
    r = requests.get(url, headers=headers, json={
        'filter': [{'share_id': pre.nfs_id}],
        'account_id': pre.account_id
    }, verify=False)
    if r.status_code != requests.codes.ok or r.json().get('result').get('code') != 0:
        logger.error(f'Failed to get nfs auth client list, return:{r.text}')
        return COMMAND_WRONG, None, r.json().get('result', {}).get('description', None)
    infos = r.json().get('data')
    for info in infos:
        if info['share_id'] == pre.nfs_id:
            logger.info(f"Successfully get pms client, client id is {info['id']}.")
            return COMMAND_SUCCESS, str(info['id']), f"Successfully get pms client, client id is {info['id']}."
    logger.info(
        f'Failed to get pms client, not found pms client id, nfs id is {pre.nfs_id}, return error: {r.text}')
    return COMMAND_FAILED, None, f'Failed to get pms client, not found pms client id, nfs id is {pre.nfs_id}.'


@exter_attack
def delete_pms_client(token, pre: DeleteNFSClientRequest):
    url = "http://127.0.0.1:8778/cfgapi/v2/nas_protocol/pms_nfs_share_auth_client"
    headers = {
        "X-Auth-Token": token
    }
    data = {
        "id": pre.client_id,
        "account_id": pre.account_id
    }
    r = requests.delete(url=url, headers=headers, json=data, verify=False)
    if r.status_code != requests.codes.ok or r.json().get('result').get('code') != 0:
        logger.error(f'Failed to delete pms client, return error: {r.text}')
        return COMMAND_FAILED, r.json().get('result', {}).get('description', None)
    logger.info(
        f'Successfully delete pms client, client id :{pre.client_id}, account_id:{pre.account_id}.')
    return COMMAND_SUCCESS, f'Successfully delete pms client, client id :{pre.client_id}, account_id:{pre.account_id}.'


@exter_attack
def get_secondary_float_ip(token):
    url = "https://127.0.0.1:8088/api/v2/console/secondary_float_ip"
    headers = {
        "X-Auth-Token": token
    }
    r = requests.get(url=url, headers=headers, verify=False)
    if r.status_code != requests.codes.ok or r.json().get('result').get('code') != 0:
        logger.error(f"Failed to get secondary float ip,return error: {r.text}.")
        return None, r.json().get('result', {}).get('description', None)
    secondary_float_ip = r.json().get('data').get('secondary_external_service_float_ip')
    logger.info(f'Successfully get secondary float ip: {secondary_float_ip}.')
    return secondary_float_ip, f'Successfully get secondary float ip: {secondary_float_ip}.'


@exter_attack
def get_hostname():
    with open('/etc/hostname') as f:
        return f.read().strip()


@exter_attack
def get_interface_from_ip(ip:str):
    cmd = f"ifconfig | grep -B1 {ip}"
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True, check=True)
    except subprocess.CalledProcessError as e:
        logger.info(F"Successfully get ip {ip} correspond network interface card.")
        return 1, e
    ip_name = result.stdout.split(":", 1)[0]
    return 0, ip_name

@exter_attack
def umount():
    mount.mount_main(reset=True)
    logger.info(f"Successfully umount")
    return COMMAND_SUCCESS, None
