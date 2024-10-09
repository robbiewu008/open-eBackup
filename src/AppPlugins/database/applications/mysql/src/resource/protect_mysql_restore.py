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

import sys

from mysql import log
from common.common import exter_attack, output_result_file
from common.const import SubJobStatusEnum, JobData, RepositoryDataTypeEnum
from common.util.check_utils import is_valid_id
from mysql.src.common.constant import MySQLExecPower, MySQLJsonConstant, MySQLParamType, \
    RestoreSubJobType, MySQLCmdStr, RestoreType, RoleType, SubJobType, SubJobPolicy, SubTaskPriority, MySQLClusterType
from mysql.src.common.error_code import BodyErr, MySQLCode
from mysql.src.common.parse_parafile import ParseParaFile
from mysql.src.protect_eapp_mysql_instance_restore import EAppMysqlInstanceRestore
from mysql.src.protect_mysql_base import MysqlBase
from mysql.src.protect_mysql_db_restore import MysqlDatabaseRestore
from mysql.src.protect_mysql_instance_restore import MysqlInstanceRestore
from mysql.src.protect_mysql_undo_service import MysqlUndoService
from mysql.src.utils.mysql_utils import MysqlUtils


class MysqlRestoreComm(object):
    def __init__(self):
        self.aaa = None
        self._mysql_object = None
        self.cluster_type = ''

    @staticmethod
    def get_code(input_ret):
        if input_ret:
            return MySQLCode.SUCCESS.value
        else:
            return MySQLCode.FAILED.value

    @staticmethod
    def format_body_err(input_ret):
        tmp_body_err = 0
        if not input_ret:
            tmp_body_err = BodyErr.ERR_PLUGIN_CANNOT_BACKUP.value
        return tmp_body_err

    @staticmethod
    def build_sub_job(task_job_id, job_priority, exec_node_id, job_info, job_name):
        """
        填充子任务信息
        :return:
        """
        sub_job_info = dict()
        sub_job_info["jobId"] = task_job_id
        sub_job_info["subJobId"] = ""
        sub_job_info["jobType"] = SubJobType.BUSINESS_SUB_JOB.value
        sub_job_info["jobName"] = job_name
        sub_job_info["jobPriority"] = job_priority
        sub_job_info["policy"] = SubJobPolicy.FIXED_NODE.value
        sub_job_info["ignoreFailed"] = False
        sub_job_info["execNodeId"] = exec_node_id
        sub_job_info["jobInfo"] = job_info
        return sub_job_info

    @classmethod
    def get_instance(cls):
        if not hasattr(MysqlRestoreComm, '_instance'):
            MysqlRestoreComm._instance = MysqlRestoreComm()
        return MysqlRestoreComm._instance

    def create_mysql_restore_by_type(self, tmp_pid, tmp_job_id, tmp_sub_job_id):
        json_param = ParseParaFile.parse_para_file(tmp_pid)
        self.aaa = json_param
        job_json = json_param.get(MySQLJsonConstant.JOB, {})
        if not job_json:
            log.error("Get job json failed.")
            return False
        target_object_json = job_json.get(MySQLJsonConstant.TARGETOBJECT, {})
        if not target_object_json:
            log.error("Get target object json failed.")
            return False
        sub_type = target_object_json.get(MySQLJsonConstant.SUBTYPE, "")
        if not sub_type:
            log.error("Get sub type json failed.")
            return False
        if sub_type == MySQLParamType.INSTANCE or sub_type == MySQLParamType.CLUSTER:
            self.cluster_type = json_param.get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.TARGETOBJECT, {}).get(
                MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.CLUSTERTYPE, "")
            if self.cluster_type == MySQLClusterType.EAPP:
                self._mysql_object = EAppMysqlInstanceRestore(tmp_pid, tmp_job_id, tmp_sub_job_id, json_param)
            else:
                self._mysql_object = MysqlInstanceRestore(tmp_pid, tmp_job_id, tmp_sub_job_id, json_param)
        elif sub_type == MySQLParamType.DATABASE:
            self._mysql_object = MysqlDatabaseRestore(tmp_pid, tmp_job_id, tmp_sub_job_id, json_param)
        else:
            log.error(f"Restore type is wrong: {sub_type}")
            return False
        return True

    def create_mysql_restore_comm(self, tmp_pid, tmp_job_id, tmp_sub_job_id):
        self._mysql_object = MysqlBase(tmp_pid, tmp_job_id, tmp_sub_job_id, "")
        return True

    def write_action_result(self, tmp_code, tmp_body_err, tmp_message):
        self._mysql_object.output_action_result(tmp_code, tmp_body_err, tmp_message)

    @exter_attack
    def restore_prerequisite(self):
        tmp_ret = self._mysql_object.exec_restore_pre_job()
        task_job_id, task_sub_job_id = self._mysql_object.get_id_and_sub_id()
        if task_sub_job_id:
            progress_type = task_sub_job_id
        else:
            progress_type = task_job_id
        if tmp_ret:
            self._mysql_object.write_progress_file(SubJobStatusEnum.COMPLETED.value, 100, progress_type)
        else:
            self._mysql_object.write_progress_file(SubJobStatusEnum.FAILED.value, 100, progress_type)
        tmp_code = self.get_code(tmp_ret)
        return tmp_code, self.format_body_err(tmp_ret), ""

    @exter_attack
    def restore(self):
        tmp_ret, restore_type, log_restore = self._mysql_object.check_restore_type()
        if not tmp_ret:
            tmp_code = self.get_code(tmp_ret)
            return tmp_code, self.format_body_err(tmp_ret), ""

        sub_job_type = self._mysql_object.get_sub_job_type()
        if self.cluster_type == MySQLClusterType.EAPP:
            tmp_ret = self.restore_for_eapp(restore_type)
        else:
            if sub_job_type == RestoreSubJobType.RESTORE:
                log.debug(f"Restore sub type: {sub_job_type}")
                tmp_ret = self._mysql_object.exec_sub_job(restore_type, log_restore)
            elif sub_job_type == RestoreSubJobType.POST_RESTORE:
                log.debug(f"Restore sub type: {sub_job_type}")
                tmp_ret = self._mysql_object.exec_restore_post()
            else:
                tmp_ret = False

        task_job_id, task_sub_job_id = self._mysql_object.get_id_and_sub_id()
        if task_sub_job_id:
            progress_type = task_sub_job_id
        else:
            progress_type = task_job_id
        if tmp_ret:
            self._mysql_object.write_progress_file(SubJobStatusEnum.COMPLETED.value, 100, progress_type)
        else:
            self._mysql_object.write_progress_file(SubJobStatusEnum.FAILED.value, 100, progress_type)
        tmp_code = self.get_code(tmp_ret)
        return tmp_code, self.format_body_err(tmp_ret), ""

    def restore_for_eapp(self, restore_type):
        sub_job_type = self._mysql_object.get_sub_job_type()
        log.info(f"Restore sub type: {sub_job_type}")
        self._mysql_object.set_restore_all_param()
        self._mysql_object.set_cache_path()
        if sub_job_type == RestoreSubJobType.ALLOW_RESTORE:
            tmp_ret = self._mysql_object.allow_restore()
        elif sub_job_type == RestoreSubJobType.RESTORE:
            tmp_ret = self._mysql_object.exec_sub_job_for_eapp(restore_type)
        elif sub_job_type == RestoreSubJobType.RESTART_CLUSTER:
            tmp_ret = self._mysql_object.restart_cluster_for_eapp(restore_type)
        elif sub_job_type == RestoreSubJobType.STOP_SLAVE:
            tmp_ret = self._mysql_object.stop_slave_for_eapp()
        elif sub_job_type == RestoreSubJobType.SYNC_STATUS_CHECK:
            tmp_ret = self._mysql_object.check_sync_status()
        elif sub_job_type == RestoreSubJobType.COMPARE_MASTER:
            tmp_ret = self._mysql_object.compare_master()
        elif sub_job_type == RestoreSubJobType.RESET_SYNC:
            tmp_ret = self._mysql_object.reset_sync()
        elif sub_job_type == RestoreSubJobType.POST_RESTORE:
            tmp_ret = self._mysql_object.exec_restore_post()
        else:
            tmp_ret = False
        return tmp_ret

    @exter_attack
    def restore_post(self):
        task_job_id, task_sub_job_id = self._mysql_object.get_id_and_sub_id()
        if task_sub_job_id:
            progress_type = task_sub_job_id
        else:
            progress_type = task_job_id
        self._mysql_object.write_progress_file(SubJobStatusEnum.RUNNING.value, 5, progress_type)
        self._mysql_object.del_restore_cache_dir()
        # 清理可能存在的undo临时文件夹
        cache_path = self._mysql_object.get_mount_path(RepositoryDataTypeEnum.CACHE_REPOSITORY.value)
        MysqlUndoService.clean_undo_tmp_dir(task_job_id, cache_path)
        tmp_code = self.get_code(True)
        self._mysql_object.write_progress_file(SubJobStatusEnum.COMPLETED.value, 100, progress_type)
        return tmp_code, self.format_body_err(True), ""

    def get_nodes(self):
        task_job_id, task_sub_job_id = self._mysql_object.get_id_and_sub_id()
        json_param = self._mysql_object.get_json_param()
        nodes = json_param.get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.TARGETENV, {}).get(
            MySQLJsonConstant.NODES, [])
        if len(nodes) <= 0:
            log.error(f"Nodes count error. jobId:{task_job_id}")
            return False, [], 0
        return True, nodes, len(nodes)

    def gen_sub_job(self):
        """
        拆分子任务实现
        """
        task_job_id, task_sub_job_id = self._mysql_object.get_id_and_sub_id()
        json_param = self._mysql_object.get_json_param()
        # 判断恢复类型是否正确，即集群的数据库恢复才正确
        sub_type = json_param.get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.TARGETOBJECT, {}).get(
            MySQLJsonConstant.SUBTYPE, "")
        if sub_type not in [MySQLParamType.DATABASE, MySQLParamType.CLUSTER]:
            log.error(f"The instance type does not support subtask splitting. jobId:{task_job_id}")
            return []
        cluster_type = json_param.get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.TARGETOBJECT, {}).get(
            MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.CLUSTERTYPE, "")
        if not cluster_type or cluster_type == RestoreType.SINGLE_DB:
            log.error(f"The database type not cluster, so not support subtask splitting. jobId:{task_job_id}")
            return []
        # 获取节点参数，用于组装子任务
        tmp_ret, nodes, nodes_lens = self.get_nodes()
        if not tmp_ret:
            log.error(f"Get nodes failed, so not support subtask splitting. jobId:{task_job_id}")
            return []
        if cluster_type == MySQLClusterType.EAPP:
            # 创建server-id文件
            cache_path = self._mysql_object.get_mount_path(RepositoryDataTypeEnum.CACHE_REPOSITORY.value)
            MysqlUtils.create_server_id_file(cache_path)
            return self.gen_eapp_sub_job(nodes)
        node_index_priority = SubTaskPriority.STANDBY_NODE_INIT_PRIORITY.value
        sub_job_array = []
        for node in nodes:
            # 需要的参数，如：节点ID，优先级等。优先级设置：主节点恢复子任务为1，其余备节点依次递增，最后各节点同时做后置任务，也设为最大值
            host_id = node.get(MySQLJsonConstant.ID, "")
            role_type = node.get(MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.ROLE, "")
            if not role_type:
                log.error(f"Get role type failed, so not support subtask splitting. jobId:{task_job_id}")
                return []
            if role_type == RoleType.ACTIVE_NODE:
                job_priority = SubTaskPriority.ACTIVE_NODE_INI_PRIORITY.value
            else:
                job_priority = node_index_priority
                node_index_priority = node_index_priority + 1
            job_info = ""
            job_name = RestoreSubJobType.RESTORE
            sub_job = self.build_sub_job(task_job_id, job_priority, host_id, job_info, job_name)
            sub_job_array.append(sub_job)
            job_name = RestoreSubJobType.POST_RESTORE
            job_priority = nodes_lens + SubTaskPriority.MAX_PRIORITY_PLUS_VALUE.value
            sub_job = self.build_sub_job(task_job_id, job_priority, host_id, job_info, job_name)
            sub_job_array.append(sub_job)
        log.debug(f"Gen sub job success. jobId:{task_job_id}")
        return sub_job_array

    def gen_eapp_sub_job(self, nodes):
        sub_job_array = []
        task_job_id, task_sub_job_id = self._mysql_object.get_id_and_sub_id()
        for node in nodes:
            host_id = node.get(MySQLJsonConstant.ID, "")
            sub_job_array.append(self.build_sub_job(task_job_id, 1, host_id, "", RestoreSubJobType.ALLOW_RESTORE))
            sub_job_array.append(self.build_sub_job(task_job_id, 2, host_id, "", RestoreSubJobType.RESTORE))
            sub_job_array.append(self.build_sub_job(task_job_id, 3, host_id, "", RestoreSubJobType.RESTART_CLUSTER))
            sub_job_array.append(self.build_sub_job(task_job_id, 4, host_id, "", RestoreSubJobType.STOP_SLAVE))
            sub_job_array.append(self.build_sub_job(task_job_id, 5, host_id, "", RestoreSubJobType.SYNC_STATUS_CHECK))
            sub_job_array.append(self.build_sub_job(task_job_id, 6, host_id, "", RestoreSubJobType.COMPARE_MASTER))
            sub_job_array.append(self.build_sub_job(task_job_id, 7, host_id, "", RestoreSubJobType.RESET_SYNC))
            sub_job_array.append(self.build_sub_job(task_job_id, 8, host_id, "", RestoreSubJobType.POST_RESTORE))
        return sub_job_array

    @exter_attack
    def call_gen_sub_job(self):
        sub_job_array = self.gen_sub_job()
        p_id = self._mysql_object.get_p_id()
        try:
            output_result_file(p_id, sub_job_array)
        except Exception as exceptionstr:
            log.error(f"Write file fail.")
            return False, False, ""
        return True, True, ""

    @exter_attack
    def query_job_permission(self):
        json_str = {
            "user": MySQLExecPower.MYSQL_USER,
            "fileMode": MySQLExecPower.MYSQL_FILE_MODE
        }
        self._mysql_object.output_other_result(json_str)
        return MySQLCode.SUCCESS.value, True, ""

    def report_progress_abort(self):
        tmp_ret = self._mysql_object.report_progress_abort()
        tmp_code = self.get_code(tmp_ret)
        return tmp_code, self.format_body_err(tmp_ret), ""

    @exter_attack
    def report_progress_comm(self):
        tmp_ret = self._mysql_object.report_restore_progress_comm()
        tmp_code = self.get_code(tmp_ret)
        return tmp_code, self.format_body_err(tmp_ret), ""

    def abort_job(self):
        self._mysql_object.abort_restore_job()
        return MySQLCode.SUCCESS.value, self.format_body_err(True), ""

    def write_action_result_when_failed(self, tmp_code, tmp_body_err, tmp_message):
        if tmp_code == MySQLCode.FAILED.value:
            self._mysql_object.output_action_result(tmp_code, tmp_body_err, tmp_message)

    def execution_exception(self, cmd_param, ob_id_param, sub_job_id_param):
        self._mysql_object.cmd_execution_exception(cmd_param, ob_id_param, sub_job_id_param)


if __name__ == '__main__':
    args = sys.argv[1:]
    log.info(f"call backup script paramCnt:{len(args)} args:{args}")
    if len(args) < 2:
        log.error(f"No enough parameters, param cnt:{len(args)}")
        sys.exit()
    cmd = args[0]
    pid = args[1]
    if not is_valid_id(pid):
        log.warn(f'req_id is invalid')
        sys.exit(1)
    job_id = ""
    sub_job_id = ""
    if len(args) > 2:
        job_id = args[2]
        if not is_valid_id(job_id):
            log.warn(f'job_id is invalid')
            sys.exit(1)
    if len(args) > 3:
        sub_job_id = args[3]
        if not is_valid_id(sub_job_id):
            log.warn(f'sub_id is invalid')
            sys.exit(1)

    JobData.CMD = cmd
    mysql_restore = MysqlRestoreComm.get_instance()
    CMD_DICT = {
        MySQLCmdStr.RESTORE_PRE: [
            mysql_restore.create_mysql_restore_by_type,
            mysql_restore.restore_prerequisite,
            mysql_restore.write_action_result
        ],
        MySQLCmdStr.RESTORE: [
            mysql_restore.create_mysql_restore_by_type,
            mysql_restore.restore,
            mysql_restore.write_action_result
        ],
        MySQLCmdStr.RESTORE_POST: [
            mysql_restore.create_mysql_restore_by_type,
            mysql_restore.restore_post,
            mysql_restore.write_action_result
        ],
        MySQLCmdStr.RESTORE_GEN_SUB: [
            mysql_restore.create_mysql_restore_by_type,
            mysql_restore.call_gen_sub_job
        ],
        MySQLCmdStr.QUERY_PERMMISSION: [
            mysql_restore.create_mysql_restore_comm,
            mysql_restore.query_job_permission,
            mysql_restore.write_action_result_when_failed
        ],
        MySQLCmdStr.PROGRESS_ABORT: [
            mysql_restore.create_mysql_restore_by_type,
            mysql_restore.report_progress_abort,
            mysql_restore.write_action_result_when_failed
        ],
        MySQLCmdStr.PROGRESS_COMM: [
            mysql_restore.create_mysql_restore_by_type,
            mysql_restore.report_progress_comm
        ],
        MySQLCmdStr.ABORT_JOB: [
            mysql_restore.create_mysql_restore_by_type,
            mysql_restore.abort_job,
            mysql_restore.write_action_result
        ],
        MySQLCmdStr.PAUSE_JOB: [
            mysql_restore.create_mysql_restore_by_type,
            mysql_restore.abort_job,
            mysql_restore.write_action_result
        ]
    }
    func_array = CMD_DICT.get(cmd)

    try:
        if not func_array:
            MysqlInstanceRestore.output_action_result_ex(pid, MySQLCode.FAILED.value,
                                                         BodyErr.ERROR_COMMON_INVALID_PARAMETER.value,
                                                         f"Cmd not found. pid:{pid} jobId:{job_id}")
        elif len(func_array) >= 1:
            ret = func_array[0](pid, job_id, sub_job_id)
            if ret and len(func_array) >= 2:
                log.info(f"Get cmd: {cmd}")
                code, body_err, message = func_array[1]()
                if len(func_array) >= 3:
                    func_array[2](code, body_err, message)
            else:
                MysqlInstanceRestore.output_action_result_ex(pid, MySQLCode.FAILED.value,
                                                             BodyErr.ERROR_COMMON_INVALID_PARAMETER.value,
                                                             f"Cmd Param error. pid:{pid} jobId:{job_id}")
        else:
            MysqlInstanceRestore.output_action_result_ex(pid, MySQLCode.FAILED.value,
                                                         BodyErr.ERROR_COMMON_INVALID_PARAMETER.value,
                                                         f"Cmd nothing to do. pid:{pid} jobId:{job_id}")
    except Exception as exception_str:
        mysql_restore.execution_exception(cmd, job_id, sub_job_id)
        log.exception(f"Exec cmd failed.errMsg:raise exception  pid:{pid} jobId:{job_id}")
