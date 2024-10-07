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
import os
import re
import shutil
import signal
import sqlite3
import stat
import time
from copy import deepcopy

from common.common import exter_attack
from common.common import output_execution_result, output_execution_result_ex, check_command_injection
from common.common_models import ActionResult, LogDetail
from common.common_models import SubJobDetails
from common.const import AuthType, ReportDBLabel
from common.const import BackupTypeEnum, SubJobStatusEnum, RepositoryDataTypeEnum
from common.const import JobData, DBLogLevel
from common.job_const import ParamKeyConst
from common.logger import Logger
from common.parse_parafile import ParamFileUtil
from common.util.scanner_utils import scan_dir_size
from dameng.commons.check_information import check_dmapserver_status
from dameng.commons.common import check_ip_in_local, del_space, del_space_in_list, get_hostsn, \
    check_path_in_white_list, get_env_value, invoke_rpc_tool_interface, timestamp
from dameng.commons.const import BackupStepEnum, BackupSubType, ClusterNodeMode, Progress, BackupJobResult, \
    DMJsonConstant
from dameng.commons.const import DbStatus, DamengStrConstant, MAX_LSN, BACKUP_MOUNT_PATH, ActionResultCode, \
    RpcParamKey, LastCopyType
from dameng.commons.const import SubJobType, SubJobPolicy, SubJobPriority, CheckBackupJobTypeCode, \
    ArrayIndex, DbArchStatus, DbAuthInfo, ErrCode, JobProgressLabel, DamengStrFormat
from dameng.commons.dameng_tool import DmSqlTool
from dameng.commons.path_operation import umount, backup_mount_bind, dameng_user_mkdir
from dameng.commons.query_information import query_db_status, show_backupset, query_process_info, get_oguid, \
    query_db_arch_status, get_primary_node_name
from dameng.resource.dameng_cluster import DamengCluster
from dameng.resource.damengsource import DamengSource

log = Logger().get_logger("dameng.log")


class BackUp:

    def __init__(self, backup_type, job_id, json_param, sub_job_id):
        self._sub_job_id = sub_job_id
        self._job_id = job_id
        self._json_param = json_param
        self._db_type = ''
        self.log_detail_errcode = None
        self.log_detail_param = None
        self.backupset_path = ''
        self.json_dict = {}
        self.begin_time = None
        self.end_time = None
        self._port = 0
        self._auth_mode = 0
        self.version = None
        self._auth_info_status = DbAuthInfo.RIGHT
        self.cur = None
        self._copy_id = ''
        self.progress = Progress.RUNNING
        self._pwd = ""
        self.channel_number = ''
        self._user = ""
        self._ip = ""
        self._db_info = {}
        self._backup_type = backup_type
        self._repositories_info = dict()
        self._data_area = ""
        self._db_status = DbStatus.DB_STATUS_OPEN
        self._arch_status = DbArchStatus.OPEN_ARCH
        self._meta_area = ""
        self._cache_area = ""
        self._log_area = ""
        self._log_arch = ""
        self._backup_dir = ""
        self._history_cout = 0
        self._backup_prerequisite_dic = {}
        self._primary_node_num = 0

    @exter_attack
    def backup_prerequisite(self):
        """
        前置任务
        :return:
        """
        result = {
            "status": SubJobStatusEnum.RUNNING.value,
            "err_code": None,
            "err_code_param": '',
        }
        self._cache_area = self.get_area_info("cache_repository")
        if not self._cache_area:
            log.info(f"Get cache area fail.{self.get_log_common()}")
            return ActionResult(code=200).dict(by_alias=True)
        file_path = os.path.join(self._cache_area, f'backup_prerequisite_status_{self._job_id}')
        output_execution_result_ex(file_path, result)
        ret, err_param = self.check_nodes_distribution()
        if not ret:
            result["status"] = SubJobStatusEnum.FAILED.value
            if err_param:
                result["err_code"] = ErrCode.ERR_NODES_DISTRIBUTION_EXCEPTION
                result["err_code_param"] = err_param
        else:
            result["status"] = SubJobStatusEnum.COMPLETED.value
        output_execution_result_ex(file_path, result)
        return ActionResult(code=0).dict(by_alias=True)

    def get_log_common(self):
        return f"Pid: {JobData.PID} jobId: {self._job_id} subjobId: {self._sub_job_id}."

    def set_db_param(self):

        out_info = {
            "code": 1,
            "bodyErr": None,
            "message": ""
        }
        object_info = self._json_param.get("job", {}).get("protectObject", {})
        sub_type = object_info.get("subType", '')
        if sub_type == BackupSubType.SINGLE_NODE:
            self._db_type = sub_type
            if self.set_single_info():
                out_info["code"] = 0
        elif sub_type == BackupSubType.CLUSTER:
            self._db_type = sub_type
            if self.set_cluster_info():
                out_info["code"] = 0
        else:
            log.error(f"Unknown sub type: {sub_type}.{self.get_log_common()}.")
        return out_info

    def get_node_real_ip(self):
        """
        op服务化时，适配内大网ip
        """
        self._ip = self._json_param.get("job", {}).get("protectEnv", {}).get("endpoint", "")
        agents_info = self._json_param.get("job", {}).get("extendInfo", {}).get("agents", [])
        agent_hostsn = get_hostsn()
        for tmp_agent in agents_info:
            tmp_agent_id = tmp_agent.get("id", "")
            tmp_advance_params = tmp_agent.get("advanceParams", {})
            if tmp_advance_params:
                sub_ip = tmp_advance_params.get("subNetFixedIp", "")
                if agent_hostsn == tmp_agent_id and sub_ip:
                    self._ip = sub_ip
                    break
                elif agent_hostsn == tmp_agent_id and not sub_ip:
                    log.warn(f"The ip of the sub net is not found, agent_hostsn; {agent_hostsn}.")

    def set_single_info(self):
        object_info = self._json_param.get("job", {}).get("protectObject", {})
        self.get_node_real_ip()
        self._port = object_info.get("extendInfo", {}).get("port", 0)
        if (not self._ip) or (not self._port):
            log.error(f"Failed get ip or port.{self.get_log_common()}.")
            return False
        self._auth_mode = get_env_value(f"job_protectObject_auth_authType_{JobData.PID}")
        big_version = object_info.get("extendInfo", {}).get("bigVersion", [])
        if self._auth_mode == str(AuthType.OS_PASSWORD.value):
            self._db_info = {
                "port": self._port,
                "auth_type": AuthType.OS_PASSWORD,
                "single_or_cluser": "single",
                "big_version": big_version
            }
        elif self._auth_mode == str(AuthType.APP_PASSWORD.value):
            self._user = f"{DamengStrConstant.KEY_PREFIX}_auth_authKey_{JobData.PID}"
            self._pwd = f"{DamengStrConstant.KEY_PREFIX}_auth_authPwd_{JobData.PID}"
            self._db_info = {
                "port": self._port,
                "auth_type": AuthType.APP_PASSWORD,
                "userkey": self._user,
                "pwdkey": self._pwd,
                "single_or_cluser": "single",
                "big_version": big_version
            }
        else:
            log.error(f"Auth type param error.{self.get_log_common()}.")
            return False
        if not check_ip_in_local(self._ip):
            log.error(f"The task cannot be executed on the current node.{self.get_log_common()}.")
            return False
        status, mode = query_db_status(self._db_info)
        if not status and not mode:
            self._auth_info_status = DbAuthInfo.ERRORS
            return False
        if status != DbStatus.DB_STATUS_OPEN:
            log.error(f"Db status not open.{self.get_log_common()}.")
            self._db_status = DbStatus.DB_STATUS_CLOSE
            return False
        return True

    def set_cluster_info(self):
        env_info = self._json_param.get("job", {}).get("protectEnv", {})
        nodes = env_info.get("nodes", [])
        if not nodes:
            log.error(f"Failed get nodes from env.{self.get_log_common()}.")
            return False
        node_index = 0

        for node in nodes:
            self._ip = node.get("endpoint", "")
            if not check_ip_in_local(self._ip):
                node_index += 1
                continue
            self._port = node.get("extendInfo", {}).get("port", 0)
            self._auth_mode = get_env_value(f"job_protectEnv_nodes_{node_index}_auth_authType_{JobData.PID}")
            big_version = env_info.get("extendInfo", {}).get("bigVersion", [])
            if int(self._auth_mode) == AuthType.OS_PASSWORD.value:
                db_info = {
                    "port": self._port,
                    "auth_type": AuthType.OS_PASSWORD,
                    "single_or_cluser": "cluser",
                    "big_version": big_version
                }
            elif int(self._auth_mode) == AuthType.APP_PASSWORD.value:
                self._user = f"job_protectEnv_nodes_{node_index}_auth_authKey_{JobData.PID}"
                self._pwd = f"job_protectEnv_nodes_{node_index}_auth_authPwd_{JobData.PID}"
                db_info = {
                    "port": self._port,
                    "auth_type": AuthType.APP_PASSWORD,
                    "userkey": self._user,
                    "pwdkey": self._pwd,
                    "single_or_cluser": "cluser",
                    "big_version": big_version
                }
            else:
                log.error(f"AuthType param error.{self.get_log_common()}.")
                return False
            node_index += 1
            status, mode = query_db_status(db_info)
            if not status and not mode:
                self._auth_info_status = DbAuthInfo.ERRORS
                return False

            if status != DbStatus.DB_STATUS_OPEN:
                log.error(f"Db status not open.{self.get_log_common()}.")
                self._db_status = DbStatus.DB_STATUS_CLOSE
                return False
            if mode == ClusterNodeMode.NODE_MODE_PRIMARY:
                self._db_info = db_info
                return True
        return False

    def build_sub_job(self, job_priority, exec_node_id, job_info, job_name):
        """
        填充子任务信息
        :return:
        """

        sub_job_info = dict()
        sub_job_info["jobId"] = self._job_id
        sub_job_info["subJobId"] = ""
        sub_job_info["jobType"] = SubJobType.BUSINESS_SUB_JOB
        sub_job_info["jobName"] = job_name
        sub_job_info["jobPriority"] = job_priority
        sub_job_info["policy"] = SubJobPolicy.FIXED_NODE
        sub_job_info["ignoreFailed"] = False
        sub_job_info["execNodeId"] = exec_node_id
        sub_job_info["jobInfo"] = job_info
        return sub_job_info

    @exter_attack
    def generator_sub_job(self):
        """
        拆分子任务
        :return:
        """
        log.info(f"Start gen sub job.")
        backup_sub_type = self._json_param.get("job", {}).get("protectObject", {}).get("subType", "")
        if backup_sub_type != BackupSubType.CLUSTER:
            log.error(f"The database type does not support subtask splitting."
                      f"{self.get_log_common()}.")
            return []
        if not self.set_cluster_info():
            return []
        # 1. 获取集群所有节点信息
        nodes = self._json_param.get("job", {}).get("protectEnv", {}).get("nodes", [])
        if not nodes:
            log.error(f"Failed get env nodes.{self.get_log_common()}.")
            return []
        primary_id_port = self.get_primary_info(nodes)
        if not primary_id_port:
            return []
        # 2. 遍历节点信息，其中一个主节点拆分备份与副本信息上报子任务，其他主节点均拆分挂载子任务
        sub_job_array = []
        main_task_generator_status = True
        main_task_node_id = ''
        for node in nodes:
            host_id = node.get("id", "")
            host_ip = node.get("endpoint")
            inst_port = node.get("extendInfo", {}).get("port")
            inst_ip = node.get("extendInfo", {}).get("endpoint")
            job_info = f"{host_ip}:{inst_port}"
            ip_info = f"{inst_ip}:{inst_port}"
            if ip_info not in primary_id_port:
                continue
            if main_task_generator_status:
                job_type = "Backup"
                job_priority = SubJobPriority.JOB_PRIORITY_2
                main_task_node_id = host_id
                main_task_generator_status = False
                sub_job = self.build_sub_job(job_priority, host_id, job_info, job_type)
                sub_job_array.append(sub_job)
            else:
                job_priority = SubJobPriority.JOB_PRIORITY_1
                job_type = "Mount"
                sub_job = self.build_sub_job(job_priority, host_id, job_info, job_type)
                sub_job_array.append(sub_job)
        self._primary_node_num = len(primary_id_port)
        if not self.set_exec_node_info(main_task_node_id):
            log.error("Set exec node info fail.")
            return []

        log.info(f"Get sub_job_array succ.{self.get_log_common()}")
        return sub_job_array

    def pre_dmini_path(self, nodes_info):
        log.info(f"Getting dminiPath : {self._job_id}")
        for node in nodes_info:
            node_ip = node.get("endpoint", '')
            if not check_ip_in_local(node_ip):
                continue
            extend_info = node.get("extendInfo", '')
            dmini_path = extend_info.get("dminiPath", "")
            if not dmini_path:
                return ''
            return dmini_path
        return ''

    def get_primary_info(self, nodes):
        primary_node_name = get_primary_node_name(self._db_info)
        dmini_path = self.pre_dmini_path(nodes)
        all_nodes_info = DamengCluster().get_cluster_info(dmini_path)
        primary_id_port = []
        for node_info in all_nodes_info:
            if node_info.get("name") in primary_node_name:
                port = node_info.get('extendInfo', {}).get("port")
                ip_info = node_info.get('endpoint')
                ip_port = f"{ip_info}:{port}"
                primary_id_port.append(ip_port)
        return primary_id_port

    @exter_attack
    def backup_allow(self):
        """
        任务检查执行: 单机检查是否存在，集群还需要检查当前是否为主节点
        :return:
        """
        out_info = self.set_db_param()
        ret, err_code, err_param = self.check_backup_prerequisites()
        if not ret:
            out_info["bodyErr"] = err_code
            out_info["code"] = 1
            if err_param:
                out_info["bodyErrParams"] = [err_param]
        return out_info

    def check_backup_job_type(self):
        """
        检查备份任务类型
        :return:
        """
        output_info = {
            "code": 1,
            "bodyErr": None,
            "message": ""
        }
        log.info("Start check backup type.")
        last_copy_info = self.get_last_copy_info()
        backup_base_backupset = last_copy_info.get("extendInfo", {}).get("backupSetName", '')
        ret = self.check_db_magic()
        if backup_base_backupset and ret:
            output_info["code"] = CheckBackupJobTypeCode.CAN_EXEC
        else:
            output_info["bodyErr"] = CheckBackupJobTypeCode.INC_TO_FULL
        return output_info

    def get_db_magic_json(self):
        self._meta_area = self.get_area_info("meta_repository")
        empty_set = set()
        if not self._meta_area:
            log.error(f"Get meta area fail.{self.get_log_common()}")
            return empty_set
        file_name = "db_magic_data.json"
        if self._backup_type == BackupTypeEnum.LOG_BACKUP:
            file_name = "db_magic_log.json"
        path = os.path.join(self._meta_area, file_name)
        if not os.path.exists(path):
            log.error(f"Db_magic file not exits.{self.get_log_common()}")
            return empty_set
        try:
            with open(path, "r") as f_object:
                read_str = f_object.read()
        except IOError:
            log.error(f"Open db_magic file fail.{self.get_log_common()}")
            return empty_set
        try:
            db_magic_dict = json.loads(read_str)
        except Exception:
            log.error(f"Failed to parse the db magic file.{self.get_log_common()}")
            return empty_set
        db_magic_list = db_magic_dict.get("db_magic", [])
        if not db_magic_list and not isinstance(db_magic_list, list):
            log.error(f"Get db_magic fail.{self.get_log_common()}")
            return empty_set
        log.info(f"Get db_magic succ.{self.get_log_common()}")
        return set(db_magic_list)

    def check_db_magic(self):

        db_magic_set = self.get_db_magic_json()
        if not db_magic_set:
            log.error(f"Get db magic fail.{self.get_log_common()}")
            return False
        local_db_magic_set = self.get_local_db_magic()
        if not local_db_magic_set:
            log.error(f"Get local db magic fail.{self.get_log_common()}")
            return False
        if local_db_magic_set != db_magic_set:
            log.error(f"Check db magic fail.{self.get_log_common()}")
            return False
        log.info(f"Check db_magic succ.{self.get_log_common()}")
        return True

    def get_local_db_magic(self):
        db_magic_set = set()
        result_type = self.set_db_param().get("code")
        if result_type:
            return db_magic_set
        dm_tool = DmSqlTool(self._db_info)
        cmd = ("select count(DB_MAGIC) from v$rlog;", "select DB_MAGIC from v$rlog;")
        status, result = dm_tool.run_disql_tool(cmd)
        if not status or len(result) != len(cmd):
            return db_magic_set
        count_info = result[0].strip('\n').split('\n')
        # 3：输出结果至少为3行，sql结果占一行，一个空行，最后一个默认输出内容占一行
        if len(count_info) < 3:
            return db_magic_set
        count_info = count_info[-3].split(' ')
        count = del_space_in_list(count_info)[-1]
        db_magic_info = result[1].strip('\n').split('\n')
        # 2：输出结果最后两行默认为一个空行和默认输出内容占一行
        if len(db_magic_info) < 2 + int(count):
            return db_magic_set
        for line in db_magic_info[-(2 + int(count)):-2]:
            db_magic = del_space(line)[-1]
            db_magic_set.add(db_magic)
        return db_magic_set

    def save_db_magic(self):
        self._meta_area = self.get_area_info("meta_repository")
        if not self._meta_area:
            log.error(f"Save db_magic fail.{self.get_log_common()}")
            return False
        db_magic_set = self.get_local_db_magic()
        json_dict = {"db_magic": list(db_magic_set)}
        file_name = "db_magic_data.json"
        if self._backup_type == BackupTypeEnum.LOG_BACKUP:
            file_name = "db_magic_log.json"
        path = os.path.join(self._meta_area, file_name)
        output_execution_result_ex(path, json_dict)
        log.info(f"Save db_magic succ.{self.get_log_common()}")
        return True

    @exter_attack
    def prerequisite_progress(self):
        """
        前置任务进度
        :return:
        """
        log_detail = None
        task_status = SubJobStatusEnum.FAILED.value
        pre_status_dict = self.get_pre_status()
        if not pre_status_dict:
            output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                                   taskStatus=task_status,
                                   progress=Progress.END)
            return output.dict(by_alias=True)

        status = pre_status_dict.get("status", SubJobStatusEnum.FAILED.value)
        if status == SubJobStatusEnum.FAILED.value:
            err_code = pre_status_dict.get("err_code", '')
            err_code_param = pre_status_dict.get("err_code_param", '')
            if err_code:
                log_detail = [
                    LogDetail(logInfoParam=[self._job_id],
                              logLevel=DBLogLevel.ERROR,
                              logDetail=err_code,
                              logDetailParam=[err_code_param])
                ]
        else:
            task_status = status
        output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                               taskStatus=task_status,
                               logDetail=log_detail,
                               progress=Progress.END)
        return output.dict(by_alias=True)

    def get_pre_status(self):
        self._cache_area = self.get_area_info("cache_repository")
        if not self._cache_area:
            log.info(f"Get cache area fail.{self.get_log_common()}")
            return {}
        file_path = os.path.join(self._cache_area, f'backup_prerequisite_status_{self._job_id}')
        if not os.path.exists(file_path):
            return {}
        with open(file_path, "r") as f_object:
            json_dict = json.loads(f_object.read())
        return json_dict

    @exter_attack
    def backup_post(self):
        """
        执行后置任务
        :return:
        """
        log.info('Start exec post task.')
        if not self.set_area_info():
            self.save_backup_post_progress_file(SubJobStatusEnum.FAILED, Progress.END)
            return ActionResult(code=ActionResultCode.FAIL).dict(by_alias=True)
        self.save_backup_post_progress_file(SubJobStatusEnum.RUNNING, Progress.RUNNING)
        json_path = os.path.join(self._cache_area, f"backup_prerequisite{self._job_id}.json")
        backup_job_result = self._json_param.get("backupJobResult", 1)
        mount_path = f"{BACKUP_MOUNT_PATH}/{self._job_id}"
        new_path = os.path.join(BACKUP_MOUNT_PATH, f"{self._job_id}_bind")
        status = SubJobStatusEnum.COMPLETED
        ret, _ = self.check_exec_node()
        if ret:
            # 如果备份任务失败，清理备份集
            if backup_job_result != BackupJobResult.SUCCESS:
                if not self.clear_backupset(json_path):
                    log.info(f"Failed to delete the backup set.{self.get_log_common()}.")
                    status = SubJobStatusEnum.FAILED
            else:
                self.save_db_magic()
            # 不管清理备份集是否成功，都要做临时文件删除
            remove_status = self.backup_post_remove_path()
            if not remove_status:
                status = SubJobStatusEnum.FAILED
            self.save_backup_post_progress_file(status, Progress.END)

        if not umount(mount_path) or not umount(new_path):
            log.error(f"Umount backup mount path fail.{self.get_log_common()}.")
            status = SubJobStatusEnum.FAILED
        if not self.remove_file_dir(mount_path) or not self.remove_file_dir(new_path):
            status = SubJobStatusEnum.FAILED
        self.save_backup_post_progress_file(status, Progress.END)
        return ActionResult(code=ActionResultCode.SUCCESS).dict(by_alias=True)

    def set_area_info(self):
        job_dict = self._json_param.get("job", {})
        self._repositories_info = ParamFileUtil.parse_backup_path(job_dict.get("repositories"))
        self._cache_area = self._repositories_info.get("cache_repository", [""])[ArrayIndex.INDEX_FIRST_0]
        self._data_area = self._repositories_info.get("data_repository", [""])[ArrayIndex.INDEX_FIRST_0]
        self._log_area = self._repositories_info.get("log_repository", [""])[ArrayIndex.INDEX_FIRST_0]
        if not self._cache_area:
            log.error(f"Get cache area fail.{self.get_log_common()}")
            return False
        if self._backup_type != BackupTypeEnum.LOG_BACKUP and not self._data_area:
            log.error(f"Get data area fail.{self.get_log_common()}")
            return False
        if self._backup_type == BackupTypeEnum.LOG_BACKUP and not self._log_area:
            log.error(f"Get log area fail.{self.get_log_common()}")
            return False
        return True

    def clear_backupset(self, json_path):

        if not os.path.exists(json_path):
            log.info(f"The file does not exist.{self.get_log_common()}.")
            return True
        with open(json_path, "r") as f_object:
            try:
                backup_prerequisite_dir = json.loads(f_object.read())
            except Exception:
                log.error(f"Failed load json file.{self.get_log_common()}.")
                return False
        backupset_path = backup_prerequisite_dir.get("backupset_path", '')
        if not backupset_path:
            log.error(f"Failed to obtain the backupset path.{self.get_log_common()}.")
            return False
        group_id_list = backup_prerequisite_dir.get("groupID", [])
        if not group_id_list:
            if not self.remove_file_dir(backupset_path):
                return False
            return True
        return self.clear_cluster_backupset(group_id_list, backupset_path)

    def clear_cluster_backupset(self, group_id_list_, backupset_path_):
        for group_id in group_id_list_:
            backupset_name = backupset_path_.rsplit("/", 1)[-1]
            backupset_path = os.path.join(self._data_area, group_id, backupset_name)
            self.remove_file_dir(backupset_path)
        return True

    def remove_file_dir(self, path_):
        ret, realpath = check_path_in_white_list(path_)
        if not ret:
            log.error(f"Remove file path non-compliant.")
            return False
        if not os.path.exists(realpath):
            log.info(f"File not exists.{self.get_log_common()}.")
            return True
        if os.path.isfile(realpath):
            try:
                os.remove(realpath)
            except Exception:
                log.error(f"Failed to delete the file.{self.get_log_common()}.")
                return False
        else:
            try:
                shutil.rmtree(realpath)
            except Exception:
                log.error(f"Folder to delete the file.{self.get_log_common()}.")
                return False
        log.info(f"File deleted successfully.{self.get_log_common()}.")
        return True

    def save_backup_post_progress_file(self, status_, progress_):
        progress_info = {"status": status_, "progress_": progress_}
        file_path = os.path.join(self._cache_area, f"{self._sub_job_id}.json")
        output_execution_result_ex(file_path, progress_info)

    def read_post_progress_file(self):
        file_path = os.path.join(self._cache_area, f"{self._sub_job_id}.json")
        if not os.path.exists(file_path):
            log.error(f"File not exists.")
            return {}
        with open(file_path, 'r') as jsonfile:
            try:
                json_dict = json.loads(jsonfile.read())
            except FileNotFoundError:
                log.error(f"Read file fail.")
                return {}
        return json_dict

    def check_exec_node(self):
        job_dict = self._json_param.get("job", {})
        object_info = job_dict.get("protectObject", {})
        sub_type = object_info.get("subType", '')
        if sub_type == BackupSubType.CLUSTER:
            # 判断是否是执行备份命令的节点
            exec_node_info = self.get_exec_node_info_json()
            exec_node_uuid = exec_node_info.get("uuid", "")
            primary_node_num = exec_node_info.get("primary_node_num", 1)
            local_uuid = get_hostsn()
            if exec_node_uuid != local_uuid:
                return False, 0
            return True, primary_node_num
        elif sub_type == BackupSubType.SINGLE_NODE:
            return True, 1
        else:
            return False, 0

    def backup_post_remove_path(self):
        data_size_path = os.path.join(self._cache_area, f"data_size{self._job_id}.json")
        prerequisite_path = os.path.join(self._cache_area, f"backup_prerequisite{self._job_id}.json")
        exec_job_node_file_path = os.path.join(self._cache_area, f'exec_node{self._job_id}.json')
        backup_succ_path = os.path.join(self._cache_area, f"backup_status_{self._job_id}.json")
        sql_errcode_path = os.path.join(self._cache_area, f"sql_errcode_{self._job_id}.json")
        times_path = os.path.join(self._cache_area, f"get_errcode_times{self._job_id}.json")
        remove_path_list = [
            data_size_path, prerequisite_path,
            exec_job_node_file_path, sql_errcode_path,
            times_path, backup_succ_path
        ]
        remove_status = True
        for remove_path in remove_path_list:
            if not self.remove_file_dir(remove_path):
                remove_status = False
        return remove_status

    @exter_attack
    def post_progress(self):
        """
        后置任务进度
        :return:
        """
        log.info('Start quert post progress.')
        job_dict = self._json_param.get("job", {})
        self._repositories_info = ParamFileUtil.parse_backup_path(job_dict.get("repositories"))
        self._cache_area = self._repositories_info.get("cache_repository")[ArrayIndex.INDEX_FIRST_0]
        file_path = os.path.join(self._cache_area, f"{self._job_id}.json")
        ret = self.read_post_progress_file()
        status = ret.get("status", SubJobStatusEnum.FAILED)
        progress = ret.get("progress", Progress.END)
        if status != SubJobStatusEnum.RUNNING:
            self.remove_file_dir(file_path)
        output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                               taskStatus=status,
                               progress=progress)
        return output.dict(by_alias=True)

    def get_single_lsn(self):
        sql_cmd = "select CUR_LSN from v$rlog;"
        tool = DmSqlTool(self._db_info)
        status, lsn_info = tool.run_disql_tool((sql_cmd,))
        if not status:
            return ''
        lsn_info = lsn_info[ArrayIndex.INDEX_FIRST_0]
        lsn_info = lsn_info.strip("\n").split("\n")
        if len(lsn_info) < ArrayIndex.INDEX_FIRST_3:
            return ''
        lsn_info = lsn_info[ArrayIndex.INDEX_LAST_3]
        lsn = del_space(lsn_info)[ArrayIndex.INDEX_LAST_1]
        return lsn

    def get_cluster_lsn(self):

        cmd = "select CUR_LSN from v$rlog;"
        cmd_list = [cmd]
        tool = DmSqlTool(self._db_info)
        status, lsn_info = tool.run_disql_tool(cmd_list, mpp_type='local')
        if not status:
            return ''
        lsn_info = lsn_info[ArrayIndex.INDEX_FIRST_0]
        lsn_info = lsn_info.strip("\n").split("\n")
        if len(lsn_info) < ArrayIndex.INDEX_FIRST_3:
            return ''
        lsn_info = lsn_info[ArrayIndex.INDEX_LAST_3]
        lsn = del_space(lsn_info)[ArrayIndex.INDEX_LAST_1]
        return lsn

    def get_backup_history(self):
        """
        通过路径关键字查数据库获取备份记录
        :return:
        """
        job_dict_ = self._json_param.get("job", {})
        err_count = -1
        self._repositories_info = ParamFileUtil.parse_backup_path(job_dict_.get("repositories"))
        self._cache_area = self._repositories_info.get("cache_repository", [""])[ArrayIndex.INDEX_FIRST_0]
        self._data_area = self._repositories_info.get("data_repository", [""])[ArrayIndex.INDEX_FIRST_0]
        if not self._cache_area:
            log.error(f"Get cache area fail.{self.get_log_common()}")
            return err_count
        if self._backup_type != BackupTypeEnum.LOG_BACKUP and not self._data_area:
            log.error(f"Get data area fail.{self.get_log_common()}")
            return err_count
        backup_prerequisite_json = self.get_backup_prerequisite_json()
        if not backup_prerequisite_json:
            return err_count
        self.backupset_path = backup_prerequisite_json.get("backupset_path", '')
        if check_command_injection(self.backupset_path):
            return err_count
        dm_tool = DmSqlTool(self._db_info)
        if self.version != "V8":
            return self.get_backup_history_v7()
        backup_history_sql_with_status = (
            f"select COUNT(*) from V$BACKUP_HISTORY where PATH=\'{self.backupset_path}\'and STATUS=\'SUCCESS\';",
        )
        status, query_result = dm_tool.run_disql_tool(disql_cmd=backup_history_sql_with_status)
        if not status:
            log.error(f"Failed exec history sql with status.{self.get_log_common()}.")
            backup_history_sql = (
                f"select COUNT(*) from V$BACKUP_HISTORY where PATH=\'{self.backupset_path}\';",
            )
            status, query_result = dm_tool.run_disql_tool(disql_cmd=backup_history_sql)
            if not status:
                log.error(f"Failed exec sql.{self.get_log_common()}.")
                return err_count
        result_info = query_result[ArrayIndex.INDEX_FIRST_0].strip('\n').split('\n')
        last_row = -1
        if len(result_info) < ArrayIndex.INDEX_FIRST_3:
            log.error(f"Sql result info err.{self.get_log_common()}.")
            return err_count
        result_info = result_info[ArrayIndex.INDEX_LAST_3].split(' ')
        result_info = del_space_in_list(result_info)
        history_cout = result_info[ArrayIndex.INDEX_LAST_1]
        history_cout = del_space(history_cout)[last_row]
        try:
            history_cout = int(history_cout)
        except ValueError:
            log.error(f"History cout err.{self.get_log_common()}.")
            return err_count
        return history_cout

    def get_backup_history_v7(self):
        job_dict = self._json_param.get("job", {})
        object_info = job_dict.get("protectObject", {})
        sub_type = object_info.get("subType", '')
        if sub_type == BackupSubType.SINGLE_NODE:
            return self.get_backup_history_v7_single(self.backupset_path)
        new_path = os.path.join(BACKUP_MOUNT_PATH, f"{self._job_id}_bind")
        ret, new_path = backup_mount_bind(self._data_area, self._job_id, new_path)
        err_count = -1
        if not ret:
            log.error(f"Backup mount bind fail.{self.get_log_common()}")
            return err_count
        oguid_list = self.get_all_oguid()
        if not oguid_list:
            log.error(f"Get oguid list fail.{oguid_list}.{self.get_log_common()}")
            return err_count
        result_count = 0
        backupset_name_info = self.backupset_path.rsplit('/', 1)
        # 2:只切割一次所以预期返回结果列表长度为2
        if len(backupset_name_info) != 2:
            log.error(f"Get backupset name fail.{self.get_log_common()}")
            return err_count
        backupset_name = backupset_name_info[-1]
        for oguid in oguid_list:
            backupset_path = os.path.join(new_path, oguid, backupset_name)
            result = self.get_backup_history_v7_single(backupset_path)
            single_backup_history = 1
            if result != single_backup_history:
                continue
            result_count += 1
        return result_count

    def get_backup_history_v7_single(self, f_backupset_path):
        backupset_path = f_backupset_path.rsplit('/', 1)[0]
        dm_tool = DmSqlTool(self._db_info)
        backup_history_sql = (
            f"select sf_bakset_backup_dir_add('disk','{backupset_path}');",
            f"select COUNT(*) from V$BACKUPSET where BACKUP_PATH='{f_backupset_path}';",
            f"select sf_bakset_backup_dir_remove('disk','{backupset_path}');"
        )
        err_count = -1
        status, query_result = dm_tool.run_disql_tool(disql_cmd=backup_history_sql)
        if not status:
            log.error(f"Failed exec sql.{self.get_log_common()}.")
            return err_count
        result_info = query_result[ArrayIndex.INDEX_FIRST_1].strip('\n').split('\n')
        last_row = -1
        if len(result_info) < ArrayIndex.INDEX_FIRST_3:
            return err_count
        result_info = result_info[ArrayIndex.INDEX_LAST_3].split(' ')
        result_info = del_space_in_list(result_info)
        history_count = result_info[ArrayIndex.INDEX_LAST_1]
        history_count = del_space(history_count)[last_row]
        try:
            history_count = int(history_count)
        except ValueError:
            return err_count
        return history_count

    def get_all_oguid(self):
        cmd = "select OGUID from v$instance;"
        tool = DmSqlTool(self._db_info)
        status, result_msg = tool.run_disql_tool((cmd,))
        if not status:
            log.error(f"Failed query oguid.{self.get_log_common()}.")
            return []
        log.info(f"Query oguid succ.")

        result_msg = result_msg[ArrayIndex.INDEX_FIRST_0].strip("\n").split("\n")
        result_msg = del_space_in_list(result_msg)
        index = 1
        all_oguid_info = []
        for line in result_msg:
            index += 1
            if "LINEID" in line or "行号" in line:
                all_oguid_info = result_msg[index:ArrayIndex.INDEX_LAST_1]
                break
        result_oguid_list = []

        for oguid_info in all_oguid_info:
            oguid = del_space(oguid_info)[ArrayIndex.INDEX_LAST_1]
            result_oguid_list.append(oguid)
        return result_oguid_list

    def fill_data_copy(self, backupset_info_):
        """
        查询数据备份集信息
        :return:
        """
        out_put_info = {
            "extendInfo": {
                "backupTime": "",
                "beginTime": None,
                "endTime": None,
                "beginSCN": None,
                "endSCN": None,
                "backupset_dir": '',
                "backupSetName": "",
                "backupType": "",
                "baseBackupSetName": "",
                "dbName": "",
                "groupId": '',
                "tabal_space_info": []
            }
        }
        ret, backupset_msg = self.parses_data_backupset_info(backupset_info_)
        if not ret:
            log.error(f'Parses data backupset info fail.{self.get_log_common()}.')
            return out_put_info
        if not self.file_copy_info_from_pre_info(out_put_info):
            return out_put_info
        extend_info = out_put_info.get("extendInfo", {})
        extend_info.update(backupset_msg)
        return out_put_info

    def file_copy_info_from_pre_info(self, out_put_info):
        json_dict = self.get_backup_prerequisite_json()
        if not json_dict:
            log.error(f"Get json_dict fail.{self.get_log_common()}.")
            return False
        extend_info = out_put_info.get("extendInfo", {})
        extend_info["backupSetName"] = json_dict.get("backupSetName")
        extend_info["backupType"] = self._backup_type
        extend_info["baseBackupSetName"] = json_dict.get("baseBackupSetName")
        extend_info["groupId"] = json_dict.get("groupID", [])
        data_path = []
        for group_id in extend_info["groupId"]:
            data_path.append(f'/{group_id}/{extend_info["backupSetName"]}')
        extend_info["dataPath"] = data_path
        if not data_path:
            extend_info["dataPath"] = extend_info["backupSetName"]
        return True

    def fill_log_copy(self, backupset_info_):
        """
        查询日志备份信息
        :return:
        """
        log.info("Run fill log copy.")
        out_put_info = {
            "extendInfo": {
                "backupTime": "",
                "beginTime": None,
                "endTime": None,
                "beginSCN": None,
                "endSCN": None,
                "backupSetName": "",
                "backupType": "",
                "baseBackupSetName": "",
                "dbName": "",
                "groupId": "",
                "backupset_dir": ''
            }
        }
        metadata_info = backupset_info_.get("backupsets", {}).get("group", {}).get("backupset", {}).get("metadata", {})
        file_info = backupset_info_.get("backupsets", {}).get("group", {}).get("backupset", {}).get("file_info", {})
        db_name = metadata_info.get("db_name")
        if not self.get_arch_time(file_info):
            return out_put_info
        if not self.file_copy_info_from_pre_info(out_put_info):
            return out_put_info
        extend_info = out_put_info.get("extendInfo", {})
        extend_info["dbName"] = db_name
        extend_info["backupTime"] = self.end_time
        extend_info["beginTime"] = self.begin_time
        extend_info["endTime"] = self.end_time
        extend_info["backupset_dir"] = backupset_info_.get("backupset_dir")
        return out_put_info

    def get_arch_time(self, file_info):
        arch_file = file_info.get("arch_file", [])
        if not arch_file:
            return False
        arch_file_info = []
        if isinstance(arch_file, dict):
            arch_file_info.append(arch_file)
        else:
            arch_file_info = arch_file
        begin_time = arch_file_info[0].get("create_time")
        # 特殊处理时间全0场景
        if str(begin_time).strip() == '0000-00-00 00:00:00':
            self.begin_time = 1
        else:
            self.begin_time = timestamp(begin_time)
        end_time = arch_file_info[-1].get("close_time")
        self.end_time = timestamp(end_time)
        return True

    @exter_attack
    def query_backup_copy(self):
        """
        查询备份信息
        :return:
        """
        log.info(f"job id: {self._job_id}, Start query copy info.")
        # 1. 查询前置任务保存的临时信息
        job_dict = self._json_param.get("job", {})
        self._repositories_info = ParamFileUtil.parse_backup_path(job_dict.get("repositories"))
        self._cache_area = self._repositories_info.get("cache_repository", [])[ArrayIndex.INDEX_FIRST_0]
        save_path = os.path.join(self._cache_area, f"backup_prerequisite{self._job_id}.json")
        if not os.path.exists(save_path):
            return False
        with open(save_path, 'r') as f:
            try:
                self.json_dict = json.loads(f.read())
            except Exception:
                return False
        if not self.json_dict:
            log.error(f"Failed get backup_prerequisite{self._job_id}.json.{self.get_log_common()}.")
            return False
        # 2. 获取副本信息
        backupset_dir = self.json_dict.get("backupset_path")
        backupset_info = show_backupset(backupset_dir)
        if not backupset_info:
            log.error(f"Failed to obtain the backup set information.{self.get_log_common()}.")
            return False
        metadata_info = backupset_info.get("backupsets", {}).get("group", {}).get("backupset", {}).get("metadata", {})
        backup_type = metadata_info.get("type", '')
        range_new = metadata_info.get("range", '')
        if not backup_type:
            log.error(f"Failed to obtain the backup type.{self.get_log_common()}.")
            return False
        # 3. 组装表空间信息
        if backup_type == DamengStrConstant.BACKUP_TYPE_ARCHIVE or range_new == DamengStrConstant.BACKUP_TYPE_ARCHIVE:
            json_copy = self.fill_log_copy(backupset_info)
            start_time = json_copy.get("extendInfo", {}).get("beginTime", 0)
            if not self.check_log_time(start_time):
                log.error(f'job id: {self._job_id}, failed to check log time')
                return False
        else:
            json_copy = self.fill_data_copy(backupset_info)
        ret, repositories = self.set_repositories(backupset_dir)
        if not ret:
            log.info(f"Set repositories fail.")
            return False
        json_copy["repositories"] = repositories
        copy_info = {"copy": json_copy, "jobId": self._job_id}
        try:
            invoke_rpc_tool_interface(self._job_id, RpcParamKey.REPORT_COPY_INFO, copy_info)
        except Exception as err_info:
            log.error(f"Report copy info fail.{self.get_log_common()}")
            return False
        log.info(f"Report copy info succ.{self.get_log_common()}")
        return True

    def check_log_time(self, start_time):
        if not start_time:
            log.error(f"The log start time is incorrect.{self.get_log_common()}")
            return False
        get_all_type_copy = 1
        last_copy_info = self.get_last_copy_info(get_all_type_copy)
        copy_extend_info = last_copy_info.get("extendInfo", {})
        if copy_extend_info.get("backupType") == self._backup_type:
            end_time = copy_extend_info.get("endTime")
        else:
            end_time = copy_extend_info.get("backupTime")
        if not end_time:
            log.error(f"Failed to obtain the backup time of the data copy.{self.get_log_common()}")
            return False
        if end_time < start_time:
            log.error(f"The log time is not continuous with the data copy time.{self.get_log_common()}")
            return False
        return True

    def query_backup_process(self):
        """
        查询备份进程信息
        :return:
        """

        # 1. 查询进程信息
        pid_rules = "disql"
        ppid_rules = [self._job_id, BackupStepEnum.BACKUP]
        ret, output, errormsg = query_process_info(pid_rules=pid_rules, ppid_rules=ppid_rules, isppid=True)

        # 2. 校验查询结果
        if not ret:
            log.error(f"Query disql process info fail: {ret}.{self.get_log_common()}.")
            return False, 0
        if len(output) != ArrayIndex.INDEX_FIRST_1:
            log.error(f"The number of processes is incorrect.{self.get_log_common()}.")
            return False, 0

        process_pid = int(output[ArrayIndex.INDEX_FIRST_0][ArrayIndex.INDEX_FIRST_0])
        return True, process_pid

    def kill_backup_process(self):
        """
        终止备份进程
        :return:
        """

        ret, process_pid = self.query_backup_process()
        if not ret:
            return False
        os.kill(process_pid, signal.SIGKILL)

        # 校验进程是否被终止
        ret, process_pid = self.query_backup_process()
        if ret:
            log.error(f"Failed kill backup process, ret: {ret}.{self.get_log_common()}.")
            return False
        return True

    @exter_attack
    def backup_abort(self):
        """
        停止备份任务
        :return:
        """
        ret = self.kill_backup_process()
        if ret:
            output = ActionResult(code=0).dict(by_alias=True)
        else:
            output = ActionResult(code=1).dict(by_alias=True)
        return output

    def backup_save_tablespace(self, backupdir_):
        """
        保存备份集的表空间信息
        :return:
        """
        log.info("Start save tablespace.")
        # 1. create dir
        log.info("Step 3: create a directory for storing tablespace info.")
        meta_path = self._meta_area
        if not meta_path:
            log.error(f'Get meta_path fail.{self.get_log_common()}.')
            return False
        sqlite_path = os.path.join(meta_path, self._copy_id, "sqlite")
        check_ret, real_path = check_path_in_white_list(sqlite_path)
        if not check_ret:
            log.error(f'Invalid sqlite path.{self.get_log_common()}.')
            return False

        if os.path.exists(sqlite_path):
            shutil.rmtree(sqlite_path)
        os.makedirs(sqlite_path)
        # 2. create table
        log.info("Step 3: creat table.")
        sqlite_file = f"{sqlite_path}/copymetadata.sqlite"
        conn = sqlite3.connect(sqlite_file)
        self.cur = conn.cursor()
        sql_create_table = 'create table IF NOT EXISTS T_COPY_METADATA(UUID TEXT, NAME TEXT, ' \
                           'TYPE TEXT DEFAULT "d", PARENT_PATH TEXT DEFAULT "/", ' \
                           'PARENT_UUID TEXT, SIZE INT64(8), CREATE_TIME TEXT, MODIFY_TIME TEXT, ' \
                           'EXTEND_INFO TEXT, RES_TYPE TEXT, RES_SUB_TYPE TEXT)'
        self.cur.execute(sql_create_table)

        # 3. insert data
        log.info("Step 3: insert tabespace.")
        ret = self.insert_data_to_sqlite(backupdir_)
        conn.commit()
        conn.close()
        if not ret:
            log.info(f"Save tabespace fail.{self.get_log_common()}.")
            return False
        log.info("Save tablespace succ.")
        return True

    def save_backupset_path(self, backupset_path):
        save_path = os.path.join(self._cache_area, f"backup_prerequisite{self._job_id}.json")
        self._backup_prerequisite_dic["backupset_path"] = backupset_path
        check_ret, real_path = check_path_in_white_list(save_path)
        if not check_ret:
            log.error(f'Invalid path.{self.get_log_common()}.')
            return False
        output_execution_result_ex(save_path, self._backup_prerequisite_dic)
        log.info('Save_backupset_path succ.')
        return True

    def insert_data_to_sqlite(self, backupdir_):
        backupset_info = show_backupset(backupdir_)
        table_info_set = set()
        table_info_list = []
        backupset_info = backupset_info.get("backupsets",
                                            {}).get("group",
                                                    {}).get("backupset", {})
        backup_time = backupset_info.get("metadata", {}).get("backup_time", {})
        if not backup_time:
            log.error(f"Get backup time fail.{self.get_log_common()}.")
            return False
        sub_backupsets = backupset_info.get("sub_backupset", [])
        if not sub_backupsets:
            sub_backupsets = []
            backupset_dict = dict()
            backupset_dict["file_info"] = backupset_info.get("file_info", {})
            sub_backupsets.append(backupset_dict)
        for sub_backupset in sub_backupsets:
            data_info = sub_backupset.get("file_info", {}).get("data_file", [])
            # 这里的校验是区分在备份时对应的数据文件是否做了拆分
            data_info_list = []
            if isinstance(data_info, dict):
                data_info_list.append(data_info)
            else:
                data_info_list = data_info
            for table_space in data_info_list:
                table_name = table_space.get("group_name")
                table_size = table_space.get("file_len")
                table_path = table_space.get("file_name")
                table_info_set.add((table_name, table_size, table_path))
                table_info_list = list(table_info_set)
        table_modify_time_dict = self.get_tablespace_modify_time()
        for table_info in table_info_list:
            table_name, table_size, table_path = table_info
            table_modify_time = table_modify_time_dict.get(table_path, '')
            if not table_modify_time:
                table_modify_time = backup_time
            sql_insert_data = f'insert into T_COPY_METADATA(NAME, SIZE, MODIFY_TIME) \
                values(\"{table_name}\", {table_size}, \"{table_modify_time}\")'
            if not self.cur:
                return False
            self.cur.execute(sql_insert_data)
        return True

    def get_tablespace_modify_time(self):
        cmd = (
            "SP_SET_SESSION_MPP_SELECT_LOCAL(1);",
            "select count(MODIFY_TIME) from v$datafile;",
            "select MODIFY_TIME, PATH from v$datafile;",
            "SP_SET_SESSION_MPP_SELECT_LOCAL(0);"
        )
        modify_time_info_dict = {}
        dmtool = DmSqlTool(self._db_info)
        status, result = dmtool.run_disql_tool(cmd)
        if not status or len(result) != len(cmd):
            log.error(f"Select MODIFY_TIME sql execution  fail.{self.get_log_common()}.")
            return {}
        count_info = result[1].strip('\n').split('\n')
        # 3：输出结果至少为3行，sql结果占一行，一个空行，最后一个默认输出内容占一行
        if len(count_info) < 3:
            log.error(f"The SQL command fails to be executed.{self.get_log_common()} ")
            return {}
        count_info = count_info[-3].split(' ')
        count = del_space_in_list(count_info)[-1]
        tablespace_modify_time_info = result[2].strip('\n').split('\n')
        # 2：输出结果最后两行默认为一个空行和默认输出内容占一行
        if len(tablespace_modify_time_info) < 2 + int(count):
            log.error(f"The SQL command fails to be executed.{self.get_log_common()} ")
            return {}
        for line in tablespace_modify_time_info[-(2 + int(count)):-2]:
            tablespace_info_list = del_space(line)
            if len(tablespace_info_list) < 3:
                continue
            tablespace_path = tablespace_info_list[-1]
            modify_time_ymd = tablespace_info_list[ArrayIndex.INDEX_LAST_3]
            modify_time_ymd = modify_time_ymd.replace('-', '/')
            modify_time_hms = tablespace_info_list[ArrayIndex.INDEX_LAST_2]
            modify_time = f"{modify_time_ymd} {modify_time_hms}"
            modify_time_info_dict[tablespace_path] = modify_time
        return modify_time_info_dict

    def save_backupset_info(self):
        if self._db_type == BackupSubType.CLUSTER:
            all_oguid_list = self.get_all_oguid()
            self._backup_prerequisite_dic["groupID"] = all_oguid_list
        return True

    def get_backup_prerequisite_json(self):
        path_info = os.path.join(self._cache_area, f"backup_prerequisite{self._job_id}.json")
        if os.path.exists(path_info):
            with open(path_info, 'r') as jsonfile:
                try:
                    json_dict = json.loads(jsonfile.read())
                except FileNotFoundError:
                    log.info("Read backup_prerequisite json fail.")
                    return {}
            return json_dict
        log.error(f"File backup_prerequisite{self._job_id}.json not exits.{self.get_log_common()}.")
        return {}

    def backup_prepare_full(self):
        """
        准备全量备份的日志的sql语句、备份目录
        :return:ret, sql, dir
        """
        log.info(f"job id: {self._job_id}, backup prepare full, db type: {self._db_type}.")
        if self._db_type == BackupSubType.SINGLE_NODE:
            lsn = self.get_single_lsn()
        elif self._db_type == BackupSubType.CLUSTER:
            lsn = self.get_cluster_lsn()
        else:
            log.error(f"Unknow database type: {self._db_type}.{self.get_log_common()}.")
            return False, '', ''
        if not lsn:
            log.error(f"Get lsn fail.{self.get_log_common()}.")
            return False, '', ''
        if not check_path_in_white_list(self._data_area):
            log.error(f"Invalid data area path..{self.get_log_common()}.")
            return False, '', ''
        backup_dir = os.path.join(self._data_area, f"DM_FULL_{lsn}")
        backup_sql = f"BACKUP DATABASE BACKUPSET \'{backup_dir}\' PARALLEL {self.channel_number};"
        self._backup_prerequisite_dic["backupSetName"] = f"DM_FULL_{lsn}"
        self._backup_prerequisite_dic["backupType"] = "FULL"
        self._backup_prerequisite_dic["baseBackupSetName"] = ''
        log.info(f"job id: {self._job_id}, backup prepare full success.")
        return True, backup_sql, backup_dir

    def backup_prepare_increment_or_diff(self):
        """
        准备增量备份的日志的sql语句、备份目录
        :return:ret, sql, dir
        """
        log.info(f"job id: {self._job_id}, backup prepare increment or diff.")
        backup_dir = self._data_area
        last_copy_info = self.get_last_copy_info()
        backup_base_backupset = last_copy_info.get("extendInfo", {}).get("backupSetName")
        if not backup_base_backupset:
            log.error(f"The base backup set does not exist.{self.get_log_common()}.")
            return False, "", ""
        if self._db_type == BackupSubType.SINGLE_NODE:
            lsn = self.get_single_lsn()
        elif self._db_type == BackupSubType.CLUSTER:
            lsn = self.get_cluster_lsn()
        else:
            log.error(f"Unknow database type: {self._db_type}.{self.get_log_common()}.")
            return False, '', ''
        backup_type = "DIFF"
        backup_sql = DamengStrFormat.CUMULATIVE_BUCKUP_SQL
        if self._backup_type == BackupTypeEnum.INCRE_BACKUP:
            backup_type = "INCRE"
            backup_sql = DamengStrFormat.INCREMENT_BUCKUP_SQL
        backup_dir = os.path.join(backup_dir, f"DM_{backup_type}_{lsn}")
        if not check_path_in_white_list(self._data_area):
            log.error(f"Invalid data area path..{self.get_log_common()}.")
            return False, '', ''
        base_backup_dir = os.path.join(self._data_area, backup_base_backupset)
        if not os.path.exists(base_backup_dir):
            log.error(f"The base backupset dir does not exist.{self.get_log_common()}.")
            return False, '', ''
        backup_sql = backup_sql.format(base_backup_dir, backup_dir, self.channel_number)
        self._backup_prerequisite_dic["backupSetName"] = f"DM_{backup_type}_{lsn}"
        self._backup_prerequisite_dic["backupType"] = backup_type
        self._backup_prerequisite_dic["baseBackupSetName"] = backup_base_backupset
        log.info(f"job id: {self._job_id}, backup prepare increment or diff success.")
        return True, backup_sql, backup_dir

    def backup_prepare_log(self):
        """
        准备日志备份的sql语句、备份目录
        :return:
        """
        log.info(f"job id: {self._job_id}, backup prepare log.")
        last_copy_info = self.get_last_copy_info()
        arch_base_lsn = 0
        ret = self.check_db_magic()
        backup_base_backupset = last_copy_info.get("extendInfo", {}).get("backupSetName")
        if backup_base_backupset and ret:
            backup_base_backupset_info = backup_base_backupset.split("_")
            if len(backup_base_backupset_info) != 3:
                return False, "", ""
            arch_base_lsn = backup_base_backupset_info[2]
        if self._db_type == BackupSubType.SINGLE_NODE:
            lsn = self.get_single_lsn()
        elif self._db_type == BackupSubType.CLUSTER:
            lsn = self.get_cluster_lsn()
        else:
            log.error(f"Unknow database type: {self._db_type}.{self.get_log_common()}.")
            return False, '', ''
        if not lsn:
            log.error(f"Get lsn fail.{self.get_log_common()}.")
            return False, '', ''
        if not check_path_in_white_list(self._log_area):
            log.error(f"Invalid log area path..{self.get_log_common()}.")
            return False, '', ''
        backup_dir = os.path.join(self._log_area, f"DM_LOG_{lsn}")
        backup_sql = f"BACKUP ARCHIVELOG LSN BETWEEN {arch_base_lsn} AND {MAX_LSN} BACKUPSET '{backup_dir}';"
        self._backup_prerequisite_dic["backupSetName"] = f"DM_LOG_{lsn}"
        self._backup_prerequisite_dic["backupType"] = "LOG"
        self._backup_prerequisite_dic["baseBackupSetName"] = ''
        log.info(f"job id: {self._job_id}, backup prepare log success.")
        return True, backup_sql, backup_dir

    def backup_prepare(self):
        """
        根据备份类型，准备备份的sql、日志目录
        :return:ret, sql, dir
        """
        # 准备备份路径
        log.info(f"job id: {self._job_id}, backup prepare.")
        if not self.prepare_backup_path():
            log.error(f'job id: {self._job_id}, prepare backup path failed')
            return False, '', ''
        # 校验是否执行备份命令
        job_name = self._json_param.get("subJob", {}).get("jobName", "")
        if job_name == "Mount":
            log.info("Mount succ.")
            return "Mount", '', ''
        # 获取备份并发数,并校验参数是否合法
        self.channel_number = self.get_channel_number()
        try:
            check_test = int(self.channel_number)
        except Exception as e_info:
            log.error(f"Channel number invalid parameter.")
            return False, '', ''
        # 2. 拼接备份sql、准备备份目录
        if not self.set_version():
            log.error(f"Set big version fail.{self.get_log_common()}")
            return '', '', ''
        if self._backup_type == BackupTypeEnum.FULL_BACKUP:
            prepare_ret, backup_sql, backup_dir = self.backup_prepare_full()
        elif self._backup_type == BackupTypeEnum.LOG_BACKUP:
            prepare_ret, backup_sql, backup_dir = self.backup_prepare_log()
        elif self._backup_type == BackupTypeEnum.INCRE_BACKUP or \
                self._backup_type == BackupTypeEnum.DIFF_BACKUP:
            prepare_ret, backup_sql, backup_dir = self.backup_prepare_increment_or_diff()
        else:
            log.error("Unknown backup type.")
            return False, "", ""
        # 3. 备份前信息保存
        if not self.save_backupset_info():
            log.error("Fail save backupset history.")
            return False, "", ""
        log.info(f"job id: {self._job_id}, backup prepare success.")
        return prepare_ret, backup_sql, backup_dir

    def prepare_backup_path(self):
        job_dict = self._json_param.get("job", {})
        self._repositories_info = ParamFileUtil.parse_backup_path(job_dict.get("repositories", ""))
        self._cache_area = self._repositories_info.get("cache_repository", [])[ArrayIndex.INDEX_FIRST_0]
        if self._backup_type == BackupTypeEnum.LOG_BACKUP:
            self._meta_area = self._repositories_info.get("meta_repository", [])[ArrayIndex.INDEX_FIRST_0]
            result_msg = self.prepare_log_back_path()
        else:
            result_msg = self.prepare_data_back_path()
            self._meta_area = self._repositories_info.get("meta_repository", [])[ArrayIndex.INDEX_FIRST_0]
        if not result_msg:
            log.error("Failed prepare backup failed.")
            return False
        return True

    def prepare_log_back_path(self):
        self._log_area = self._repositories_info.get("log_repository", [])[ArrayIndex.INDEX_FIRST_0]
        if self._db_type == BackupSubType.SINGLE_NODE:
            result_type, self._log_area = backup_mount_bind(self._log_area, self._job_id)
        else:
            oguid = get_oguid(self._db_info)
            if not oguid:
                return False
            backdir = f"{self._log_area}/{oguid}"
            if not os.path.exists(backdir):
                if not dameng_user_mkdir(backdir):
                    return False
            # 集群环境，两主机dmdba用户id不同时，主备切换后需要others的读写权限
            os.chmod(backdir, 0o777)
            result_type, self._log_area = backup_mount_bind(backdir, self._job_id)
        return result_type

    def prepare_data_back_path(self):
        self._data_area = self._repositories_info.get("data_repository", [])[ArrayIndex.INDEX_FIRST_0]
        if self._db_type == BackupSubType.SINGLE_NODE:
            result_type, self._data_area = backup_mount_bind(self._data_area, self._job_id)
        else:
            oguid = get_oguid(self._db_info)
            if not oguid:
                return False
            backdir = f"{self._data_area}/{oguid}"
            # 获取数据库安装用户信息
            if not os.path.exists(backdir) and not dameng_user_mkdir(backdir):
                return False
            # 集群环境，两主机dmdba用户id不同时，主备切换后需要others的读写权限
            os.chmod(backdir, 0o777)
            result_type, self._data_area = backup_mount_bind(backdir, self._job_id)
        return result_type

    @exter_attack
    def backup_progress(self):
        """
        查询备份进度
        :return:
        """
        log.info("Start to exec progress task.")
        job_dict = self._json_param.get("job", {})
        object_info = job_dict.get("protectObject", {})
        self._db_type = object_info.get("subType", '')
        if not self.set_version():
            return self.return_err_info()
        backup_history_add_num = 1
        if self._db_type == BackupSubType.CLUSTER:
            # 判断是否是执行备份命令的节点
            exec_node_info = self.get_exec_node_info_json()
            exec_node_uuid = exec_node_info.get("uuid", "")
            backup_history_add_num = exec_node_info.get("primary_node_num", "")
            local_uuid = get_hostsn()
            if exec_node_uuid != local_uuid:
                status = SubJobStatusEnum.COMPLETED.value
                output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, taskStatus=status,
                                       progress=Progress.END)
                return output.dict(by_alias=True)
        result_type = self.set_db_param().get("code")
        if result_type:
            log.error(f"Set db patam fail.{self.get_log_common()}.")
            return self.return_err_info()
        # 获取backup_history_num
        output = self.get_backup_progress(backup_history_add_num)
        hostsn = get_hostsn()
        pre_path_info = os.path.join(self._cache_area, f"backup_prerequisite{self._job_id}.json")
        if output.task_status == SubJobStatusEnum.FAILED and not os.path.exists(pre_path_info):
            return output.dict(by_alias=True)
        if output.task_status == SubJobStatusEnum.FAILED:
            return self.backup_progress_fail(output)
        if output.task_status == SubJobStatusEnum.RUNNING:
            log_detail = LogDetail(logInfo=JobProgressLabel.backup_subjob_running,
                                   logInfoParam=[hostsn, str(self.progress)],
                                   logTimestamp=int(time.time()), logLevel=1)
            output.log_detail = [log_detail]
            return output.dict(by_alias=True)
        copy_size = self.get_copy_size()
        output.data_size = copy_size
        log_detail = LogDetail(logInfo=JobProgressLabel.backup_subjob_success,
                               logInfoParam=[hostsn],
                               logTimestamp=int(time.time()), logLevel=1)
        output.log_detail = [log_detail]
        log.info(f"Backup progress info {output.dict(by_alias=True)}. {self.get_log_common()}")
        return output.dict(by_alias=True)

    def get_copy_size(self):
        """
        获取备份副本大小
        :return:
        """
        path_info = os.path.join(self._cache_area, f"backup_prerequisite{self._job_id}.json")
        with open(path_info, 'r') as f_object:
            try:
                json_dict = json.loads(f_object.read())
            except Exception:
                return 0
        #单位 kb
        copy_size = 0
        copy_name = json_dict.get(DMJsonConstant.BACKUPSETNAME, '')
        if self._db_type == BackupSubType.CLUSTER:
            all_oguid = json_dict.get("groupID", [])
            for oguid in all_oguid:
                copy_path = os.path.join(self._data_area, oguid, copy_name)
                ret, size = scan_dir_size(self._job_id, copy_path)
                if not ret:
                    log.error(f"Get copy size fail. {self.get_log_common()}")
                    return 0
                copy_size += size
            return copy_size
        copy_path = os.path.join(self._data_area, copy_name)
        ret, copy_size = scan_dir_size(self._job_id, copy_path)
        if not ret:
            log.error(f"Get copy size fail. {self.get_log_common()}")
        return copy_size

    def save_sql_errcode(self, sql_result):

        log.info(f"Start save sql errcode.{self.get_log_common()}")
        re_rule = "(.*?)]"
        slot_list = re.findall(re_rule, sql_result)
        errcode = "0"
        status = True
        if slot_list:
            errcode = f"-{slot_list[0]}"
            try:
                str_to_int = int(errcode)
            except Exception:
                status = False
                log.info(f"Get sql errcode fail.{self.get_log_common()}")
        else:
            status = False
            log.info(f"Get sql errcode fail.{self.get_log_common()}")
        json_file = {"errcode": errcode}
        json_path = os.path.join(self._cache_area, f"sql_errcode_{self._job_id}.json")
        output_execution_result(json_path, json_file)
        log.info(f"Save sql errcode succ.{self.get_log_common()}")
        return status

    def get_sql_errcode(self):
        log.info(f"Start get sql errcode.{self.get_log_common()}")
        json_path = os.path.join(self._cache_area, f"sql_errcode_{self._job_id}.json")
        if not os.path.exists(json_path):
            return True, ''
        with open(json_path, 'r') as f_object:
            json_dict = json.loads(f_object.read())
        if json_dict.get("status", '') == "fail":
            return False, ''
        errcode = json_dict.get("errcode", '')
        if not errcode:
            return False, ''
        log.info(f"Get sql errcode succ.{self.get_log_common()}")
        return True, errcode

    def save_get_sql_errcode_times(self):
        log.info(f"Start save get sql errcode times.{self.get_log_common()}")
        path_info = os.path.join(self._cache_area, f"get_errcode_times{self._job_id}.json")
        times = 1
        if os.path.exists(path_info):
            with open(path_info, 'r') as f_object:
                json_dict = json.loads(f_object.read())
            times = json_dict.get("times", 1)
            times += 1
        json_dict = {"times": times}
        output_execution_result_ex(path_info, json_dict)

    def check_times(self):
        log.info(f"Start check times.{self.get_log_common()}")
        path_info = os.path.join(self._cache_area, f"get_errcode_times{self._job_id}.json")
        if not os.path.exists(path_info):
            return False
        with open(path_info, "r") as f_object:
            json_dict = json.loads(f_object.read())
        times = json_dict.get("times", -1)
        if times == -1:
            return False
        if times >= 3:
            log.error(f"More than 3 times.{self.get_log_common()}")
            return False
        return True

    def backup_progress_fail(self, output_):
        if self.log_detail_errcode:
            return self.return_err_info()
        ret, errcode = self.get_sql_errcode()
        self.save_get_sql_errcode_times()
        if not ret:
            return self.return_err_info()
        status = self.check_times()
        if not status:
            return self.return_err_info()
        if not errcode:
            output_.task_status = SubJobStatusEnum.RUNNING
            return output_.dict(by_alias=True)
        self.log_detail_errcode = ErrCode.ERR_EXECUTE_SQL_FAIL
        self.log_detail_param = [errcode]
        return self.return_err_info()

    def return_err_info(self):
        log_detail = LogDetail(logInfoParam=[self._job_id],
                               logInfo=ReportDBLabel.BACKUP_SUB_FAILED,
                               logLevel=DBLogLevel.ERROR,
                               logDetail=self.log_detail_errcode,
                               logDetailParam=self.log_detail_param
                               )
        output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                               taskStatus=SubJobStatusEnum.FAILED.value,
                               progress=Progress.END,
                               logDetail=[log_detail])
        return output.dict(by_alias=True)

    def get_backup_progress(self, backup_history_add_num_):
        self._cache_area = self.get_area_info("cache_repository")
        succ_file_path = os.path.join(self._cache_area, f"backup_status_{self._job_id}.json")
        fail_file_path = os.path.join(self._cache_area, f"sql_errcode_{self._job_id}.json")
        pre_path_info = os.path.join(self._cache_area, f"backup_prerequisite{self._job_id}.json")
        output_info = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, taskStatus=SubJobStatusEnum.FAILED,
                                    progress=Progress.END)
        if os.path.exists(succ_file_path):
            backup_count_query = self.get_backup_history()
            if backup_count_query == backup_history_add_num_:
                output_info.task_status = SubJobStatusEnum.COMPLETED.value
            else:
                log.error(f"Backup history error.{self.get_log_common()}")
                self.log_detail_errcode = ErrCode.ERR_BACKUP_HISTORY
            return output_info
        if os.path.exists(fail_file_path):
            return output_info
        if not os.path.exists(pre_path_info):
            # 框架在执行备份任务30s后开始查询备份进度，针对挂载卡住导致备份在此时仍未开始的情况，返回running状态
            log.info(f"Cannot find pre_path, return running, job_id: {self._job_id}, sub_id: {self._sub_job_id}")
            output_info = SubJobDetails(taskId=self._job_id,
                                    subTaskId=self._sub_job_id,
                                    taskStatus=SubJobStatusEnum.RUNNING,
                                    progress=Progress.START)
            return output_info
        output_info = SubJobDetails(taskId=self._job_id,
                                    subTaskId=self._sub_job_id,
                                    taskStatus=SubJobStatusEnum.RUNNING,
                                    progress=Progress.RUNNING)
        if not self.set_progress(backup_history_add_num_):
            output_info.task_status = SubJobStatusEnum.FAILED.value
        return output_info

    def set_progress(self, backup_history_add_num_):
        if not self.set_version():
            return False
        if self.version != "V8":
            return True
        tool = DmSqlTool(self._db_info)
        backup_monitor_sql = "select PCNT from V$BACKUP_MONITOR;"
        ret, backup_monitor_result = tool.run_disql_tool((backup_monitor_sql,))
        if not ret:
            return False
        result_info = backup_monitor_result[ArrayIndex.INDEX_FIRST_0].strip('\n').split('\n')[-3]
        if "no rows" in result_info or "未选定行" in result_info:
            backup_count_query = self.get_backup_history()
            if backup_history_add_num_ == backup_count_query:
                self.progress = Progress.END
            else:
                self.progress = Progress.RUNNING
        else:
            result_msg = del_space(result_info)
            progress = result_msg[ArrayIndex.INDEX_LAST_1]
            self.progress = progress
        return True

    def judge_backup_status(self, backup_history_add_num_):
        log.info(f"Start judge backup status.{self.get_log_common()}")
        backup_succ_path = os.path.join(self._cache_area, f"backup_status_{self._job_id}.json")
        err_code_path = os.path.join(self._cache_area, f"sql_errcode_{self._job_id}.json")
        status = SubJobStatusEnum.FAILED.value
        if not os.path.exists(backup_succ_path) and not os.path.exists(err_code_path):
            status = SubJobStatusEnum.RUNNING.value
        elif os.path.exists(backup_succ_path):
            backup_count_query = self.get_backup_history()
            if backup_history_add_num_ == backup_count_query:
                status = SubJobStatusEnum.COMPLETED.value
            else:
                self.log_detail_errcode = ErrCode.ERR_BACKUP_HISTORY
        return status

    def save_tablespace_info(self):
        job_dict = self._json_param.get("job", {})
        self._repositories_info = ParamFileUtil.parse_backup_path(job_dict.get("repositories", ""))
        self._cache_area = self._repositories_info.get("cache_repository", [])[ArrayIndex.INDEX_FIRST_0]
        self._meta_area = self._repositories_info.get("meta_repository", [])[ArrayIndex.INDEX_FIRST_0]
        # 从文件中获取backup_dir
        backup_prerequisite_dict = self.get_backup_prerequisite_json()
        if not backup_prerequisite_dict:
            log.error(f"Get backup prerequisite dict.{self.get_log_common()}.")
            return False
        backup_dir = backup_prerequisite_dict.get("backupset_path", "")
        if not backup_dir:
            log.error(f"Get backupset path.{self.get_log_common()}.")
            return False
        if self.backup_save_tablespace(backup_dir):
            return True
        else:
            log.error(f"Save tablespace fail.{self.get_log_common()}.")
            return False

    @exter_attack
    def backup(self):
        """
        执行备份任务
        :return:
        """
        log.info(f"job id: {self._job_id}, Start backup.")
        result_type = self.set_db_param().get("code")
        if result_type:
            return ActionResult(code=ActionResultCode.FAIL).dict(by_alias=True)
        prepare_ret, backup_sql, backup_dir = self.backup_prepare()
        if prepare_ret == "Mount":
            log.info(f'job id: {self._job_id}, mount task')
            return ActionResult(code=ActionResultCode.SUCCESS).dict(by_alias=True)
        if not prepare_ret:
            log.error(f"Failed to prepare backup information.{self.get_log_common()}.")
            return ActionResult(code=ActionResultCode.FAIL).dict(by_alias=True)
        ret, real_path = check_path_in_white_list(backup_dir)
        if not ret:
            log.error(f"Invalid backup set path.")
            return ActionResult(code=ActionResultCode.FAIL).dict(by_alias=True)
        # 保存本次任务备份集路径信息
        if not self.save_backupset_path(backup_dir):
            log.info(f"Failed save backupset path info.{self.get_log_common()}.")
            return ActionResult(code=ActionResultCode.FAIL).dict(by_alias=True)
        tool = DmSqlTool(self._db_info)
        log.info(f'job id: {self._job_id}, start to execute backup sql')
        status, backup_res = tool.run_disql_tool((backup_sql,), timeout=None)
        log.info(f'job id: {self._job_id}, execute backup sql success, status: {status}')
        if not status:
            log.error(f"The backup:{self._job_id} operation failed.{self.get_log_common()}.")
            self.save_sql_errcode(backup_res[0])
            return ActionResult(code=ActionResultCode.FAIL).dict(by_alias=True)
        log.info(f'job id: {self._job_id}, start to after backup')
        self.after_backup()
        log.info(f'job id: {self._job_id}, after backup end')
        return ActionResult(code=ActionResultCode.SUCCESS).dict(by_alias=True)

    def after_backup(self):
        log.info(f'job id: {self._job_id}, after backup type: {self._backup_type}')
        result_tag = True
        if self._backup_type != BackupTypeEnum.LOG_BACKUP:
            if not self.set_copy_id():
                result_tag = False
            if self._db_type == BackupSubType.SINGLE_NODE:
                if not self.save_tablespace_info():
                    log.error(f"Save tablespace info fail.{self.get_log_common()}.")
                    result_tag = False
        log.info(f'job id: {self._job_id}, after backup result flag: {result_tag}')
        if not self.query_backup_copy():
            log.error(f"job id: {self._job_id}, Report copy info fail.{self.get_log_common()}.")
            self.log_detail_errcode = ErrCode.ERR_INVALID_LOG_COPY
            self.report_job_detail()
            result_tag = False
        json_file = {"status": "succ"}
        json_path = os.path.join(self._cache_area, f"backup_status_{self._job_id}.json")
        if not result_tag:
            log.error(f'job id: {self._job_id}, after backup result flag failed: {result_tag}')
            json_file = {"status": "fail"}
            json_path = os.path.join(self._cache_area, f"sql_errcode_{self._job_id}.json")
        log.info(f'job id: {self._job_id}, after backup complete')
        output_execution_result_ex(json_path, json_file)

    def set_exec_node_info(self, uuid):
        job_dict = self._json_param.get("job")
        self._repositories_info = ParamFileUtil.parse_backup_path(job_dict.get("repositories", {}))
        self._cache_area = self._repositories_info.get("cache_repository", [''])[ArrayIndex.INDEX_FIRST_0]
        json_str = json.dumps({"uuid": uuid, "primary_node_num": self._primary_node_num})
        flags = os.O_WRONLY | os.O_CREAT
        modes = stat.S_IWUSR | stat.S_IRUSR
        file_path = f"{self._cache_area}/exec_node{self._job_id}.json"
        check_ret, real_path = check_path_in_white_list(file_path)
        if not check_ret:
            log.error(f'Invalid path.{self.get_log_common()}.')
            return False
        with os.fdopen(os.open(file_path, flags, modes), 'w') as jsonfile:
            jsonfile.write(json_str)
        log.info(f"Set exec node succ.")
        return True

    def get_exec_node_info_json(self):

        job_dict = self._json_param.get("job")
        self._repositories_info = ParamFileUtil.parse_backup_path(job_dict.get("repositories"))
        self._cache_area = self._repositories_info.get("cache_repository")[ArrayIndex.INDEX_FIRST_0]
        json_path = f"{self._cache_area}/exec_node{self._job_id}.json"
        if os.path.exists(json_path):
            with open(json_path, 'r') as jsonfile:
                json_dict = json.loads(jsonfile.read())
                return json_dict
        log.info("Read exec_node json fail.")
        return {}

    def set_repositories(self, backupset_dir_):
        # 获取repositories信息
        job_dict = self._json_param.get("job", {})
        self._db_type = job_dict.get("protectObject", {}).get("subType", '')
        get_repositories = deepcopy(job_dict.get("repositories"))
        copy_info = job_dict.get("copy", [])
        if not copy_info:
            return False, []
        self._copy_id = copy_info[ArrayIndex.INDEX_FIRST_0].get("id", '')
        if not self._copy_id:
            log.error(f"Failed get copy id.{self.get_log_common()}.")
            return False, []
        if self._db_type == BackupSubType.SINGLE_NODE:
            out_repositories = self.set_single_repositories(get_repositories)
        else:
            out_repositories = self.set_cluster_repositories(get_repositories)
        log.info(f"Report the number of replica paths:{len(out_repositories)} out_repositories:{out_repositories}, "
                 f"{self.get_log_common()}")
        return True, out_repositories

    def set_single_repositories(self, get_repositories):
        out_repositories = []
        backupset_name = self.json_dict.get(DMJsonConstant.BACKUPSETNAME, '')
        for repository in get_repositories:
            repository_type = repository.get(DMJsonConstant.REPORITORYTYPE, '')
            if repository_type == RepositoryDataTypeEnum.DATA_REPOSITORY:
                old_remote_path = repository.get(DMJsonConstant.REMOTEPATH)
                repository[DMJsonConstant.REMOTEPATH] = os.path.join(old_remote_path, backupset_name)
                out_repositories.append(repository)
            elif repository_type == RepositoryDataTypeEnum.META_REPOSITORY:
                old_remote_path = repository.get(DMJsonConstant.REMOTEPATH)
                repository[DMJsonConstant.REMOTEPATH] = os.path.join(old_remote_path, self._copy_id)
                out_repositories.append(repository)

        return out_repositories

    def set_cluster_repositories(self, get_repositories):
        out_repositories = []
        backupset_name = self.json_dict.get(DMJsonConstant.BACKUPSETNAME, '')
        all_oguid = self.json_dict.get("groupID", [])
        if not all_oguid:
            log.error(f"Get all oguid fail. {self.get_log_common()}")
            return out_repositories
        for repository in get_repositories:
            repository_type = repository.get(DMJsonConstant.REPORITORYTYPE)
            if repository_type == RepositoryDataTypeEnum.DATA_REPOSITORY:
                old_remote_path = repository.get(DMJsonConstant.REMOTEPATH)
                for oguid in all_oguid:
                    repository_ = deepcopy(repository)
                    repository_[DMJsonConstant.REMOTEPATH] = os.path.join(old_remote_path, oguid, backupset_name)
                    out_repositories.append(repository_)
        return out_repositories

    def get_channel_number(self):
        job_dict = self._json_param.get("job", {})
        job_extend_info = job_dict.get("extendInfo", {})
        channel_number = job_extend_info.get("channel_number", "")
        if not channel_number:
            log.info(f'The number of concurrent calls is not set.{self.get_log_common()}.')
            channel_number = "4"
        return channel_number

    def parses_data_backupset_info(self, backupset_info_):
        """
        解析数据备份集信息
        :return:
        """
        backupset_info = backupset_info_.get("backupsets", {}).get("group", {}).get("backupset", {})
        metadata_info = backupset_info.get("metadata", {})
        sub_backupsets = backupset_info.get("sub_backupset", [])
        if not sub_backupsets:
            sub_backupsets = []
            backupset_dict = dict()
            backupset_dict["file_info"] = backupset_info.get("file_info", {})
            sub_backupsets.append(backupset_dict)
        db_name = metadata_info.get("db_name", '')
        backup_time = metadata_info.get("backup_time", '')
        if not any((sub_backupsets, metadata_info, backupset_info, db_name, backup_time)):
            log.error(f'Parses data backupset info fail.{self.get_log_common()}.')
            return False, {}
        backup_time = timestamp(backup_time)
        table_info_set = set()
        for sub_backupset in sub_backupsets:
            data_info = sub_backupset.get("file_info", {}).get("data_file", [])
            data_info_list = []
            # 校验在备份时对应的数据文件是否做了拆分
            if isinstance(data_info, dict):
                data_info_list.append(data_info)
            else:
                data_info_list = data_info
            for table_space in data_info_list:
                table_name = table_space.get("group_name")
                table_size = table_space.get("file_len")
                tabal_info = (table_name, table_size)
                table_info_set.add(tabal_info)
        table_info_list = []
        for table_tuple in table_info_set:
            table_info_dict = {
                "tabal_name": table_tuple[0],
                "tabal_len": table_tuple[1],
            }
            table_info_list.append(table_info_dict)
        table_info_json_str = json.dumps(table_info_list)
        backupset_msg = {
            "tabal_space_info": [table_info_json_str],
            "dbName": db_name,
            "backupTime": backup_time
        }
        return True, backupset_msg

    def get_data_size(self, job_dict_):
        backup_data_size = 0
        self._repositories_info = ParamFileUtil.parse_backup_path(job_dict_.get("repositories"))
        try:
            backup_data_size_path = os.path.join(
                self._repositories_info.get("cache_repository", [])[ArrayIndex.INDEX_FIRST_0],
                f"data_size{self._job_id}.json"
            )
        except IndexError:
            return backup_data_size
        if os.path.exists(backup_data_size_path):
            with open(backup_data_size_path, "r") as f:
                try:
                    backup_prerequisite_dir = json.loads(f.read())
                except Exception:
                    log.error(f"Failed load json file.{self.get_log_common()}")
                    return backup_data_size
            backup_data_size = backup_prerequisite_dir.get("data_size", 0)
        else:
            log.error(f"Failed make backup_data_size_path.{self.get_log_common()}.")
        return backup_data_size

    def save_data_size(self, data_size_):

        save_path = os.path.join(self._repositories_info.get("cache_repository", [])[ArrayIndex.INDEX_FIRST_0],
                                 f"data_size{self._job_id}.json")
        check_ret, real_path = check_path_in_white_list(save_path)
        if not check_ret:
            log.error(f'Invalid path.{self.get_log_common()}.')
            return False

        data_size = {"data_size": data_size_}
        output_execution_result_ex(save_path, data_size)
        log.info('Save_data_size succ.')
        return True

    def check_backup_prerequisites(self):
        err_code_parameters = ''
        if self._db_status == DbStatus.DB_STATUS_CLOSE:
            log.error(f'The database service is not started.{self.get_log_common()}.')
            return False, ErrCode.DB_NOT_RUNNING, err_code_parameters
        if self._auth_info_status == DbAuthInfo.ERRORS:
            log.error(f'The user authentication information is incorrect.{self.get_log_common()}.')
            return False, ErrCode.AUTH_INFO_ERR, err_code_parameters
        if not self._db_info:
            log.error(f'Set db info fail.{self.get_log_common()}.')
            return False, '', ''
        arch_status = query_db_arch_status(self._db_info)
        if arch_status == DbArchStatus.CLOSE_ARCH:
            log.error(f'The archive mode is not enabled for the database.{self.get_log_common()}.')
            return False, ErrCode.DB_ARCH_NOT_OPEN, err_code_parameters
        if not check_dmapserver_status():
            log.error(f'The DmAPService is not started.{self.get_log_common()}.')
            return False, ErrCode.DMAPSERVER_NOT_RUNNING, err_code_parameters
        return True, '', ''

    def check_nodes_distribution(self):
        """
        在子任务前，检查一台主机上是否有多个主节点
        """
        log.info(f"Start check nodes distribution.{self.get_log_common()}")
        job_dict = self._json_param.get("job", {})
        self._db_type = job_dict.get("protectObject", {}).get("subType", '')
        if self._db_type == BackupSubType.SINGLE_NODE:
            return True, ''
        if not self.set_cluster_info():
            log.error(f"Set nodes info fail.{self.get_log_common()}")
            return False, ''
        nodes_info = self._json_param.get("job", {}).get("protectEnv", {}).get("nodes", [])
        if not nodes_info:
            log.error(f"Get nodes info fail.{self.get_log_common()}")
            return False, ''
        primary_id_port = self.get_primary_info(nodes_info)
        if not primary_id_port:
            log.error(f"Get primary info fail.{self.get_log_common()}")
            return False, ''
        primary_list = []
        repeats_ip_list = []
        for node in nodes_info:
            node_ip = node.get("extendInfo", {}).get("endpoint", '')
            node_id = node.get("id", '')
            inst_port = node.get("extendInfo", {}).get("port")
            ip_port = f"{node_ip}:{inst_port}"
            log.info(f"ip_port:{ip_port}")
            if ip_port in primary_id_port:
                if node_id in primary_list:
                    repeats_ip_list.append(node_ip)
                primary_list.append(node_id)
        if repeats_ip_list:
            log.error(f"Multiple master nodes on the same host.{self.get_log_common()}")
            err_param = ",".join(repeats_ip_list)
            return False, err_param
        log.info(f"Check nodes distribution succ.{self.get_log_common()}")
        return True, ''

    def set_copy_id(self):

        copy_info = self._json_param.get("job", {}).get("copy", [])
        if not copy_info:
            return False
        self._copy_id = copy_info[ArrayIndex.INDEX_FIRST_0].get("id", '')
        if not self._copy_id:
            log.error(f"Failed get copy id.{self.get_log_common()}.")
            return False
        return True

    def get_last_copy_info(self, backup_type=None):
        if not backup_type:
            backup_type = self._backup_type
        last_copy_type = LastCopyType.last_copy_type_dict.get(backup_type, [])
        if not last_copy_type:
            log.error(f"Backup type err.{self.get_log_common()}")
            return {}
        input_param = {
            RpcParamKey.APPLICATION: self._json_param.get("job", {}).get("protectObject"),
            RpcParamKey.TYPES: last_copy_type,
            RpcParamKey.COPY_ID: "",
            ParamKeyConst.JOB_ID: self._job_id
        }
        try:
            result = invoke_rpc_tool_interface(self._job_id, RpcParamKey.QUERY_PREVIOUS_CPOY, input_param)
        except Exception as err_info:
            log.error(f"Get last copy info fail.{self.get_log_common()}")
            return {}
        return result

    def get_area_info(self, area_name):
        job_dict = self._json_param.get("job", {})
        self._repositories_info = ParamFileUtil.parse_backup_path(job_dict.get("repositories", ""))
        if not isinstance(self._repositories_info, dict):
            log.error(f"Get repositories fail.{self.get_log_common()}")
            return ''
        area = self._repositories_info.get(area_name, [])
        if not area:
            return ''
        return area[0]

    def set_version(self):
        version = DamengSource().get_big_version()
        if not version:
            return False
        self.version = version
        return True

    def report_job_detail(self):
        result = self.return_err_info()
        result[ParamKeyConst.JOB_ID] = self._job_id
        try:
            result = invoke_rpc_tool_interface(self._job_id, RpcParamKey.REPORT_JOB_DETAILS, result)
        except Exception as err_info:
            log.error(f"Get last copy info fail.{self.get_log_common()}")
            return False
        return True
