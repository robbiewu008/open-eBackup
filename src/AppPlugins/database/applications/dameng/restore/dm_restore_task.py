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

import configparser
import datetime
import json
import os
import re
import stat
import shutil

from common.common import output_execution_result_ex, output_result_file, execute_cmd, exter_attack, \
    check_command_injection, is_clone_file_system, report_job_details
from common.common_models import SubJobDetails, ActionResult, LogDetail
from common.const import ExecuteResultEnum, ParamConstant, AuthType, JobData, ReportDBLabel, DBLogLevel, Progress, \
    SubJobStatusEnum, RepositoryDataTypeEnum, BackupTypeEnum, RoleType, SubJobPriorityEnum, CMDResultInt, RpcParamKey
from common.file_common import exec_chmod_dir_recursively
from common.job_const import JobNameConst
from common.logger import Logger
from common.util.cmd_utils import get_livemount_path
from dameng.commons.check_information import cheak_instance_status, cheak_backupset_info, read_cheak_backupset_progress
from dameng.commons.common import IniParses, get_hostsn, build_sub_job, clean_dir, check_path_in_white_list, \
    get_env_value, del_file, matching_dameng_field, remove_file_dir, cmd_grep, invoke_rpc_tool_interface
from dameng.commons.const import DMFunKey, ProgressType, RetoreCmd, DMJsonConstant, BackupSubType, BACKUP_MOUNT_PATH, \
    ExecCmdResult, DM_FILE_PATH, DMRestoreSubjobName, DamengStrConstant, ArrayIndex, DmRestoreType, LOG_MOUNT_PATH, \
    DMArchIniStr, ErrCode, DamengStrFormat
from dameng.commons.dameng_tool import DmSqlTool
from dameng.commons.dm_init import DmInitTool
from dameng.commons.dm_rman_tool import DmRmanTool
from dameng.resource.dameng_cluster import DamengCluster
from dameng.resource.damengsource import DamengSource

log = Logger().get_logger("dameng.log")


class DMRestoreTask:

    def __init__(self, p_id, job_id, sub_job_id, json_param):
        self._p_id = p_id
        self._job_id = job_id
        self._sub_job_id = sub_job_id
        self._json_param = json_param
        self._sub_type = ""
        self.log_detail = None
        self._data_path = ""
        self.bin_path = ''
        self._cache_path = ""
        self._mount_path = ''
        self._log_path = ""
        self._log_mount_path = ""
        self._dmini_path = ''
        self.err_code = None
        self._db_path = ""
        self._db_name = ""
        self._db_port = ''
        self._backup_set_name = ""
        self.big_version = ''
        self._version = ''
        self._tablespace_name_list = []
        self._backup_type = 0
        self._base_backup_set_name = ""
        self._restore_type_key = ''
        self._restore_value = ''
        self.restore_type_dict = {}
        self._tablespace_name = ''
        self._target_env_msg = {}
        self._port = 0
        self._group_index = 0
        self._role_type = -1
        self._nodes = []
        self._install_user = ''
        self._db_info = {}
        self._copy_group_id_array = []
        self._log_parent_array = []
        self.init_restore_cmd_dict()
        self.init_exec_cmd_dict_spe()
        self.init_exec_cmd_dict_comm()
        self.init_restore_func_dict()
        self.init_restore_type_dict()

    @staticmethod
    def output_action_result(pid, code, body_err, message):
        """
        将actionResult写入到结果文件,供框架读取
        :return: 
        """
        json_result = ActionResult(code=code, body_err=body_err, message=message).dict(by_alias=True)
        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{pid}")
        log.info("Write file.")
        output_execution_result_ex(file_path, json_result)

    @staticmethod
    def empty_func():
        return True

    @staticmethod
    def gen_job_info(node, group_id_array, node_port, node_index):
        """
        生成子任务需要的job,info
        :return: 
        """
        job_info = dict()
        group_id = node.get(DMJsonConstant.EXTENDINFO, {}).get(DMJsonConstant.GROUPID, "")
        db_path = node.get(DMJsonConstant.EXTENDINFO, {}).get(DMJsonConstant.DBPATH, "")
        db_name = node.get(DMJsonConstant.EXTENDINFO, {}).get(DMJsonConstant.DBNAME, "")
        db_port = node.get(DMJsonConstant.EXTENDINFO, {}).get(DMJsonConstant.DBPORT, "")
        dmini_path = node.get(DMJsonConstant.EXTENDINFO, {}).get(DMJsonConstant.DMINIPATH, "")
        role_type = node.get(DMJsonConstant.EXTENDINFO, {}).get(DMJsonConstant.ROLE, 0)
        index = group_id_array.index((int)(group_id))
        job_info[DMJsonConstant.PORT] = node_port
        job_info[DMJsonConstant.GROUPINDEX] = index
        job_info[DMJsonConstant.DBPATH] = db_path
        job_info[DMJsonConstant.DBNAME] = db_name
        job_info[DMJsonConstant.DBPORT] = db_port
        job_info[DMJsonConstant.DMINIPATH] = dmini_path
        job_info[DMJsonConstant.NODEINDEX] = node_index
        job_info[DMJsonConstant.ROLETYPE] = int(role_type)
        return json.dumps(job_info)

    @staticmethod
    def modify_path_permissions(path: str):
        """
        集群环境，恢复时clone文件系统需要多个主机同时读取，同时需要在文件夹生成新文件，需要文件夹对othoer支持读写权限
        """
        ret, real_path = check_path_in_white_list(path)
        if not ret:
            log.error(f"The path is not in the white list.")
            return False

        modes = stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO | stat.S_ISVTX
        os.chmod(real_path, modes)
        return True

    def init_exec_cmd_dict_spe(self):
        self.exec_cmd_dict_spe = {
            RetoreCmd.RESTORE_ALLOW:
                {
                    DMFunKey.INIT: [self.empty_func],
                    DMFunKey.EXEC_TASK: [self.restore_allow],
                    DMFunKey.EXEC_EXCEPT: [self.empty_func],
                    DMFunKey.EXEC_SUCCESS: [self.empty_func],
                    DMFunKey.EXEC_FAILED: [self.empty_func]
                },
            RetoreCmd.RESTORE_PRE:
                {
                    DMFunKey.EXEC_TASK: [self.restore_pre_task],
                    DMFunKey.EXEC_EXCEPT: [self.exec_pre_when_except],
                    DMFunKey.EXEC_SUCCESS: [self.exec_pre_when_success],
                    DMFunKey.EXEC_FAILED: [self.exec_pre_when_failed]
                },
            RetoreCmd.RESTORE_GEN_SUB:
                {
                    DMFunKey.EXEC_TASK: [self.gen_sub_job],
                    DMFunKey.EXEC_SUCCESS: [DMRestoreTask.empty_func]
                },
            RetoreCmd.RESTORE_EXEC_SUB:
                {
                    DMFunKey.EXEC_TASK: [self.restore_sub_job],
                    DMFunKey.EXEC_EXCEPT: [self.exec_sub_when_except],
                    DMFunKey.EXEC_SUCCESS: [self.exec_sub_when_success],
                    DMFunKey.EXEC_FAILED: [self.exec_sub_when_failed]
                },
            RetoreCmd.RESTORE_POST:
                {
                    DMFunKey.EXEC_TASK: [self.restore_post_job],
                    DMFunKey.EXEC_EXCEPT: [DMRestoreTask.empty_func],
                    DMFunKey.EXEC_SUCCESS: [DMRestoreTask.empty_func],
                    DMFunKey.EXEC_FAILED: [DMRestoreTask.empty_func]
                }
        }
        self.exec_cmd_dict_spe.update(self.restore_cmd_progress)

    def init_restore_cmd_dict(self):
        self.restore_cmd_progress = {
            f"{RetoreCmd.RESTORE_PROGRESS}_{ProgressType.PRE}":
                {
                    DMFunKey.EXEC_TASK: [self.report_progress_pre],
                    DMFunKey.EXEC_EXCEPT: [self.report_progress_when_except],
                    DMFunKey.EXEC_SUCCESS: [DMRestoreTask.empty_func],
                    DMFunKey.EXEC_FAILED: [self.report_progess_when_failed]
                },
            f"{RetoreCmd.RESTORE_PROGRESS}_{ProgressType.RESTORE_SUB}":
                {
                    DMFunKey.EXEC_TASK: [self.report_progress_restore_sub],
                    DMFunKey.EXEC_EXCEPT: [self.report_progress_when_except],
                    DMFunKey.EXEC_SUCCESS: [DMRestoreTask.empty_func],
                    DMFunKey.EXEC_FAILED: [self.report_progess_when_failed]
                },
            f"{RetoreCmd.RESTORE_PROGRESS}_{ProgressType.POST}":
                {
                    DMFunKey.EXEC_TASK: [self.report_progress_post],
                    DMFunKey.EXEC_EXCEPT: [self.report_progress_when_except],
                    DMFunKey.EXEC_SUCCESS: [DMRestoreTask.empty_func],
                    DMFunKey.EXEC_FAILED: [self.report_progess_when_failed]
                }
        }

    def init_exec_cmd_dict_comm(self):
        self.exec_cmd_dict_comm = {
            # 该字典为每个命令默认执行的方法，如有命令要特殊执行某个方法请配置在EXEC_CMD_DICT_SPE字典中
            DMFunKey.INIT: [self.init_param, self.set_big_version],
            DMFunKey.EXEC_TASK: [],
            DMFunKey.EXEC_EXCEPT: [self.exec_cmd_when_execpt_comm],
            DMFunKey.EXEC_SUCCESS: [self.exec_cmd_when_success_comm],
            DMFunKey.EXEC_FAILED: [self.exec_cmd_when_failed_comm]
        }

    def init_restore_func_dict(self):
        self.restore_func_dict = {
            DMRestoreSubjobName.RESTORE: self.restore_data,
            DMRestoreSubjobName.START: self.restore_start_dm,
            DMRestoreSubjobName.UNMOUNT: self.restore_unmount_bind,
            DMRestoreSubjobName.GENERARESTORE: self.single_restore_sub_job
        }

    def init_restore_type_dict(self):
        self.restore_type_dict = {
            DmRestoreType.DATABASE: self.cmd_retore_database,
            DmRestoreType.TIMESTAMP: self.cmd_retore_database,
            DmRestoreType.SCN: self.cmd_retore_database,
            DmRestoreType.TABALSPACE: self.restore_tablespace
        }

    def get_func_by_key(self, cmd, key):
        func_array = []
        one_cmd_dict = self.exec_cmd_dict_spe.get(cmd, {})
        if one_cmd_dict:
            func_array = one_cmd_dict.get(key, [])
            if len(func_array) > 0:
                return func_array
        func_array = self.exec_cmd_dict_comm.get(key)
        return func_array

    def call_func_by_key(self, cmd, key):
        func_array = self.get_func_by_key(cmd, key)
        if len(func_array) <= 0:
            log.error(f"Get fun array failed.{self.get_log_comm()}.")
            return False
        for fun in func_array:
            ret = fun()
            if not ret:
                log.error(f"Exec fun failed.{self.get_log_comm()}.")
                return False
        return True

    def output_other_result(self, json_str):
        """
        将json_str写入到结果文件,供框架读取
        :return: 
        """
        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self._p_id}")
        log.info(f"Write file.{self.get_log_comm()}.")
        output_execution_result_ex(file_path, json_str)

    def init_param(self):
        ret, self._install_user, _ = DamengSource.discover_application()
        if not ret:
            log.info(f"Failed to obtain the database installation user.{self.get_log_comm()}.")
            return False
        self.bin_path = DamengSource.get_bin_path(self._install_user)
        if not self.bin_path:
            log.info(f"Failed to obtain the database installation path.{self.get_log_comm()}.")
            return False
        self._restore_type_key = self.get_restore_type()
        if not self.set_cache_path() or not self.set_data_path():
            return False
        if not self.get_copy_extend_info():
            return False
        if self._restore_type_key in [DmRestoreType.SCN, DmRestoreType.TIMESTAMP]:
            ret = self.restore_get_log_mount()
            if not ret:
                return False
            ret = self.get_log_parent_dir_array()
            if not ret:
                return False
        return True

    def restore_allow(self):

        code = CMDResultInt.SUCCESS.value
        err_code = None
        if not self.set_sub_type():
            json_str = ActionResult(code=CMDResultInt.FAILED.value,
                                    bodyErr=err_code).dict(by_alias=True)
            output_result_file(self._p_id, json_str)
            return False
        if self._sub_type == BackupSubType.CLUSTER:
            json_str = ActionResult(code=code, bodyErr=err_code).dict(by_alias=True)
            output_result_file(self._p_id, json_str)
            return True
        ret, self._target_env_msg = self.get_target_env()
        if not ret:
            json_str = ActionResult(code=CMDResultInt.FAILED.value,
                                    bodyErr=err_code).dict(by_alias=True)
            output_result_file(self._p_id, json_str)
            return False
        if not self.set_node_extendinfo(self._target_env_msg):
            code = CMDResultInt.FAILED.value
        if not self.check_db_server_name():
            code = CMDResultInt.FAILED.value
            err_code = self.err_code
        json_str = ActionResult(code=code, bodyErr=err_code).dict(by_alias=True)
        output_result_file(self._p_id, json_str)
        return True

    @exter_attack
    def restore_pre_task(self):
        log.info(f"Restore params: {self._json_param}.")
        if not self.set_sub_type():
            log.error(f"Set sub type fail {self.get_log_comm()}.")
            return False
        ret, self._mount_path = self.restore_mount_bind()
        if not ret:
            log.error(f"Mount bind fail {self.get_log_comm()}.")
            return False
        ret, self._target_env_msg = self.get_target_env()
        if not ret:
            log.error(f"Get target env fail {self.get_log_comm()}.")
            return False
        if self._sub_type == BackupSubType.SINGLE_NODE:
            return self.restore_pre_single_task()
        elif self._sub_type == BackupSubType.CLUSTER:
            return self.restore_pre_cluster_task()
        log.error(f"Subtype param fail.{self.get_log_comm()}.")
        return False

    def restore_pre_single_task(self):
        log.info(f"Start restore_pre_single_task.{self.get_log_comm()}.")
        if not self.set_node_extendinfo(self._target_env_msg):
            return False
        # 判断实例进程是否存在，如果存在不允许执行恢复任务
        if cheak_instance_status(self._dmini_path):
            log.error(f"Instance running.{self.get_log_comm()}.")
            log_detail = LogDetail(logInfoParam=[self._job_id],
                                   logLevel=DBLogLevel.ERROR,
                                   logDetail=ErrCode.ERR_DB_IS_RUNNING, )
            self.log_detail = [log_detail]
            return False
        # 检查服务名是否重复
        if not self.check_db_server_name():
            return False
        # 检查备份集信息
        backupset_path = os.path.join(self._mount_path, self._backup_set_name)
        ret, info = cheak_backupset_info(backupset_path, f"dmcheck_{self._job_id}")
        if not ret:
            log.error(f"check backupset info error info: {info}")
            log_detail = LogDetail(logInfoParam=[self._job_id],
                                   logLevel=DBLogLevel.ERROR,
                                   logDetailParam=[JobNameConst.RESTORE, info],
                                   logDetail=ErrCode.EXEC_BACKUP_RECOVER_CMD_FAIL, )
            self.log_detail = [log_detail]
            return False
        log.info(f"Restore pre single task succ.{self.get_log_comm()}.")
        return True

    def restore_pre_cluster_task(self):
        if not self.set_nodes():
            log.error(f'Set nodes fail.{self.get_log_comm()}.')
            return False
        flag = 1
        for node in self._nodes:
            if self.cheak_uuid(node):
                flag = 0
                break
        if flag:
            log.error(f"Not node in local.{self.get_log_comm()}.")
            return False
        if not self.set_node_extendinfo(node):
            log.error(f'Set nodes extendinfo fail.{self.get_log_comm()}.')
            return False
        # 判断实例进程是否存在，如果存在不允许执行恢复任务
        if cheak_instance_status(self._dmini_path):
            log_detail = LogDetail(logInfoParam=[self._job_id],
                                   logLevel=DBLogLevel.ERROR,
                                   logDetail=ErrCode.ERR_DB_IS_RUNNING, )
            self.log_detail = [log_detail]
            log.error(f"Instance running.{self.get_log_comm()}.")
            return False
        # 检查备份集信息
        ret, copy_info = self.get_copy()
        if not ret:
            log.error(f'Get copy info fail.{self.get_log_comm()}.')
            return False
        extend_info = copy_info.get(DMJsonConstant.EXTENDINFO, {})
        if extend_info.get(DMJsonConstant.EXTENDINFO, {}):
            extend_info = extend_info.get(DMJsonConstant.EXTENDINFO, {})
        group_id_list = extend_info.get(DMJsonConstant.GROUPID, [])
        if not group_id_list:
            log.error(f"Get groupId from param fail.{self.get_log_comm()}.")
            return False
        primary_info = DamengCluster().get_primary_info(self._dmini_path)
        primary_num = len(group_id_list)
        local_primary_num = len(primary_info)
        if primary_num != local_primary_num:
            log_detail = LogDetail(logInfoParam=[self._job_id],
                                   logLevel=DBLogLevel.ERROR,
                                   logDetailParam=[str(primary_num), str(local_primary_num)],
                                   logDetail=ErrCode.ERR_INCONSISTENT_CLUSTER_TOPOLOGY, )
            self.log_detail = [log_detail]
            log.error(f"Inconsistent cluster topology.{self.get_log_comm()}.")
            return False
        ret = self.check_backupset(group_id_list)
        if not ret:
            log.error(f"Restore pre cluster task fail.{self.get_log_comm()}.")
            return False
        log.info(f"Restore pre cluster task succ.{self.get_log_comm()}.")
        return True

    def check_backupset(self, group_id_list):
        # 集群上报备份副本路径后适配，副本恢复修改权限
        for group_id in group_id_list:
            oguid_path = os.path.join(self._mount_path, group_id)
            if is_clone_file_system(self._json_param):
                if not self.chown_file(oguid_path):
                    log.error(f"Failed to modify the file permission.{self.get_log_comm()}.")
                    return False
            backupset_path = os.path.join(oguid_path, self._backup_set_name)
            ret, info = cheak_backupset_info(backupset_path, f"dmcheck_{self._job_id}")
            if not ret:
                log.error(f"Check backupset info fail.{self.get_log_comm()}.")
                log_detail = LogDetail(logInfoParam=[self._job_id],
                                       logLevel=DBLogLevel.ERROR,
                                       logDetailParam=[JobNameConst.RESTORE, info],
                                       logDetail=ErrCode.EXEC_BACKUP_RECOVER_CMD_FAIL, )
                self.log_detail = [log_detail]
                return False
        return True

    def check_db_server_name(self):
        if os.path.exists(self._dmini_path):
            log.info(f"Dmini file already exist.{self.get_log_comm()}.")
            return True
        ret, uname, _ = DamengSource.discover_application()
        if not ret:
            log.error(f"Get install username fail. {self.get_log_comm()}.")
            return False
        bin_path = DamengSource.get_bin_path(uname)
        if not bin_path:
            log.error(f"Get bin path fail. {self.get_log_comm()}.")
            return False
        server_list = DamengSource.discover_all_server(bin_path)
        if not server_list:
            server_list = DamengSource.discover_all_server("/etc/rc.d/init.d")
        server_name = f"DmService{self._db_name}"
        if server_name in server_list:
            self.err_code = ErrCode.DAMENG_DBNAME_CONFLICT
            return False
        return True

    def check_instance_local(self, node_):

        if not self.cheak_uuid(node_):
            return False
        if not self.set_node_extendinfo(node_):
            log.error(f"Set node extendinfo fail.{self.get_log_comm()}.")
            return False
        self._dmini_path = os.path.join(self._db_path, self._db_name, "dm.ini")
        inst_port_info = DamengSource.get_instancename_port(self._dmini_path)
        port_local = 2
        if len(inst_port_info) < port_local:
            log.error(f"Get inst port fail.{self.get_log_comm()}.")
            return False
        return True

    def cheak_uuid(self, node_):
        uuid = node_.get("uuid", '')
        if not uuid:
            log.error(f"Get uuid fail.{self.get_log_comm()}.")
            return False
        local_hostsn = get_hostsn()
        if uuid != local_hostsn:
            log.error(f"Set node extendinfo fail.{self.get_log_comm()}.")
            return False
        return True

    def cheak_cluster_node_num(self):

        all_nodes = DamengCluster().get_all_node_info(self._dmini_path)
        if not all_nodes:
            log.error(f"Get local primary info fail.{self.get_log_comm()}.")
            return False
        if len(all_nodes) == len(self._nodes):
            return True
        log.error(f"Primary num is not same local_primary_info.{self.get_log_comm()}.")
        return False

    def get_all_group_id_list(self):
        result_array = []
        for node in self._nodes:
            group_id = node.get(DMJsonConstant.EXTENDINFO, {}).get(DMJsonConstant.GROUPID, "")
            if not group_id:
                log.error(f"Get group id failed.{self.get_log_comm()}.")
                return False, result_array
            result_array.append((int)(group_id))
        # 直接返回对列表去重排序之后的值
        new_array = sorted(set(result_array))
        return True, new_array

    @exter_attack
    def gen_sub_job(self):
        """
        拆分子任务实现
        :return:
        """
        backup_sub_type = self._json_param.get(DMJsonConstant.JOB, {}). \
            get(DMJsonConstant.TARGETOBJECT, {}).get(DMJsonConstant.SUBTYPE, "")
        if backup_sub_type != BackupSubType.CLUSTER:
            log.error(f"The database type does not support subtask splitting."
                      f"{self.get_log_comm()}.")
            return False
        ret = self.set_nodes()
        if not ret:
            return False

        ret, group_id_array = self.get_all_group_id_list()
        if not ret:
            return False

        node_index = 0
        sub_job_array = []
        for node in self._nodes:
            host_id = node.get(DMJsonConstant.ID, "")
            node_port = node.get(DMJsonConstant.EXTENDINFO, {}).get(DMJsonConstant.PORT, 0)
            job_type = DMRestoreSubjobName.RESTORE
            job_priority = SubJobPriorityEnum.JOB_PRIORITY_1.value
            job_info = self.gen_job_info(node, group_id_array, node_port, node_index)
            sub_job = build_sub_job(self._job_id, job_priority, host_id, job_info, job_type)
            sub_job_array.append(sub_job)
            job_type = DMRestoreSubjobName.START
            job_priority = SubJobPriorityEnum.JOB_PRIORITY_2.value
            sub_job = build_sub_job(self._job_id, job_priority, host_id, job_info, job_type)
            sub_job_array.append(sub_job)
            role_type = node.get(DMJsonConstant.EXTENDINFO, {}).get(DMJsonConstant.ROLE, 0)
            if int(role_type) == RoleType.PRIMARY.value:
                # 主节点再拆分卸载任务
                job_type = DMRestoreSubjobName.UNMOUNT
                job_priority = SubJobPriorityEnum.JOB_PRIORITY_3.value
                sub_job = build_sub_job(self._job_id, job_priority, host_id, job_info, job_type)
                sub_job_array.append(sub_job)
            node_index += 1
        log.info(f"Gen sub job success.{self.get_log_comm()}.")
        try:
            output_result_file(self._p_id, sub_job_array)
        except Exception as exception_str:
            log.error(f"Write file fail.{self.get_log_comm()}.")
            return False
        return True

    def mkdir_and_chmod(self, path, username, user_group):
        ret, real_path = check_path_in_white_list(path)
        if not ret:
            log.error(f"The path is not in the white list.{self.get_log_comm()}.")
            return False
        # 创建文件夹 并修改权限
        if not os.path.exists(real_path):
            try:
                os.mkdir(real_path)
            except Exception as exception_str:
                log.error(f"Mkdir failed.{self.get_log_comm()}.")
                return False
            try:
                os.chmod(path, stat.S_IRWXU + stat.S_IRGRP + stat.S_IXGRP)
            except Exception as exception_str:
                log.error(f"Chmod failed.{self.get_log_comm()}.")
                return False
            cmd = f"id {username}"
            result_type, out_info, err_info = execute_cmd(cmd)
            if result_type != ExecCmdResult.SUCCESS:
                log.error(f"Get uid and gid failed.")
                return False
            id_info = out_info.replace('(', ' ')
            id_info = id_info.replace('=', ' ')
            id_info = id_info.split(' ')
            uid_info_local = 1
            uid = id_info[uid_info_local]
            gid_info_local = 4
            gid = id_info[gid_info_local]
            try:
                os.lchown(real_path, int(uid), int(gid))
            except Exception as exception_str:
                log.error(f"Chown failed.{self.get_log_comm()}.exception_str：{exception_str}")
                return False
        return True

    def perpare_path(self, username, user_group, path_array):
        for path in path_array:
            ret = self.mkdir_and_chmod(path, username, user_group)
            if not ret:
                return False
        log.info(f"Perpare success.{self.get_log_comm()}.")
        return True

    def restore_mount_log(self):
        log_mount_path = os.path.join(LOG_MOUNT_PATH, self._job_id)
        self._log_mount_path = log_mount_path
        if os.path.ismount(log_mount_path):
            log.info(f"Already mount.{self.get_log_comm()}.")
            return True
        ret, username, user_group = DamengSource.discover_application()
        if not ret or not username:
            log.error(f"Discover application failed.{self.get_log_comm()}.")
            return False
        path_array = [LOG_MOUNT_PATH, log_mount_path]
        ret = self.perpare_path(username, user_group, path_array)
        if not ret:
            log.error(f"Perpare log failed.{self.get_log_comm()}.")
            return False
        cmd = f"mount --bind {self._log_path} {log_mount_path}"
        if check_command_injection(cmd):
            log.error(f"The command contains special characters.{self.get_log_comm()}.")
            return False
        ret, out, err = execute_cmd(cmd)
        if ret != ExecCmdResult.SUCCESS:
            log.error(f"Mount log bind failed.{self.get_log_comm()}.")
            return False
        return True

    def restore_mount_bind(self):
        """
        重新挂载data仓,达梦不允许副本在/tmp目录下
        :return:
        """
        mount_path = os.path.join(BACKUP_MOUNT_PATH, self._job_id)
        if os.path.ismount(mount_path):
            log.info(f"Already mount.{self.get_log_comm()}.")
            return True, mount_path
        ret, username, user_group = DamengSource.discover_application()
        if not ret or not username:
            log.error(f"Discover application failed.{self.get_log_comm()}.")
            return False, ""
        path_array = [BACKUP_MOUNT_PATH, mount_path]
        ret = self.perpare_path(username, user_group, path_array)
        if not ret:
            return False, ""
        cmd = f"mount --bind {self._data_path} {mount_path}"
        if check_command_injection(cmd):
            log.error(f"The command contains special characters.{self.get_log_comm()}.")
            return False, ''
        ret, out, err = execute_cmd(cmd)
        if ret != ExecCmdResult.SUCCESS:
            log.error(f"Mount bind failed.{self.get_log_comm()}.")
            return False, ""
        log.info(f"Mount success.{self.get_log_comm()}.")
        return True, mount_path

    def restore_unmount_bind_comm(self, mount_path):
        if not os.path.ismount(mount_path):
            return True
        cmd = f"umount {mount_path}"
        if check_command_injection(cmd):
            log.error(f"The command contains special characters.{self.get_log_comm()}.")
            return False
        return_code, out_info, _ = execute_cmd(cmd)
        if return_code == ExecCmdResult.SUCCESS:
            log.info(f"Umount success.{self.get_log_comm()}.")
            if os.path.exists(mount_path):
                remove_file_dir(mount_path)
                log.info(f"Del rmtree succ.{self.get_log_comm()}.")
            return True
        log.error(f"Umount failed.{self.get_log_comm()}.")
        return False

    def restore_unmount_bind(self):
        """
        清理自己挂载上来的目录
        :return:
        """
        mount_path = os.path.join(BACKUP_MOUNT_PATH, self._job_id)
        _ = self.restore_unmount_bind_comm(mount_path)
        if self._restore_type_key in [DmRestoreType.SCN, DmRestoreType.TIMESTAMP]:
            _ = self.restore_unmount_bind_comm(self._log_mount_path)

        # 卸载目录不影响子任务执行结果
        return True

    def gen_db_info(self, port, node_index):
        if node_index < 0:
            log.error(f"Get node_index error.{self.get_log_comm()}.")
            return False
        auth_mode = get_env_value(f"job_targetEnv_nodes_{node_index}_auth_authType_{JobData.PID}")
        if auth_mode == str(AuthType.OS_PASSWORD.value):
            self._db_info = {"port": port, "auth_type": AuthType.OS_PASSWORD, "single_or_cluser": "cluser"}
        elif auth_mode == str(AuthType.APP_PASSWORD.value):
            user = f"job_targetEnv_nodes_{node_index}_auth_authKey_{JobData.PID}"
            pwd = f"job_targetEnv_nodes_{node_index}_auth_authPwd_{JobData.PID}"
            self._db_info = {
                "port": port,
                "auth_type": AuthType.APP_PASSWORD,
                "userkey": user,
                "pwdkey": pwd,
                "single_or_cluser": "cluser"
            }
        log.info(f"Gen db info success.{self.get_log_comm()}.")
        return True

    def parse_restore_param_cluster(self):
        job_info = self._json_param.get(DMJsonConstant.SUBJOB, {}).get(DMJsonConstant.JOBINFO, {})
        if not job_info:
            log.error(f"Get job info failed.{self.get_log_comm()}.")
            return False
        job_info = json.loads(job_info)
        self._port = (int)(job_info.get(DMJsonConstant.PORT, -1))
        self._group_index = (int)(job_info.get(DMJsonConstant.GROUPINDEX, -1))
        self._db_path = job_info.get(DMJsonConstant.DBPATH, "")
        self._db_name = job_info.get(DMJsonConstant.DBNAME, "")
        self._dmini_path = job_info.get(DMJsonConstant.DMINIPATH, "")
        self._role_type = job_info.get(DMJsonConstant.ROLETYPE, -1)
        node_index = job_info.get(DMJsonConstant.NODEINDEX, -1)
        is_db_invalid = (self._port == -1 or not self._db_path or not self._db_name)
        is_cluster_invaild = (self._group_index == -1 or self._role_type == -1)
        if is_db_invalid or is_cluster_invaild:
            log.error(f"Get port.group index.dbpath failed.{self.get_log_comm()}.")
            return False
        ret = self.gen_db_info(self._port, node_index)
        if not ret:
            return False
        return True

    def parse_restore_param_signal(self):
        ret, targe_env = self.get_target_env()
        if not ret:
            return False
        self._db_path = targe_env.get(DMJsonConstant.EXTENDINFO, {}).get(DMJsonConstant.DBPATH, "")
        self._dmini_path = targe_env.get(DMJsonConstant.EXTENDINFO, {}).get(DMJsonConstant.DMINIPATH, "")
        self._db_port = targe_env.get(DMJsonConstant.EXTENDINFO, {}).get(DMJsonConstant.DBPORT, "")
        job_extend_info = self._json_param.get(DMJsonConstant.JOB,
                                               {}).get(DMJsonConstant.EXTENDINFO, {})
        target_location = job_extend_info.get(DMJsonConstant.TARGETLOCATION, "")
        if not target_location:
            log.error(f"Target location failed.{self.get_log_comm()}.")
            return False
        # 新位置重新取一下db_name,单机的新位置恢复,db_name前面已经在副本里面获取
        if target_location == DMJsonConstant.NEW:
            self._db_name = job_extend_info.get(DMJsonConstant.DBNAME, '')
            self._dmini_path = os.path.join(self._db_path, self._db_name, 'dm.ini')
        if not self._db_path or not self._db_name or not self._dmini_path:
            log.error(f"Get dbpath.dbname failed.{self.get_log_comm()}.")
            return False
        return True

    def parse_job_info(self):
        backup_sub_type = self._json_param.get(DMJsonConstant.JOB, {}). \
            get(DMJsonConstant.TARGETOBJECT, {}).get(DMJsonConstant.SUBTYPE, "")
        if backup_sub_type != BackupSubType.CLUSTER:
            return self.parse_restore_param_signal()
        else:
            return self.parse_restore_param_cluster()

    def find_backup_parent_dir(self, mount_path):
        """
        查找备份集的父路劲，因为集群备份根据组ID做了分割
        :return:
        """
        backup_sub_type = self._json_param.get(DMJsonConstant.JOB, {}). \
            get(DMJsonConstant.TARGETOBJECT, {}).get(DMJsonConstant.SUBTYPE, "")
        if backup_sub_type != BackupSubType.CLUSTER:
            path = mount_path
        else:
            # 前置任务中校验了备份拓扑结构一致
            group_id_str = self._copy_group_id_array[self._group_index]
            path = os.path.join(mount_path, group_id_str)
        if not os.path.exists(path):
            log.error(f"Get backup set parent dir failed.{self.get_log_comm()}.")
            return False, ""
        return True, path

    def chown_file(self, file_path):
        ret, username, user_group = DamengSource.discover_application()
        if not ret or not username:
            log.error(f"Discover application failed.{self.get_log_comm()}.")
            return False
        cmd = f"chown {username}:{user_group} {file_path}"
        ret, out, err = execute_cmd(cmd)
        if ret != ExecCmdResult.SUCCESS:
            log.error(f"Chown failed.{self.get_log_comm()}.")
            return False
        return True

    def create_arch_ini(self):
        dmarch_ini = self._dmini_path.replace("dm.ini", "dmarch.ini")
        if os.path.exists(dmarch_ini):
            log.warn(f"Dmarch.ini already exist.{self.get_log_comm()}.")
            return True
        # 生成dmarch.ini文件
        arch_path = self._dmini_path.replace("dm.ini", "arch")
        ini_conf = configparser.ConfigParser()
        ini_conf.add_section(DMArchIniStr.ARCHIVE_LOCAL)
        ini_conf.set(DMArchIniStr.ARCHIVE_LOCAL, DMArchIniStr.ARCH_TYPE, DMArchIniStr.ARCH_TYPE_VALUE)
        ini_conf.set(DMArchIniStr.ARCHIVE_LOCAL, DMArchIniStr.ARCH_DEST, arch_path)
        ini_conf.set(DMArchIniStr.ARCHIVE_LOCAL, DMArchIniStr.ARCH_FILE_SIZE, \
                     DMArchIniStr.ARCH_FILE_SIZE_VALUE)
        ini_conf.set(DMArchIniStr.ARCHIVE_LOCAL, DMArchIniStr.ARCH_SPACE_LIMIT, \
                     DMArchIniStr.ARCH_SPACE_LIMIT_VALUE)
        ini_conf.set(DMArchIniStr.ARCHIVE_LOCAL, DMArchIniStr.ARCH_HANG_FLAG, DMArchIniStr.ARCH_HANG_FLAG_VALUE)

        flags = os.O_WRONLY | os.O_CREAT
        modes = stat.S_IRUSR | stat.S_IWUSR + stat.S_IRGRP + stat.S_IROTH
        try:
            with os.fdopen(os.open(dmarch_ini, flags, modes), 'w') as fout:
                ini_conf.write(fout)
        except Exception as exception_str:
            log.error(f"Create dmarch.ini failed.{self.get_log_comm()}.")
            return False
        os.chmod(dmarch_ini, stat.S_IRUSR | stat.S_IWUSR + stat.S_IRGRP + stat.S_IROTH)
        ret = self.chown_file(dmarch_ini)
        if not ret:
            return False
        log.info(f"Create dmarch.ini success.{self.get_log_comm()}.")
        return True

    def cmd_init_database(self, backup_set_path):
        if os.path.exists(self._dmini_path):
            log.info(f"Dmini file already exist.{self.get_log_comm()}.")
            if not self._db_port:
                return True
            # 修改PORT_NUM
            self.modify_dm_ini_port_num()
            log.info(f'modify dm.ini port num.{self.get_log_comm()}.')
            return True
        if self._db_port:
            cmd = f"path={self._db_path} db_name={self._db_name} auto_overwrite=1 ARCH_FLAG=1 PORT_NUM={self._db_port}"
        else:
            cmd = f"path={self._db_path} db_name={self._db_name} auto_overwrite=1 ARCH_FLAG=1"
        if check_command_injection(cmd):
            log.error(f"The command also contains special characters.{self.get_log_comm()}.")
            return False
        ret, out = DmInitTool().run_dm_init(cmd)
        if not ret:
            location_path = os.path.join(self._db_path, self._db_name)
            if f"create dir '{location_path}' failed" in out:
                log_detail = ErrCode.PERMISSION_ERROR
                log.error(f"The path permission is error.{self.get_log_comm()}.")
            else:
                log_detail = ErrCode.ERR_DB_INIT_FAIL
                log.error(f"Dmini failed.{self.get_log_comm()}.")
            log_detail = LogDetail(logInfoParam=[self._job_id],
                                   logInfo=ReportDBLabel.RESTORE_SUB_FAILED,
                                   logLevel=DBLogLevel.ERROR,
                                   logDetail=log_detail,
                                   )
            self.log_detail = [log_detail]
            return False
        ret = self.create_arch_ini()
        if not ret:
            return False
        log.info(f"Init database success.{self.get_log_comm()}.")
        return True

    def modify_dm_ini_port_num(self):
        flag = True
        bak_file = f'{self._dmini_path}.bak'
        shutil.copy(self._dmini_path, bak_file)
        with open(self._dmini_path, mode='r', encoding='utf-8') as f1, \
                open(f'{bak_file}', mode='w', encoding='utf-8') as f2:
            for line in f1:
                if not line.strip().startswith('PORT_NUM'):
                    f2.write(line)
                    continue
                params = line.split('#')
                if len(params) < 1:
                    log.error(f'file format is not correct')
                    flag = False
                    break
                datas = params[0].strip().split('=')
                if len(datas) < 2:
                    log.error(f'file data format is not correct')
                    flag = False
                    break
                data = datas[-1].strip()
                if not data.isdigit():
                    log.error(f'get port failed')
                    flag = False
                    break
                new_line = line.replace(data, str(self._db_port))
                f2.write(new_line)
        if flag:
            os.remove(self._dmini_path)
            os.rename(f'{bak_file}', self._dmini_path)
        else:
            os.remove(f'{bak_file}')

    def cmd_retore_database(self, backup_set_path):
        base_backupset_path = os.path.join(backup_set_path, self._backup_set_name)
        if is_clone_file_system(self._json_param):
            if not self.modify_path_permissions(base_backupset_path):
                log.error(f"Failed to modify the permission.{self.get_log_comm()}.")
                return False
        restore_cmd = f"RESTORE DATABASE '{self._dmini_path}' FROM BACKUPSET " \
                      f"'{base_backupset_path}'"
        result_info = DmRmanTool().run_rman_tool((restore_cmd, "exit;"), DM_FILE_PATH,
                                                 file_name_id=f"{JobData.JOB_ID}_{JobData.SUB_JOB_ID}")
        if not result_info.get("result", False):
            rman_out = result_info.get('out_info')
            self.set_rman_err_info(rman_out)
            log.error(f"Exec rman failed.{self.get_log_comm()}.err_info:{rman_out}")
            return False
        log.info(f"Exec rman success.{self.get_log_comm()}.")
        return True

    def get_log_parent_dir_array(self):
        ret, copy = self.get_copy(True)
        if not ret:
            return False
        log.info(f"copy: {copy}, {self.get_log_comm()}.")
        repositories = copy.get(DMJsonConstant.REPORITTORIES, [])
        if len(repositories) <= 0:
            log.error(f"Repositories count error.{self.get_log_comm()}.")
            return False
        log.info(f"repositories: {repositories}, {self.get_log_comm()}.")
        for repositorie in repositories:
            if repositorie.get(DMJsonConstant.REPORITORYTYPE, "") == \
                    RepositoryDataTypeEnum.LOG_REPOSITORY.value:
                path_array = repositorie.get(DMJsonConstant.PATH, [])
                log.info(f"path_array: {path_array}, {self.get_log_comm()}.")
                if len(path_array) <= 0:
                    continue
                path = path_array[0]
                path_ex = (path.split("/"))[-1]
                if "-" in path_ex:
                    log.info(f"path_ex: {path_ex}, {self.get_log_comm()}.")
                    self._log_parent_array.append(path_ex)
        if len(self._log_parent_array) <= 0:
            log.error(f"Get log parent array failed.{self.get_log_comm()}.")
            return False
        log.info(f"Get log parent array success.{self.get_log_comm()}.")
        return True

    def get_local_achive_dir(self):
        archini_path = self._dmini_path.replace('dm.ini', "dmarch.ini")
        if not os.path.exists(archini_path):
            log.error(f"Archini file not exist.{self.get_log_comm()}.")
            return False, ""
        result = matching_dameng_field(DamengStrConstant.DM_ARCH_DEST, archini_path)
        if len(result) != 1:
            log.error(f"Get arch path failed.{self.get_log_comm()}.")
            return False, ""
        arch_path = result[0]

        arch_path = arch_path.strip()
        if not arch_path.startswith('/'):
            arch_path = os.path.join(self.bin_path, arch_path)
        return True, arch_path

    def exec_restore_archive(self, arch_path, par_path):
        log_path = ""
        for log_path in os.listdir(par_path):
            if "DM" in log_path:
                break
        if not log_path:
            log.error(f"Dm log dir not exist.{self.get_log_comm()}.")
            return False
        backup_set_path = os.path.join(par_path, log_path)
        exec_chmod_dir_recursively(par_path, 0o777)
        cmd = f"RESTORE ARCHIVE LOG FROM BACKUPSET '{backup_set_path}' TO ARCHIVEDIR '{arch_path}' OVERWRITE 3"
        result_info = DmRmanTool().run_rman_tool((cmd, "exit;"), DM_FILE_PATH,
                                                 file_name_id=f"{JobData.JOB_ID}_{JobData.SUB_JOB_ID}")
        out = result_info.get("out_info", "")
        if not result_info.get("result", False):
            self.set_rman_err_info(out)
            log.error(f"Exec rman failed.{self.get_log_comm()}.err_info:{out}")
            return False
        log.info(f"Exec rman success.{self.get_log_comm()}.")
        return True

    def cmd_restore_archive(self, arch_path):
        for path in self._log_parent_array:
            log_path = os.path.join(self._log_mount_path, path)
            log.info(f"log_path: {log_path}, {self.get_log_comm()}")
            if os.path.exists(log_path):
                log.info(f"Path: {log_path} exists, {self.get_log_comm()}.")
                ret = self.exec_restore_archive(arch_path, log_path)
                if not ret:
                    return False
        return True

    def gen_recover_rman_cmd_log(self):
        value_str = ""
        if self._restore_type_key == DmRestoreType.SCN:
            value_str = f"UNTIL LSN {self._restore_value}"
        if self._restore_type_key == DmRestoreType.TIMESTAMP:
            value_str = f"UNTIL TIME '{datetime.datetime.fromtimestamp(int(self._restore_value))}'"
        return value_str

    def cmd_recover_log(self, backup_set_path):
        ret, arch_path = self.get_local_achive_dir()
        if not ret:
            return False
        ret = self.cmd_restore_archive(arch_path)
        if not ret:
            log.error(f"Recover archive failed.{self.get_log_comm()}.")
            return False
        log.info(f"Recover archive success.{self.get_log_comm()}.")
        # 获取LSN 或者TIME
        value_str = self.gen_recover_rman_cmd_log()
        if not value_str:
            log.error(f"Gen value str failed.{self.get_log_comm()}.")
            return False
        recover_cmd_array = [
            f"RECOVER  DATABASE '{self._dmini_path}' WITH ARCHIVEDIR '{arch_path}' {value_str}",
            f"RECOVER  DATABASE '{self._dmini_path}' UPDATE DB_MAGIC"
        ]
        for cmd in recover_cmd_array:
            result_info = DmRmanTool().run_rman_tool((cmd, "exit;"), DM_FILE_PATH,
                                                     file_name_id=f"{JobData.JOB_ID}_{JobData.SUB_JOB_ID}")
            out = result_info.get("out_info", "")
            if not result_info.get("result", False):
                self.set_rman_err_info(out)
                log.error(f"Exec rman failed.{self.get_log_comm()}.err_info:{out}")
                return False
            log.info(f"Exec rman success.{self.get_log_comm()}.")
            if self.big_version == "V7":
                break
        return True

    def cmd_recover(self, backup_set_path):
        recover_dict = {
            DmRestoreType.DATABASE: self.cmd_recover_database,
            DmRestoreType.TIMESTAMP: self.cmd_recover_log,
            DmRestoreType.SCN: self.cmd_recover_log
        }
        # 表空间恢复区别于数据库恢复和指定时间点恢复
        if self._restore_type_key == DmRestoreType.TABALSPACE:
            return True
        func = recover_dict.get(self._restore_type_key, None)
        if not func:
            log.error(f"Type error.{self.get_log_comm()}.")
            return False
        ret = func(backup_set_path)
        if not ret:
            log.error(f"Recover failed.{self.get_log_comm()}.")
            return False
        log.info(f"Recover success.{self.get_log_comm()}.")
        return True

    def cmd_recover_database(self, backup_set_path):
        recover_cmd = f"RECOVER  DATABASE '{self._dmini_path}' FROM BACKUPSET \
            '{os.path.join(backup_set_path, self._backup_set_name)}'"
        for _ in [1, 2]:
            result_info = DmRmanTool().run_rman_tool((recover_cmd, "exit;"), DM_FILE_PATH,
                                                     file_name_id=f"{JobData.JOB_ID}_{JobData.SUB_JOB_ID}")
            out = result_info.get("out_info", "")
            if not result_info.get("result", False):
                self.set_rman_err_info(out)
                log.error(f"Exec rman failed.{self.get_log_comm()}.err_info:{out}")
                return False
            log.info(f"Exec rman success.{self.get_log_comm()}.")
            if self.big_version == 'V7':
                break
            recover_cmd = f"RECOVER  DATABASE '{self._dmini_path}' UPDATE DB_MAGIC"
        return True

    def restore_data(self):
        """
        恢复数据的函数
        :return:
        """
        backup_sub_type = self._json_param.get(DMJsonConstant.JOB, {}). \
            get(DMJsonConstant.TARGETOBJECT, {}).get(DMJsonConstant.SUBTYPE, "")
        log_detail = LogDetail(logInfo=ReportDBLabel.RESTORE_SUB_START_COPY, logInfoParam=[self._sub_job_id],
                               logLevel=DBLogLevel.INFO.value)
        report_job_details(self._p_id, SubJobDetails(taskId=self._job_id, progress=100,
                                                     subTaskId=self._sub_job_id, logDetail=[log_detail],
                                                     taskStatus=SubJobStatusEnum.RUNNING.value))
        ret = self.parse_job_info()
        if not ret:
            return False
        ret, mount_path = self.restore_mount_bind()
        if not ret:
            return False
        if self._restore_type_key in [DmRestoreType.SCN, DmRestoreType.TIMESTAMP]:
            ret = self.restore_mount_log()
            if not ret:
                return False
        log.info(f"Mount log success.{self.get_log_comm()}.")
        ret, parent_path = self.find_backup_parent_dir(mount_path)
        if not ret:
            return False
        cmd_retore_func = self.restore_type_dict.get(self._restore_type_key, "")
        if not cmd_retore_func:
            return False
        exec_dict = [self.cmd_init_database, cmd_retore_func, self.cmd_recover]
        exec_chmod_dir_recursively(parent_path, 0o777)
        for func in exec_dict:
            ret = func(parent_path)
            if not ret:
                return False
        log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS, logInfoParam=[self._sub_job_id],
                               logLevel=DBLogLevel.INFO.value)
        report_job_details(self._p_id, SubJobDetails(taskId=self._job_id, progress=100,
                                                     subTaskId=self._sub_job_id, logDetail=[log_detail],
                                                     taskStatus=SubJobStatusEnum.RUNNING.value))
        return True

    def get_restore_type(self):
        extend_info = self._json_param.get(DMJsonConstant.JOB,
                                           {}).get(DMJsonConstant.EXTENDINFO, {})
        restore_timestamp = extend_info.get(DMJsonConstant.RESTORETIMESTAMP, "")
        restore_type = DmRestoreType.DATABASE
        if restore_timestamp:
            restore_type = DmRestoreType.TIMESTAMP
            self._restore_value = restore_timestamp
        sub_objects = self._json_param.get(DMJsonConstant.JOB, {}).get(DMJsonConstant.RESTORESUBOBJECTS, [])
        if sub_objects:
            restore_type = DmRestoreType.TABALSPACE
            self._restore_value = sub_objects
        restore_scn = extend_info.get(DMJsonConstant.RESTORESCN, "")
        if restore_scn:
            restore_type = DmRestoreType.SCN
            self._restore_value = restore_scn
        return restore_type

    def get_server_name(self, bin_path, server_name_list):
        for server_name in server_name_list:
            server_path = os.path.join(bin_path, server_name)
            result = matching_dameng_field("INI_PATH", server_path)
            if len(result) != 1:
                continue
            ini_path = result[0].replace('\"', '')
            ini_path = ini_path.replace('\'', '')
            if self._dmini_path == ini_path:
                return True, server_name
        log.error(f"Find server name failed.{self.get_log_comm()}.")
        return False, ""

    def get_watcher_server_name(self, bin_path, server_name_list):
        for server_name in server_name_list:
            server_path = os.path.join(bin_path, server_name)
            result = matching_dameng_field("INI_PATH", server_path)
            if len(result) != 1:
                continue
            server_ini_path = result[0].replace('\"', '')
            server_ini_path = server_ini_path.replace('\'', '')
            result = matching_dameng_field("INST_INI", server_ini_path)
            if self._dmini_path in result:
                return True, server_name
        log.error(f"Find watcher server name failed.{self.get_log_comm()}.")
        return False, ""

    def get_server_bin_path(self):
        ret, username, _ = DamengSource.discover_application()
        if not ret or not username:
            log.error(f"Discover application failed.{self.get_log_comm()}.")
            return False, ""
        bin_path = DamengSource.get_bin_path(username)
        if not bin_path:
            log.error(f"Get bin path failed.{self.get_log_comm()}.")
            return False, ""
        return True, bin_path

    def stop_watcher_server(self, watch_path):
        cmd = DamengStrFormat.CHECK_DMWATCH_RUNNING_CMD.format(self._install_user, watch_path)
        result = cmd_grep("running", cmd)
        if not result:
            log.error("The path not running.")
            return True
        log.info(f"Need stop the path,{self.get_log_comm()},")
        cmd = DamengStrFormat.STOP_DMWATCH_CMD.format(self._install_user, watch_path)
        ret, out, err = execute_cmd(cmd)
        if ret != ExecCmdResult.SUCCESS:
            log.error(f"Stop the path failed.{self.get_log_comm()}.")
            return False
        log.info(f"Stop the path success.{self.get_log_comm()}.")
        return True

    def start_watcher_server(self, watch_path):
        cmd = DamengStrFormat.CHECK_DMWATCH_RUNNING_CMD.format(self._install_user, watch_path)
        ret, out, err = execute_cmd(cmd)
        if ret == ExecCmdResult.SUCCESS:
            log.info(f"The path is running.")
            return True
        log.info(f"Need start the path.{self.get_log_comm()}.")
        cmd = DamengStrFormat.START_DMWATCH_CMD.format(self._install_user, watch_path)
        ret, out, err = execute_cmd(cmd)
        if ret != ExecCmdResult.SUCCESS:
            log.error(f"Start the path failed.{self.get_log_comm()}.")
            return False
        log.info(f"Start the path success.{self.get_log_comm()}.")
        return True

    def update_db_standby(self):
        # 为备机设置备状态
        sqls = [
            "SP_SET_PARA_VALUE(1, 'ALTER_MODE_STATUS', 1);",
            "alter database standby;",
            "SP_SET_PARA_VALUE(1, 'ALTER_MODE_STATUS', 0);"
        ]
        tool = DmSqlTool(self._db_info)
        sql_status, result = tool.run_disql_tool(sqls, mpp_type='local')
        if not sql_status:
            log.error(f"Update db standby failed.{self.get_log_comm()}.{result}")
            return False
        log.info(f"Update db standby success.{self.get_log_comm()}.")
        return True

    def start_standby_db(self):
        ret, bin_path = self.get_server_bin_path()
        if not ret:
            return False
        server_name_list = DamengSource.discover_all_server(bin_path, DamengStrConstant.DM_WATCHER)
        server_path = bin_path
        if not server_name_list:
            # redhat6版本数据库服务文件存放路径为/etc/rc.d/init.d
            server_path = "/etc/rc.d/init.d"
            server_name_list = DamengSource.discover_all_server(server_path, DamengStrConstant.DM_WATCHER)
        ret, server_name = self.get_watcher_server_name(server_path, server_name_list)
        if not ret:
            return False
        wantch_path = os.path.join(server_path, server_name)
        ret = self.stop_watcher_server(wantch_path)
        if not ret:
            return False
        # 更新数据库失败 不影响人物结果
        _ = self.update_db_standby()
        ret = self.start_watcher_server(wantch_path)
        if not ret:
            return False
        return True

    def restore_tablespace(self, parent_path_):
        log.info("Start restore_tablespace.")
        if not self.set_tablespace_name_list():
            log.error(f'Get tablespace name list fail.{self.get_log_comm()}.')
            return False
        # 如果同时有系统表空间和其他表空间做恢复，优先恢复系统表空间
        if "SYSTEM" in self._tablespace_name_list:
            index_num = self._tablespace_name_list.index("SYSTEM")
            self._tablespace_name_list.pop(index_num)
            self._tablespace_name_list.insert(0, "SYSTEM")
        for self._tablespace_name in self._tablespace_name_list:
            exec_dict = [self.cmd_restore_tablespace, self.cmd_recover_tablespace]
            for func in exec_dict:
                ret = func(parent_path_)
                if not ret:
                    return False
        return True

    def cmd_restore_tablespace(self, backup_set_path):
        restore_cmd = f"RESTORE DATABASE '{self._dmini_path}' TABLESPACE {self._tablespace_name} FROM BACKUPSET " \
                      f"'{os.path.join(backup_set_path, self._backup_set_name)}'"
        result_info = DmRmanTool().run_rman_tool((restore_cmd, "exit;"), DM_FILE_PATH,
                                                 file_name_id=f"{JobData.JOB_ID}_{JobData.SUB_JOB_ID}")
        out = result_info.get("out_info", "")
        if "invalid tablespace name" in out:
            log_detail = LogDetail(logInfoParam=[self._job_id],
                                   logInfo=ReportDBLabel.RESTORE_SUB_FAILED,
                                   logLevel=DBLogLevel.ERROR,
                                   logDetail=ErrCode.TABLESPACE_NOT_EXISTS,
                                   logDetailParam=[self._tablespace_name])
            self.log_detail = [log_detail]
            return False
        if not result_info.get("result", False):
            self.set_rman_err_info(out)
            log.error(f"Exec rman failed.{self.get_log_comm()}.err_info:{out}")
            return False
        log.info(f"Exec rman success.{self.get_log_comm()}.")
        return True

    def cmd_recover_tablespace(self, backup_set_path):
        if self.big_version != "V8":
            return True
        copy_time = self._json_param.get('job', {}).get('copies', [])[0].get('extendInfo', {}).get('backupTime', '')
        time_json = datetime.datetime.fromtimestamp(int(copy_time))
        copy_backup_time = time_json.strftime("%Y-%m-%d %H:%M:%S")
        recover_cmd = f"RECOVER  DATABASE '{self._dmini_path}' TABLESPACE {self._tablespace_name} WITH ARCHIVEDIR " \
                      f"'{os.path.join(backup_set_path, self._backup_set_name)}' UNTIL TIME '{copy_backup_time}'"
        log.info(f'job id: {self._job_id}, recover cmd with time: {recover_cmd}')
        # 先使用带时间点的命令恢复，失败后使用不带时间点的命令进行恢复
        result_info = DmRmanTool().run_rman_tool((recover_cmd, "exit;"), DM_FILE_PATH,
                                                 file_name_id=f"{JobData.JOB_ID}_{JobData.SUB_JOB_ID}")
        if not result_info.get('result', False):
            log.info(f'result_info: {result_info}')
            self.report_tablespace_not_incomplete()
            recover_cmd = f"RECOVER  DATABASE '{self._dmini_path}' TABLESPACE {self._tablespace_name}"
            log.info(f'job id: {self._job_id}, recover cmd: {recover_cmd}')
            result_info = DmRmanTool().run_rman_tool((recover_cmd, "exit;"), DM_FILE_PATH,
                                                     file_name_id=f"{JobData.JOB_ID}_{JobData.SUB_JOB_ID}")
        out = result_info.get("out_info", "")
        if not result_info.get("result", False):
            self.set_rman_err_info(out)
            log.error(f"Exec rman failed.{self.get_log_comm()}.err_info:{out}")
            return False
        log.info(f"Exec rman success.{self.get_log_comm()}.")
        return True

    def restore_start_dm(self):
        """
        恢复成功之后启动数据库
        :return: 
        """
        ret = self.parse_job_info()
        if not ret:
            return True
        ret, bin_path = self.get_server_bin_path()
        if not ret:
            return True
        server_name_list = DamengSource.discover_all_server(bin_path)
        server_path = bin_path
        if not server_name_list:
            # redhat6版本数据库服务文件存放路径为/etc/rc.d/init.d
            server_path = "/etc/rc.d/init.d"
            server_name_list = DamengSource.discover_all_server(server_path)
        ret, server_name = self.get_server_name(server_path, server_name_list)
        if not self.set_sub_type():
            return True
        if not ret and self._sub_type == BackupSubType.CLUSTER:
            return True
        if not ret:
            return self.restore_start_single(bin_path)
        start_path = os.path.join(server_path, server_name)
        cmd = DamengStrFormat.DB_RESTART_CMD.format(self._install_user, start_path)
        ret, out, err = execute_cmd(cmd)
        if ret != ExecCmdResult.SUCCESS:
            log.error(f"Start failed.{self.get_log_comm()}.")
        else:
            log.info(f"Start success.{self.get_log_comm()}.")
        # 如果当前是备节点，还需要去恢复备状态
        if self._role_type == RoleType.STANDBY.value:
            _ = self.start_standby_db()
        return True

    def restore_start_single(self, bin_path):
        """
        使用命令启动数据库，不使用服务拉起
        """
        cmd = f'su - {self._install_user} -c "{bin_path}/dmserver {self._dmini_path} -noconsole 1>/dev/null 2>&1 &"'
        ret, out, err = execute_cmd(cmd)
        if ret != ExecCmdResult.SUCCESS:
            log.error('Start db fail')
        return True

    def single_restore_sub_job(self):
        fun_array = [self.restore_data, self.restore_start_dm, self.restore_unmount_bind]
        for func in fun_array:
            ret = func()
            if not ret:
                return False
        return True

    @exter_attack
    def restore_sub_job(self):
        self.write_progress_file(SubJobStatusEnum.RUNNING.value, 1, ProgressType.RESTORE_SUB)
        job_name = self._json_param.get(DMJsonConstant.SUBJOB, {}).get(DMJsonConstant.JOBNAME, "")
        func = self.restore_func_dict.get(job_name, None)
        if not func:
            log.error(f"Unknown sub job.{self.get_log_comm()}.")
            return False
        ret = func()
        if not ret:
            log.error(f"Exec sub job failed.{self.get_log_comm()}.")
            return False
        return True

    @exter_attack
    def restore_post_job(self):
        """
        执行后置子任务
        :return: 
        """
        self.write_progress_file(SubJobStatusEnum.RUNNING.value, 1, ProgressType.POST)
        clean_dir(self._cache_path)
        backup_info_path = f"{DM_FILE_PATH}/dmrman_cmd_dmcheck_{self._job_id}.log"
        remove_file_dir(backup_info_path)
        self._log_mount_path = os.path.join(LOG_MOUNT_PATH, self._job_id)
        self.restore_unmount_bind()
        log.info(f"Restore post job success.{self.get_log_comm()}.")
        return True

    def init_fun(self, cmd):
        return self.call_func_by_key(cmd, DMFunKey.INIT)

    def exec_cmd_when_execpt(self, cmd):
        return self.call_func_by_key(cmd, DMFunKey.EXEC_EXCEPT)

    def exec_cmd_when_success(self, cmd):
        return self.call_func_by_key(cmd, DMFunKey.EXEC_SUCCESS)

    def exec_cmd_when_failed(self, cmd):
        return self.call_func_by_key(cmd, DMFunKey.EXEC_FAILED)

    def dispatch_task(self, cmd):
        return self.call_func_by_key(cmd, DMFunKey.EXEC_TASK)

    def get_log_comm(self):
        return f"pid:{self._p_id} jobId:{self._job_id} subjobId:{self._sub_job_id}"

    def exec_cmd_when_success_comm(self):
        self.output_action_result(self._p_id, ExecuteResultEnum.SUCCESS.value, 0, "")
        return True

    def exec_cmd_when_failed_comm(self):
        self.output_action_result(self._p_id, ExecuteResultEnum.INTERNAL_ERROR.value, 0, "")
        return True

    def exec_cmd_when_execpt_comm(self):
        self.output_action_result(self._p_id, ExecuteResultEnum.INTERNAL_ERROR.value, 0, "")
        return True

    def gen_failed_progress(self):
        progress_str = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                                     taskStatus=SubJobStatusEnum.FAILED.value,
                                     progress=100
                                     )
        json_str = progress_str.dict(by_alias=True)
        return json_str

    def gen_success_progress(self):
        progress_str = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                                     taskStatus=SubJobStatusEnum.COMPLETED.value,
                                     progress=100)
        json_str = progress_str.dict(by_alias=True)
        return json_str

    def report_progress_pre(self):
        return self.report_progress_pre_backupset(ProgressType.PRE)

    def report_progress_restore_sub(self):
        return self.report_progress(ProgressType.RESTORE_SUB)

    def report_progress_post(self):
        # 后置任务直接上报成功
        json_str = self.gen_success_progress()
        self.output_other_result(json_str)
        return True

    def report_progess_when_failed(self):
        json_str = self.gen_failed_progress()
        self.output_other_result(json_str)
        return True

    def report_progress_when_except(self):
        return self.report_progess_when_failed()

    def get_data_size(self):
        table_len_list = []
        data_len = 0
        ret, copy = self.get_copy()
        if not ret:
            return False, data_len
        extend_info = copy.get(DMJsonConstant.EXTENDINFO, {})
        if extend_info.get(DMJsonConstant.EXTENDINFO, {}):
            extend_info = extend_info.get(DMJsonConstant.EXTENDINFO, {})
        if extend_info:
            table_len_list = extend_info.get(DMJsonConstant.TABAL_SPACE_INFO, [])
        if not table_len_list:
            log.error(f"Get extendInfo failed. {self.get_log_comm()}.")
            return False, data_len
        if self._restore_type_key == DmRestoreType.DATABASE:
            for table_len in table_len_list:
                data_len = data_len + int(table_len.get(DMJsonConstant.TABLE_LEN, "0"))
        if self._restore_type_key == DmRestoreType.TABALSPACE:
            if not self.set_tablespace_name_list():
                return False, data_len
            for table_len in table_len_list:
                if table_len.get(DMJsonConstant.TABLE_NAME, "") in self._tablespace_name_list:
                    data_len += int(table_len.get(DMJsonConstant.TABLE_LEN, "0"))
            try:
                data_len = data_len / len(self._tablespace_name_list)
            except ZeroDivisionError:
                log.error(f"Tablespace name list is empty.{self.get_log_comm()}.")
                return False, 0
        if self._restore_type_key == DmRestoreType.TIMESTAMP:
            data_len = 0
            return True, data_len
        try:
            data_len = int(data_len / DMJsonConstant.B_TO_KB)
        except ZeroDivisionError as exception_str:
            log.error("Get data size fail.")
            return False, 0
        return True, data_len

    def set_tablespace_name_list(self):
        tablespace_name_list = self._json_param.get(DMJsonConstant.JOB,
                                                    {}).get(DMJsonConstant.RESTORESUBOBJECTS, [])
        self._tablespace_name_list = []
        # 只能有一个tablespace_name_list 结构体
        if len(tablespace_name_list) != 1:
            log.error(f'Tablespace name list err.{self.get_log_comm()}.')
            return False
        tablespace_name_info = tablespace_name_list[0].get(DMJsonConstant.EXTENDINFO, {}).get("name", '')
        if not tablespace_name_info:
            log.error(f'Get tablespace name list err.{self.get_log_comm()}.')
            return False
        self._tablespace_name_list = tablespace_name_info.split(';')
        return True

    def write_progress_file(self, task_status, progress, process_type):
        progress_str = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, taskStatus=task_status,
                                     progress=progress, logDetail=self.log_detail)
        json_str = progress_str.dict(by_alias=True)
        progress_file = os.path.join(self._cache_path, f"{self._job_id}_{self._sub_job_id}_{process_type}")
        log.info(f"Write file.{self.get_log_comm()}.")
        output_execution_result_ex(progress_file, json_str)

    def report_progress_pre_backupset(self, process_type):
        file_path = os.path.join(self._cache_path, f"{self._job_id}_{self._sub_job_id}_{process_type}")
        if os.access(file_path, os.F_OK):
            json_str = IniParses.read_param_file(file_path)
            del_file(file_path)
            self.output_other_result(json_str)
        else:

            ret, progress = read_cheak_backupset_progress(f"dmcheck_{self._job_id}")
            if not ret:
                task_status = SubJobStatusEnum.FAILED.value
            elif progress == DamengStrConstant.ONE_HUNDRED:
                task_status = SubJobStatusEnum.COMPLETED.value
            else:
                task_status = SubJobStatusEnum.RUNNING.value
            progress_str = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                                         taskStatus=task_status,
                                         progress=progress)
            json_str = progress_str.dict(by_alias=True)
            self.output_other_result(json_str)
        return True

    def report_progress(self, process_type):
        file_path = os.path.join(self._cache_path, f"{self._job_id}_{self._sub_job_id}_{process_type}")
        # 读取到进度文件就上报，读不到是进度文件可能还没生成，不返回失败，等框架下一次调度
        if os.access(file_path, os.F_OK):
            json_str = IniParses.read_param_file(file_path)
            self.output_other_result(json_str)
        return True

    def exec_pre_when_failed(self):
        self.write_progress_file(SubJobStatusEnum.FAILED.value, 100, ProgressType.PRE)
        return True

    def exec_pre_when_success(self):
        self.write_progress_file(SubJobStatusEnum.COMPLETED.value, 100, ProgressType.PRE)
        return True

    def exec_pre_when_except(self):
        return self.exec_pre_when_failed()

    def exec_sub_when_failed(self):
        self.write_progress_file(SubJobStatusEnum.FAILED.value, 100, ProgressType.RESTORE_SUB)
        return True

    def exec_sub_when_success(self):
        self.write_progress_file(SubJobStatusEnum.COMPLETED.value, 100, ProgressType.RESTORE_SUB)
        return True

    def exec_sub_when_except(self):
        return self.exec_sub_when_failed()

    def get_copy(self, is_log=False):
        if not self._json_param:
            log.error(f"Json param is none.{self.get_log_comm()}.")
            return False, ""
        copies = self._json_param.get(DMJsonConstant.JOB, {}).get(DMJsonConstant.COPIES, [])
        copy_num = len(copies)
        if copy_num <= 0:
            log.error(f"Copy count error.{self.get_log_comm()}.")
            return False, ""
        if not is_log and self._restore_type_key in [DmRestoreType.SCN, DmRestoreType.TIMESTAMP]:
            # 日志备份最后一个日志副本，倒数第二个才是所需的数据副本
            return True, copies[-2]
        else:
            # 根据dme的下发规则，最后一个副本是恢复的当前副本
            return True, copies[-1]

    def get_restore_mount_path(self, mount_type, is_log=False):
        """
        获取副本的挂载路径,所有副本的挂载路径都是一样的
        :return: 
        """
        ret, copy = self.get_copy(is_log)
        if not ret:
            return False, ""
        repositories = copy.get(DMJsonConstant.REPORITTORIES, [])
        if len(repositories) <= 0:
            log.error(f"Repositories count error.{self.get_log_comm()}.")
            return False, ""
        for repositorie in repositories:
            if repositorie.get(DMJsonConstant.REPORITORYTYPE, "") == mount_type:
                return True, repositorie.get(DMJsonConstant.PATH, [""])[0]
        log.error(f"Path dont found.{self.get_log_comm()}.")
        return False, ""

    def get_copy_extend_info(self):
        """
        获取副本的挂载路径,所有副本的挂载路径都是一样的
        :return: 
        """
        ret, copy = self.get_copy()
        if not ret:
            return False
        extend_info = copy.get(DMJsonConstant.EXTENDINFO, {})
        if extend_info.get(DMJsonConstant.EXTENDINFO, {}):
            extend_info = extend_info.get(DMJsonConstant.EXTENDINFO, {})
        if extend_info:
            self._db_name = extend_info.get(DMJsonConstant.DBNAME, "")
            self._backup_set_name = extend_info.get(DMJsonConstant.BACKUPSETNAME, "")
            self._backup_type = extend_info.get(DMJsonConstant.BACKUPTYPE, 0)
            self._copy_group_id_array = extend_info.get(DMJsonConstant.GROUPID, [])
            self._copy_group_id_array = sorted(self._copy_group_id_array)
        if not self._db_name or not self._backup_set_name or self._backup_type == 0:
            log.error(f"Get extendInfo failed.{self.get_log_comm()}.")
            return False
        if self._backup_type == BackupTypeEnum.DIFF_BACKUP.value or \
                self._backup_type == BackupTypeEnum.INCRE_BACKUP.value:
            self._base_backup_set_name = extend_info.get(DMJsonConstant.BASEBACKUPSETNAME, "")
            if not self._base_backup_set_name:
                log.error(f"Get baseBackupSetName failed.{self.get_log_comm()}.")
                return False
        return True

    def get_target_env(self):
        if not self._json_param:
            log.error(f"Json param is none.{self.get_log_comm()}.")
            return False, ""
        target_env = self._json_param.get(DMJsonConstant.JOB, {}).get(DMJsonConstant.TARGETENV, {})
        if not target_env:
            log.error(f"Get target env failed.{self.get_log_comm()}.")
            return False, ""
        return True, target_env

    def get_nodes(self):
        ret, target_env = self.get_target_env()
        if not ret:
            return False, []
        nodes = target_env.get(DMJsonConstant.EXTENDINFO, {}).get(DMJsonConstant.NODES, [])
        nodes = json.loads(nodes)
        if len(nodes) <= 0:
            log.error(f"Nodes count error.{self.get_log_comm()}.")
            return False, []
        return True, nodes

    def set_data_path(self):
        if self._data_path:
            return True
        ret, path = self.get_restore_mount_path(RepositoryDataTypeEnum.DATA_REPOSITORY.value)
        path = get_livemount_path(self._job_id, path)
        if ret:
            self._data_path = path
            log.info(f"Get data path success: {self._data_path}")
            return True
        log.error(f"Set data path failed.{self.get_log_comm()}.")
        return False

    def set_cache_path(self):
        if self._cache_path:
            return True
        ret, path = self.get_restore_mount_path(RepositoryDataTypeEnum.CACHE_REPOSITORY.value)
        if ret:
            self._cache_path = path
            return True
        log.error(f"Set cache path failed.{self.get_log_comm()}.")
        return False

    def set_log_path(self):
        if self._log_path:
            return True
        ret, path = self.get_restore_mount_path(RepositoryDataTypeEnum.LOG_REPOSITORY.value)
        path = get_livemount_path(self._job_id, path)
        if ret:
            self._log_path = path
            log.info(f"Get log path success: {self._log_path}.")
            return True
        log.error(f"Set log path failed.{self.get_log_comm()}.")
        return False

    def set_nodes(self):
        ret, nodes = self.get_nodes()
        if ret:
            self._nodes = nodes
            return True
        log.error(f"Set nodes path failed.{self.get_log_comm()}.")
        return False

    def restore_get_log_mount(self):
        # 日志仓每个都一样 取一个就行
        ret, log_path = self.get_restore_mount_path(RepositoryDataTypeEnum.LOG_REPOSITORY.value, True)
        if not ret:
            log.error(f"Get log repository failed.{self.get_log_comm()}.")
            return False
        path_info = log_path.split("/")
        self._log_path = "/".join(path_info[ArrayIndex.INDEX_FIRST_0:ArrayIndex.INDEX_LAST_1])
        if not self._log_path:
            log.error(f"Get log path failed.{self.get_log_comm()}.")
            return False
        return True

    def set_sub_type(self):
        self._sub_type = self._json_param.get(DMJsonConstant.JOB,
                                              {}).get(DMJsonConstant.TARGETOBJECT,
                                                      {}).get(DMJsonConstant.SUBTYPE, "")
        if not self._sub_type:
            log.error(f"Set subtype error.{self.get_log_comm()}.")
            return False
        return True

    def set_node_extendinfo(self, node_: dict):
        job_extend_info = self._json_param.get(DMJsonConstant.JOB,
                                               {}).get(DMJsonConstant.EXTENDINFO, {})
        target_location = job_extend_info.get(DMJsonConstant.TARGETLOCATION, "")
        if not target_location:
            log.error(f"Target location failed.{self.get_log_comm()}.")
            return False
        node_extendinfo = node_.get(DMJsonConstant.EXTENDINFO, {})
        if target_location == DMJsonConstant.NEW \
                and self._sub_type == BackupSubType.SINGLE_NODE:
            self._db_name = job_extend_info.get(DMJsonConstant.DBNAME, '')
            self._db_path = job_extend_info.get(DMJsonConstant.DBPATH, '')
            self._dmini_path = os.path.join(self._db_path, self._db_name, "dm.ini")
        else:
            self._db_name = node_extendinfo.get(DMJsonConstant.DBNAME, '')
            self._db_path = node_extendinfo.get(DMJsonConstant.DBPATH, '')
            self._dmini_path = node_extendinfo.get(DMJsonConstant.DMINIPATH, '')
        self._port = node_extendinfo.get(DMJsonConstant.PORT, '')
        if all([self._db_path, self._db_name, self._port, self._dmini_path]):
            return True
        log.error(f"Set node extendinfo fail.{self.get_log_comm()}.")
        return False

    def set_version(self):
        self._version = self._json_param.get(DMJsonConstant.JOB,
                                             {}).get(DMJsonConstant.EXTENDINFO,
                                                     {}).get(DMJsonConstant.VERSION, "")
        if not self._version:
            log.error(f"Get version from param fail.{self.get_log_comm()}.")
            return False
        return True

    def set_big_version(self):
        self.big_version = DamengSource().get_big_version()
        if not self.big_version:
            log.error(f"Get big version fail.{self.get_log_comm()}.")
            return False
        return True

    def set_rman_err_info(self, rman_out):
        err_code = self.get_rman_errcode(rman_out)
        log_detail = LogDetail(logInfoParam=[self._job_id],
                               logInfo=ReportDBLabel.RESTORE_SUB_FAILED,
                               logLevel=DBLogLevel.ERROR,
                               logDetail=ErrCode.ERR_EXECUTE_RMAN_FAIL,
                               logDetailParam=[err_code])
        self.log_detail = [log_detail]

    def get_rman_errcode(self, rman_out):
        """
        获取rman执行失败错误码
        :return: 错误码
        """
        re_rule = "\[-(.*?)]:"
        slot_list = re.findall(re_rule, rman_out)
        errcode = "0"
        if not slot_list:
            log.error(f"Get rman errcode fail.{self.get_log_comm()}.")
            return errcode
        errcode_info = slot_list[0]
        if errcode_info.isdigit():
            errcode = f'-{errcode_info}'
            return errcode
        log.error(f"Get rman errcode fail.{self.get_log_comm()}.")
        return errcode

    def report_tablespace_not_incomplete(self):
        log_detail = LogDetail(logInfoParam=[self._job_id],
                               logInfo='plugin_dameng_tablespace_restore_incomplete_label',
                               logLevel=DBLogLevel.WARN)
        output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=Progress.RUNNING,
                               taskStatus=SubJobStatusEnum.RUNNING.value, logDetail=[log_detail])
        try:
            invoke_rpc_tool_interface(self._job_id, RpcParamKey.REPORT_JOB_DETAILS, output.dict(by_alias=True))
        except Exception as e:
            log.error('report tablespace not incomplete failed')
