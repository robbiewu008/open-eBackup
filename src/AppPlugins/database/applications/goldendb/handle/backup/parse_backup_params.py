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

import collections
import json
import os
import time

import pexpect
from goldendb.logger import log
from common.cleaner import clear
from common.const import BackupTypeEnum
from common.parse_parafile import get_env_variable
from common.util.exec_utils import exec_append_file
from common.util.scanner_utils import scan_dir_size
from goldendb.handle.common.const import MasterSlavePolicy

BackupParams = collections.namedtuple('BackupParams',
                                      ['cluster_id', 'backup_type_str', 'master_or_slave', 'cluster_user'])


class GoldenDBStructure:
    # GoldenDB集群结构体类
    def __init__(self, cluster_info):
        self.manager_nodes = []
        self.gtm_nodes = []
        self.data_nodes = {}
        self.cluster_id = cluster_info["id"]
        self.group_number = len(cluster_info["group"])

    def add_manager_node(self, node_info):
        self.manager_nodes.append(node_info)

    def add_gtm_node(self, node_info):
        self.gtm_nodes.append(node_info)

    def add_dn_nodes(self, group_id, user, master_or_slave, agent_id, node_type):
        if group_id not in self.data_nodes.keys():
            self.data_nodes[group_id] = {}
            self.data_nodes[group_id][master_or_slave] = [[user, master_or_slave, agent_id, node_type]]
        else:
            if master_or_slave not in self.data_nodes[group_id].keys():
                self.data_nodes[group_id][master_or_slave] = [[user, master_or_slave, agent_id, node_type]]
            else:
                self.data_nodes[group_id][master_or_slave].append([user, master_or_slave, agent_id, node_type])


def get_goldendb_structure(file_content):
    # 从解析出的配置文件中获取GoldenDB集群结构信息
    cluster_info = json.loads(
        file_content.get("job", {}).get("protectObject", {}).get("extendInfo", {}).get("clusterInfo", ""))
    goldendb_info = json.loads(
        file_content.get("job", {}).get("protectEnv", {}).get("extendInfo", {}).get("GoldenDB", ""))
    cluster_structure = GoldenDBStructure(cluster_info)

    dn_group_infos = cluster_info["group"]
    for groups in dn_group_infos:
        group_id = groups["groupId"]

        for node in groups["mysqlNodes"]:
            user = node["osUser"]
            master_or_slave = node["role"]
            agent_id = node["parentUuid"]
            cluster_structure.add_dn_nodes(group_id, user,
                                           master_or_slave, agent_id, "dataNode")
    gtm_group_infos = cluster_info["gtm"]
    for gtm_node in gtm_group_infos:
        cluster_structure.add_gtm_node([gtm_node["osUser"], "master", gtm_node["parentUuid"], gtm_node["nodeType"]])
    manager_nodes = goldendb_info["nodes"]
    for manager_node in manager_nodes:
        cluster_structure.add_manager_node([manager_node["osUser"], "master", manager_node["parentUuid"],
                                            manager_node["nodeType"]])
    log.info("get the goldendb structure success")
    return cluster_structure


def check_goldendb_structure(master_or_slave, cluster_structure):
    # 检查当前goldendb结构是否符合主备策略
    for group in cluster_structure.data_nodes.values():
        if master_or_slave not in group.keys() or len(group[master_or_slave]) == 0:
            log.info(f"cluster_info {str(cluster_structure.data_nodes)}")
            return False
    log.info(f'check goldendb structure')
    return True


def get_master_or_slave_policy(file_content):
    # 获取SLA主备策略
    sla = file_content.get('job', {}).get('extendInfo', {}).get('slave_node_first', "false")
    log.info(f"sla {str(sla)}")
    if sla == 'false':
        return MasterSlavePolicy.MASTER
    return MasterSlavePolicy.SLAVE


def write_file(file_name, message):
    if os.path.islink(file_name):
        log.error(f"Link file:{file_name},stop writing.")
        return
    exec_append_file(file_name, message)


def query_size_and_speed(time_file, data_path, original_size, job_id):
    size = 0
    speed = 0
    if os.path.islink(time_file):
        log.error(f"Link file:{time_file},stop writing.")
        return size, speed
    if not os.path.exists(time_file):
        log.error(f"Time file: {time_file} not exist")
        return size, speed
    log.info('Time path exist')
    with open(time_file, "r", encoding='UTF-8') as file:
        start_time = file.read().strip()
    ret, data_size = scan_dir_size(job_id, data_path)
    if ret:
        size = data_size
    new_time = int((time.time()))
    try:
        speed = int((size - original_size) / (new_time - int(start_time)))
    except Exception:
        log.error("Error while calculating speed! time difference is 0!")
        return 0, 0
    return size, speed


def exec_cmd_spawn(cmd, pid):
    try:
        child = pexpect.spawn(cmd, timeout=None)
    except Exception as exception_str:
        log.error(f"Spawn except an exception {exception_str}.")
        return 1, "cmd error"
    try:
        ret_code = child.expect(
            ["please input password: ", "Failed to connect to GoldenDB server", pexpect.TIMEOUT, pexpect.EOF])
    except Exception as exception_str:
        # 此处不打印异常，异常会显示用户名
        log.error(f"Exec cmd except an exception {exception_str}.")
        child.close()
        return 1, "cmd error"
    if ret_code != 0:
        log.error(f"Exec cmd failed.")
        child.close()
        return 1, "cmd error"
    passwd = get_env_variable(f"job_protectObject_auth_authPwd_{pid}")

    if not passwd:
        child.close()
        return False, "get passwd failed"
    child.sendline(passwd)
    clear(passwd)
    try:
        out_str = child.read()
    except Exception as exception_str:
        log.error(f"Exec cmd except an exception {exception_str}.")
        child.close()
        return 1, "cmd error"
    child.close()
    return child.exitstatus, str(out_str)


def get_backup_param(req_id, file_content, sla_policy):
    # 获取备份参数
    cluster_info = json.loads(file_content["job"]["protectObject"]["extendInfo"]["clusterInfo"])
    cluster_id = cluster_info["id"]
    backup_type_str = ""
    master_or_slave = "-master"
    backup_type = file_content['job']['jobParam']['backupType']
    if backup_type == BackupTypeEnum.FULL_BACKUP:
        backup_type_str = "-full"

    if sla_policy != MasterSlavePolicy.MASTER:
        master_or_slave = ""
    cluster_user = get_env_variable(f"job_protectObject_auth_authKey_{req_id}")
    bkp_params = BackupParams(cluster_id, backup_type_str, master_or_slave, cluster_user)
    return bkp_params


def get_copy_result_info(data_path, cluster_id):
    task_id_path = os.path.join(data_path, f"DBCluster_{cluster_id}/DATA_BACKUP")
    task_ids = []
    for file in os.listdir(task_id_path):
        abs_path = os.path.join(task_id_path, file)
        if os.path.isdir(abs_path):
            task_ids.append(file)
    task_ids.sort()
    task_id = task_ids[-1]
    resultinfo_name = ""
    resultinfo_path = os.path.join(task_id_path, task_id, "ResultInfo")
    for file in os.listdir(resultinfo_path):
        if "backup_resultsinfo" in file and 'audit' not in file:
            resultinfo_name = file
            break
    return resultinfo_name, task_id
