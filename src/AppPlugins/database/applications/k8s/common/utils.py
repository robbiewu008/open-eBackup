#
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
#

import json
import math

import os
import re

import stat
import uuid

from pydantic.types import UUID4

from common.cleaner import clear
from common.common import execute_cmd, check_path_legal
from common.const import ParamConstant, CMDResult, SysData
from common.util.exec_utils import exec_overwrite_file, exec_mkdir_cmd, exec_write_file_without_x_permission
from k8s.common.const import PATH_WHITE_LIST, PVC_PENDING_TIMEOUT, OCEAN_PROTECT_GROUP
from k8s.logger import log
from k8s.common.kubernetes_client.struct import ResourceInfo, AuthType


def get_uuid_last_section(uid: UUID4):
    return str(uid).split('-')[-1]


def get_id_last_section(task_id: str):
    return task_id.split('-')[-1]


def k8s_size_to_byte(size: str):
    if 'i' not in size:
        return int(size)
    else:
        size_len = len(size) - 2
        size_unit = size[size_len:len(size)]
        size_map = dict()
        size_map['Ei'] = 6
        size_map['Pi'] = 5
        size_map['Ti'] = 4
        size_map['Gi'] = 3
        size_map['Mi'] = 2
        size_map['Ki'] = 1
        if size_unit in size_map.keys():
            return int(size[0:size_len]) * math.pow(1024, size_map[size_unit])
        else:
            return 0


def save_resource_to_yaml(meta_path, resource: ResourceInfo):
    if not resource.groups:
        api_version = resource.version
        resource_group = OCEAN_PROTECT_GROUP
    else:
        api_version = '/'.join([resource.groups, resource.version])
        resource_group = resource.groups
    log.debug(f'Api_version is {api_version}')
    metapath_prefix = f'{meta_path}/namespace/{resource.namespace}/{resource_group}/{resource.kind}' \
        if resource.namespace else f'{meta_path}/cluster/{resource_group}/{resource.kind}'

    log.info(f"metapath_prefix {metapath_prefix}")
    if not prepare_kubernetes_meta_data_store_path(metapath_prefix):  # 准备仓库
        return False

    # 写入kind metadata
    kind_data = dict()
    kind_data.update({'group': resource.groups})
    kind_data.update({'version': resource.version})
    kind_data.update({'namespace': resource.namespace})
    kind_data.update({'plural': resource.plural})
    kind_data.update({'kind': resource.kind})
    kind_path = f'{metapath_prefix}/kind.metadata'
    write_json_content_into_file(json.dumps(kind_data), kind_path)

    # 写入各资源yaml
    for data in resource.items:
        data.update({'apiVersion': api_version})
        data.update({'kind': resource.kind})
        if "metadata" not in data:
            log.error(f"Resource have no metadata info")
            return False
        if "managedFields" in data.get("metadata"):
            data.get("metadata").pop("managedFields")
        if "managedFields" in data:
            data.pop("managedFields")
        data.get("metadata").pop("resourceVersion", None)

        log.debug(f'Backup metadata body: {data}')
        name = re.sub('[/:|<>?*"\\\\]', '', data["metadata"]["name"])
        file_path = f'{metapath_prefix}/{name}.json'
        write_json_content_into_file(json.dumps(data), file_path)
    return True


def prepare_kubernetes_meta_data_store_path(meta_data_dir):
    try:
        if not os.path.exists(meta_data_dir):
            log.info(f"Create metadata store path: {meta_data_dir}")
            return exec_mkdir_cmd(meta_data_dir)
        else:
            log.debug(f"Metadata store path: {meta_data_dir}, already exist")
    except Exception as e:
        log.exception(f'Prepare dir failed: {e}', exc_info=True)
        return False
    return True


def write_json_content_into_file(data, file_path):
    exec_write_file_without_x_permission(file_path, data, json_flag=False)
    log.warning(f'Backup metadata:{file_path.split("/")[-1]}')
    return True


def exec_rc_tool_cmd(cmd, in_param, unique_id):
    """
    执行rc_tool命令
    @@param cmd: 需要执行的命令
    @@param in_param: 需要写入输入文件的命令参数
    @@param unique_id: 输入输出文件唯一标识
    @@return result:bool 命令执行结果
    @@return output:string 命令输出
    """
    random_id = str(uuid.uuid4())
    input_file_path = os.path.join(ParamConstant.PARAM_FILE_PATH,
                                   ParamConstant.INPUT_FILE_PREFFIX + unique_id + random_id)
    output_file_path = os.path.join(ParamConstant.RESULT_PATH,
                                    ParamConstant.OUTPUT_FILE_PREFFIX + unique_id + random_id)
    try:
        write_tmp_file(in_param, input_file_path)
    except Exception as exception_str:
        log.error(f"Write param file exception. {exception_str}")
        if os.path.exists(input_file_path):
            os.remove(input_file_path)
        return False, ""
    cmd = f"{os.path.join(ParamConstant.BIN_PATH, 'rpctool.sh')} {cmd} {input_file_path} {output_file_path}"
    ret, out, err = execute_cmd(cmd)
    if os.path.exists(input_file_path):
        os.remove(input_file_path)
    if ret != CMDResult.SUCCESS:
        log.error(f"An error occur in execute cmd. ret:{ret}. out: {out} err:{err}")
        return False, ""
    output = read_json_file(output_file_path)
    if os.path.exists(output_file_path):
        os.remove(output_file_path)
    if not output:
        log.error(f"Output is null.")
        return False, ""
    return True, output


def read_json_file(file_path):
    if not os.path.isfile(file_path):
        raise Exception(f"File not exist")
    try:
        with open(file_path, "r", encoding='UTF-8') as file_obj:
            json_dict = json.loads(file_obj.read())
    except Exception as exception:
        raise Exception("parse param file failed") from exception
    return json_dict


def write_tmp_file(context, file_path):
    path_white_list = [ParamConstant.PARAM_FILE_PATH, ParamConstant.RESULT_PATH]
    for single_path in path_white_list:
        if check_path_legal(file_path, single_path):
            exec_overwrite_file(file_path, context)
            return
            # 传进来的path不在/tmp/目录下不合法
    raise Exception(f"File not legal")


def read_file_in_dir(path):
    if os.path.exists(path):
        file_list = os.listdir(path)
        log.info(f'File name is {file_list}')
        return file_list
    return []


def check_white_list(check_paths):
    for check_path in check_paths:
        if not re.match(PATH_WHITE_LIST, check_path):
            log.error(f"Path: {check_path} is not in the white list.")
            raise Exception(f"Path: {check_path} is not in the white list.")
    log.info('Check white list complete.')


def validate_ip_str(ip: str):
    return (ip is not None) and ([True] * 4 == [x.isdigit() and 0 <= int(x) <= 255 for x in ip.split(".")])


def validate_port_str(port: str):
    return (port is not None) and (1024 < int(port) <= 65535)


def validate_auth_str(auth_info: str):
    return auth_info is not None and auth_info != ""


def validate_cmd_strs(strs):
    if type(strs) == int:
        return True
    strs = strs.replace('-', '')
    strs = strs.replace('.', '')
    return strs.isalnum()


def get_env_variable(str_env_variable: str):
    env_variable = ''
    input_str = json.loads(SysData.SYS_STDIN)
    if input_str.get(str_env_variable):
        env_variable = input_str.get(str_env_variable)
    return env_variable


def clean_auth_info(auth_info):
    if not _check_before_clean_auth(auth_info):
        return
    try:
        _clean_str(auth_info.is_verify_ssl)
        if auth_info.auth_type == AuthType.TOKEN:
            _clean_str(auth_info.id)
            _clean_str(auth_info.token.token_info)
            _clean_str(auth_info.token.port)
            _clean_str(auth_info.token.address)
            _clean_str(auth_info.token.certificateAuthorityData)
            _clean_str(auth_info.id)
        elif auth_info.auth_type == AuthType.CONFIGFILE:
            _clean_dict(auth_info.kube_config)
    except Exception as err:
        raise Exception from err


def clean_ssl_file(auth_info):
    try:
        input_file_path = os.path.join(ParamConstant.PARAM_FILE_PATH,
                                       ParamConstant.INPUT_FILE_PREFFIX + auth_info.id + ".pem")
        if os.path.exists(input_file_path):
            os.remove(input_file_path)
            log.info(f"clean ssl file{auth_info.id}")
    except Exception as err:
        raise Exception from err


def clean_sys_data():
    clear(SysData.SYS_STDIN)


def _check_before_clean_auth(auth_info):
    if not auth_info:
        return False
    if auth_info.auth_type == AuthType.TOKEN:
        if not auth_info.token:
            return False
    if auth_info.auth_type == AuthType.CONFIGFILE:
        if not auth_info.kube_config:
            return False
    return True


def _clean_dict(config):
    for key, value in config.items():
        if isinstance(value, dict):
            _clean_dict(value)
        elif isinstance(value, list):
            _clean_list(value)
        else:
            _clean_str(config[key])
        _clean_str(key)


def _clean_list(lists):
    for item in iter(lists):
        if isinstance(item, dict):
            _clean_dict(item)
        elif isinstance(item, list):
            _clean_list(item)
        else:
            _clean_str(item)


def _clean_str(secret):
    if not isinstance(secret, str) or len(secret) == 1:
        return
    clear(secret)


def get_task_timeout(timeout: str, default_timeout=PVC_PENDING_TIMEOUT):
    try:
        timeout = json.loads(timeout)
        days = int(timeout.get("days", 0))
        hours = int(timeout.get("hours", 0))
        minutes = int(timeout.get("minutes", 0))
        seconds = int(timeout.get("seconds", 0))
        task_timeout = 24 * 60 * 60 * days + 60 * 60 * hours + 60 * minutes + seconds
    except Exception as e:
        log.exception(f"Parse time error, err is {e}.")
        task_timeout = 0
    return task_timeout if task_timeout > 0 else default_timeout
