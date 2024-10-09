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

import datetime
import json
import os.path
import threading
import time
import pwd

from common.common import output_result_file, output_execution_result_ex, execute_cmd, execute_cmd_list, \
    execute_cmd_with_expect

from common.common_models import SubJobDetails, LogDetail, SubJobModel
from common.const import ParamConstant, SubJobPolicyEnum, SubJobPriorityEnum, CMDResult, RestoreTypeEnum, \
    RepositoryDataTypeEnum
from common.const import SubJobStatusEnum, DBLogLevel
from common.util.checkout_user_utils import get_path_owner
from tidb.common.const import ErrorCode, SubJobType, TiDBTaskType, ClusterRequiredHost
from tidb.common.tidb_param import JsonParam
from tidb.common.tidb_common import exec_mysql_sql, get_tidb_structure, report_job_details, get_env_variable, \
    get_status_up_role_one_host, check_roles_up, get_cluster_user, check_params_valid, query_local_business_ip
from tidb.handle.restore.restore_common import write_progress_file, write_file_append
from tidb.logger import log


class TiDBConst:
    # 数据库常量
    ALL_HOSTS = "%"
    ALL_PRIVILEGE = "ALL PRIVILEGES"
    DROP = "DROP"
    SELECT = "SELECT"
    BR_VERSION = "Release Version"
    LOG_PATH = "storage"
    LOG_STATUS = "status"
    LOG_NORMAL = "NORMAL"
    LOG_START_TIME = "log-min-ts"
    LOG_END_TIME = "log-max-ts"
    CHECK_POINT = "checkpoint[global]"
    BACKUP_META = "backupmeta"
    DATABASE_EXIST = 'ErrDatabasesAlreadyExisted'
    PD_DOWN = "pd down"
    TIDB_DOWN = "tidb down"
    DROP_DB_FAILED = "Drop databases on target cluster failed."
    LOG_TASK_EXIST = "Log task exist"
    TIKV_DOWN = "tikv down"
    TIFLASH_DOWN = "tiflash down"
    USER_ID_CHECK_FAILED = "userid check failed"
    CONFLICT_TABLES = "exist conflict tables"
    ERROR_START = "Error:"


class TiDBDataBaseFilter:
    # 集群下默认数据库，不需备份的数据库
    INFORMATION_SCHEMA = "INFORMATION_SCHEMA"
    METRICS_SCHEMA = "METRICS_SCHEMA"
    PERFORMANCE_SCHEMA = "PERFORMANCE_SCHEMA"
    MYSQL = "mysql"
    TIDB_BR_TEMP = "__TiDB_BR_Temporary_mysql"


def get_sub_job_name(param):
    """
    获取sub job执行函数名称
    :param param:
    :return:
    """
    sub_name = param.get("subJob", {}).get("jobName", "")
    log.info(f"get_sub_job_name: {sub_name}")
    if sub_name not in (
            TidbRestoreSubJobName.EXEC_CHECK, TidbRestoreSubJobName.EXEC_DROP, TidbRestoreSubJobName.EXEC_RESTORE,
            TidbRestoreSubJobName.EXEC_PRE, TidbRestoreSubJobName.EXEC_LOG_MOUNT,
            TidbRestoreSubJobName.EXEC_LOG_DROP, TidbRestoreSubJobName.EXEC_LOG_RESTORE):
        log.error(f"{sub_name} not found in sub jobs!")
        return ""
    return sub_name


class TidbRestoreSubJobName:
    EXEC_CHECK = "exec_check"
    EXEC_PRE = "exec_pre"
    EXEC_DROP = "exec_drop"
    EXEC_RESTORE = "exec_restore"
    EXEC_LOG_CHECK = "exec_log_check"
    EXEC_LOG_MOUNT = "exec_log_mount"
    EXEC_LOG_DROP = "exec_log_drop"
    EXEC_LOG_RESTORE = "exec_log_restore"


class TidbName:
    CLUSTER = "TiDB-cluster"
    DATABASE = "TiDB-database"
    TABLE = "TiDB-table"


class IfDropTables:
    DROP_TABLES = "1"


DROP_DICT = {
    TidbName.DATABASE: "DATABASE",
    TidbName.TABLE: "TABLE"
}
RESTORE_DICT = {
    TidbName.CLUSTER: "full",
    TidbName.DATABASE: "db",
    TidbName.TABLE: "table"
}


class Restore:
    def __init__(self, pid, job_id, sub_job_id, data, json_param):
        if not json_param:
            log.error("Parse params obj is null.")
            raise Exception("Parse params obj is null.")
        log.debug(f"json_param{json_param}")
        self._std_in = data
        self._pid = pid
        self._job_id = job_id
        self._sub_job_id = sub_job_id
        self._json_param_object = json_param
        self._logdetail = None
        self._err_info = {}
        self._query_progress_interval = 15
        self._cache_area = JsonParam.get_cache_path(json_param)
        self._meta_path = JsonParam.get_meta_path(json_param)
        self._data_path = JsonParam.get_data_path(json_param)

        self._job_status = SubJobStatusEnum.RUNNING

        self._copy_id = self._json_param_object.get("job", {}).get("copies", [{}, {}])[0].get("id", "")
        self._host_ip = self._json_param_object.get("job", {}).get("targetEnv", {}).get("endpoint", "")
        self._nodes = self._json_param_object.get("job", {}).get("targetEnv", {}).get("nodes", "")
        self._instance_id = self._json_param_object.get("job", {}).get("targetObject", {}).get("id", "")
        self.drop_sub_job_name = "sub_job_drop"
        self.restore_sub_job_name = "sub_job_exec"
        self.restore_gran = self._json_param_object.get("job", {}).get("targetObject", {}).get("subType", "")

        self._target_obj_extend_info = self._json_param_object.get("job", {}).get("targetObject", {}).get(
            "extendInfo", {})
        self._cluster_info_str = self._target_obj_extend_info.get("clusterInfoList", "")
        self._cluster_structure = get_tidb_structure(self._cluster_info_str)
        log.info(f"self._cluster_structure: {self._cluster_structure}")
        self._cluster_name = self._target_obj_extend_info.get("clusterName", "")
        log.info(f"self._cluster_name {self._cluster_name}")
        self.tiup_uuid = self._target_obj_extend_info.get("tiupUuid")
        self._tiup_path = self._target_obj_extend_info.get("tiupPath", '/root/.tiup/bin/tiup')
        self.restore_type = self._json_param_object.get("job", {}).get("copies", [{}, {}])[-1].get("type", "")
        log.info(f"self.restore_type: {self.restore_type}")
        self.copies = self._json_param_object.get("job", {}).get("copies", [])

    @staticmethod
    def get_repository_paths(file_content, repository_type):
        if repository_type == RepositoryDataTypeEnum.LOG_REPOSITORY:
            repositories = file_content.get("job", {}).get("copies", [])[1].get("repositories", [])
        elif repository_type == RepositoryDataTypeEnum.DATA_REPOSITORY:
            repositories = file_content.get("job", {}).get("copies", [])[0].get("repositories", [])
        repositories_paths = []
        for repository in repositories:
            if repository['repositoryType'] == repository_type:
                repositories_paths.append(repository["path"][0])
                log.info(repository)
        return repositories_paths

    @staticmethod
    def merge_log(log_path_list):
        log_path_target = log_path_list[0]
        log.info(f"step 5-5 merge_log log_path_target is {log_path_target}")
        for log_path in log_path_list:
            ret = Restore.execute_merge_log(log_path, log_path_target)
            if not ret:
                log.error("step 5-5 merge_log execute_merge_log failed!")
                return False
        # 把文件backup.lock、backupmeta替换为最新的
        if len(log_path_list) > 1:
            ret = Restore.cp_log_files(log_path_list[-1], log_path_target)
            if not ret:
                log.error("step 5-5 merge_log cp log files failed!")
                return False
        return True

    @staticmethod
    def execute_merge_log(log_path, log_path_target):
        for dirs in os.listdir(log_path):
            dir_path = os.path.join(log_path, dirs)
            # 将日志仓中，所有日志备份产生的副本合并到v1文件夹中
            if os.path.isdir(dir_path) and dirs != 'v1':
                log.info(f"step 5-5 merge_log  dir_path: {dir_path}")
                os.chdir(dir_path)
                merge_cmd = ["find . -type f", f"xargs -i /bin/cp --parents -pr {{}} {log_path_target}"]
                log.info(f"merge_cmd: {merge_cmd}")
                ret_code, std_out, std_err = execute_cmd_list(merge_cmd)
                if ret_code != CMDResult.SUCCESS.value:
                    log.error(f"step 5-5 Exec merge command failed!")
                    log.info(f"step 5-5 Exec merge out_info: {std_out}{std_err}")
                    return False
        return True

    @staticmethod
    def cp_log_files(cp_file_src, cp_file_target):
        log.info(f"cp_log_files cp_file_src: {cp_file_src}, cp_file_target : {cp_file_target}")
        os.chdir(cp_file_src)
        cp_cmd = ["find . -maxdepth 1 -type f", f"xargs -i /bin/cp --parents -pr {{}} {cp_file_target}"]
        log.info(f"cp_log_files cp_cmd: {cp_cmd}")
        ret_code, std_out, std_err = execute_cmd_list(cp_cmd)
        if ret_code != CMDResult.SUCCESS.value:
            log.error(f"Exec cp_cmd command failed!")
            log.info(f"cp_log_files out_info: {std_out}{std_err}")
            return False
        return True

    def get_log_comm(self):
        return f"pid:{self._pid} jobId:{self._job_id} subjobId:{self._sub_job_id}"

    def get_progress(self):
        log.info('Query restore progress!')
        status = SubJobStatusEnum.RUNNING
        progress = 0
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}")
        log.info(f'get_progress progress_file {progress_file}')
        if not os.path.exists(progress_file):
            status = SubJobStatusEnum.FAILED
            log.error(f"Progress file: {progress_file} not exist")
            return status, progress
        log.info(f'Path exist')
        with open(progress_file, "r", encoding='UTF-8') as file_stream:
            data = file_stream.read()
            log.info(f"data: {data}")
        if 'FAILED' in data:
            status = SubJobStatusEnum.FAILED
            progress = 100
            # 可设置错误码
            return status, progress
        if 'SUCCEED' in data:
            status = SubJobStatusEnum.COMPLETED
            progress = 100
        else:
            progress = 0
        log.info(f'Progress is {progress}')
        return status, progress

    def set_logdetail_with_params(self, log_label, sub_job_id, err_code=None, log_detail_param=None,
                                  log_level=DBLogLevel.INFO.value):
        err_dict = LogDetail(logInfo=log_label,
                             logInfoParam=[sub_job_id],
                             logTimestamp=int(time.time()),
                             logDetail=err_code,
                             logDetailParam=log_detail_param,
                             logLevel=log_level)
        self._logdetail = []
        self._logdetail.append(err_dict)
        return True

    def upload_restore_progress(self):
        while self._job_status == SubJobStatusEnum.RUNNING:
            log.info("Start to report progress.")
            task_status, progress = self.get_progress()
            progress_dict = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                                          taskStatus=SubJobStatusEnum.RUNNING,
                                          progress=0, logDetail=self._logdetail)
            if task_status == SubJobStatusEnum.FAILED:
                self.set_logdetail_with_params("plugin_restore_subjob_fail_label", self._sub_job_id,
                                               0, [], DBLogLevel.ERROR.value)
            progress_dict.task_status = task_status
            progress_dict.progress = progress
            self._job_status = task_status
            report_job_details(self._job_id, progress_dict.dict(by_alias=True))
            time.sleep(self._query_progress_interval)

    def restore_prerequisite_progress(self):
        """
        执行前置任务：空
        @return:
        """
        log.info("start step-2 restore_prerequisite_progress")
        pre_job_status = SubJobStatusEnum.COMPLETED.value
        output = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id,
                               taskStatus=pre_job_status, progress=100, logDetail=self._logdetail)
        output_result_file(self._pid, output.dict(by_alias=True))
        return True

    def build_sub_job(self, job_priority, job_type, job_name, node_id, job_info):
        return SubJobModel(jobId=self._job_id, jobType=SubJobType.BUSINESS_SUB_JOB.value, execNodeId=node_id,
                           jobPriority=job_priority, jobName=job_name, policy=job_type, jobInfo=job_info,
                           ignoreFailed=False).dict(by_alias=True)

    def gen_sub_job(self):
        if self.restore_type == "log":
            return self.gen_sub_job_log()
        else:
            return self.gen_sub_job_full()

    def gen_sub_job_full(self):
        log.info("start step-3 exec_gen_sub_job")
        """
        执行生成子任务
        @return:
        """
        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self._pid}")
        sub_job_array = []

        log.info("gen_sub_job sub_job_01")
        job_type = SubJobPolicyEnum.FIXED_NODE.value
        job_name = TidbRestoreSubJobName.EXEC_CHECK
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_1
        node_id = self.tiup_uuid
        sub_job = self.build_sub_job(job_priority, job_type, job_name, node_id, None)
        sub_job_array.append(sub_job)

        log.info("gen_sub_job sub_job_02")
        restore_roles = (ClusterRequiredHost.TIKV, ClusterRequiredHost.TIFLASH)
        cluster_info = json.loads(self._cluster_info_str)
        for node in cluster_info:
            if node.get("role", "") not in restore_roles:
                continue
            job_type = SubJobPolicyEnum.FIXED_NODE.value
            job_name = TidbRestoreSubJobName.EXEC_PRE
            job_priority = SubJobPriorityEnum.JOB_PRIORITY_2
            node_id = node.get("hostManagerResourceUuid", "")
            sub_job = self.build_sub_job(job_priority, job_type, job_name, node_id, None)
            sub_job_array.append(sub_job)

        log.info("gen_sub_job sub_job_03")
        job_type = SubJobPolicyEnum.FIXED_NODE.value
        job_name = TidbRestoreSubJobName.EXEC_DROP
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_3
        node_id = self.tiup_uuid
        sub_job = self.build_sub_job(job_priority, job_type, job_name, node_id, None)
        sub_job_array.append(sub_job)

        log.info("gen_sub_job sub_job_04")
        job_type = SubJobPolicyEnum.FIXED_NODE.value
        job_name = TidbRestoreSubJobName.EXEC_RESTORE
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_4
        node_id = self.tiup_uuid
        sub_job = self.build_sub_job(job_priority, job_type, job_name, node_id, None)
        sub_job_array.append(sub_job)
        log.info(f"Sub-job splitting succeeded.sub-job :{sub_job_array}")
        output_execution_result_ex(file_path, sub_job_array)
        return True

    def report_before_sub_job(self, job_info):

        log_detail = LogDetail(logInfo="agent_start_execute_sub_task_success_label",
                               logInfoParam=[job_info, self._sub_job_id],
                               logLevel=DBLogLevel.INFO.value)
        report_job_details(self._pid,
                           SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                         logDetail=[log_detail], taskStatus=SubJobStatusEnum.RUNNING.value).dict(
                               by_alias=True))

    def report_error_result(self, progress_file):
        write_progress_file('FAILED', progress_file)
        with open(progress_file, "r", encoding='UTF-8') as file_stream:
            data = file_stream.read()

        restore_error_info = {
            TiDBConst.PD_DOWN: "All pd hosts down. Please check target cluster status.",
            TiDBConst.TIDB_DOWN: "All tidb hosts down. Please check target cluster status.",
            TiDBConst.TIKV_DOWN: "Tikv host down. Please check target cluster status.",
            TiDBConst.TIFLASH_DOWN: "Tiflash host down. Please check target cluster status.",
            TiDBConst.DROP_DB_FAILED: TiDBConst.DROP_DB_FAILED,
            TiDBConst.LOG_TASK_EXIST:
                "Log task exists on target cluster! Stop log task and try again.",
            TiDBConst.DATABASE_EXIST: "[BR:Restore:ErrDatabasesAlreadyExisted] "
                                      "databases already existed in restored cluster.",
            TiDBConst.USER_ID_CHECK_FAILED: "Uid check failed. Please make sure target cluster Deploy user has "
                                            "the same uid on all tikv and tiflash hosts.",
            TiDBConst.CONFLICT_TABLES: "Conflicts tables exist in the target cluster."
        }

        message = ""
        for error_label, error_message in restore_error_info.items():
            if error_label in data:
                message = error_message
                break
        if not message:
            message = data[5:]
            tmp_message = message[:-7]
            if not tmp_message.isspace():
                message = tmp_message

        log.info(f"data: {data}")
        log.info(f"message: {message}")
        log_detail = LogDetail(logInfo="plugin_restore_subjob_fail_label", logInfoParam=[self._sub_job_id],
                               logLevel=DBLogLevel.ERROR.value, logDetail=ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL,
                               logDetailParam=["Restore", message])

        return log_detail

    def restore_task(self):
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}")
        log.info(f"restore_task progress_file {progress_file}")
        try:
            write_progress_file('START', progress_file)
        except Exception as ex:
            log.error(f"write progress file failed {ex}")
            return False
        sub_job_dict = {
            TidbRestoreSubJobName.EXEC_CHECK: self.sub_job_exec_check,
            TidbRestoreSubJobName.EXEC_DROP: self.sub_job_exec_drop,
            TidbRestoreSubJobName.EXEC_RESTORE: self.sub_job_exec_restore,
            TidbRestoreSubJobName.EXEC_PRE: self.sub_job_exec_pre,
            TidbRestoreSubJobName.EXEC_LOG_MOUNT: self.sub_job_exec_log_mount,
            TidbRestoreSubJobName.EXEC_LOG_DROP: self.sub_job_exec_log_drop,
            TidbRestoreSubJobName.EXEC_LOG_RESTORE: self.sub_job_exec_log_restore,
        }
        progress_thread = threading.Thread(name='exec_restore', target=self.upload_restore_progress)
        progress_thread.daemon = True
        progress_thread.start()
        job_info = query_local_business_ip(self._nodes)
        log.debug(f"job_info: {job_info}")
        self.report_before_sub_job(job_info)
        # 执行子任务
        sub_job_name = get_sub_job_name(self._json_param_object)
        log.info(f"get sub job name {sub_job_name}")
        if not sub_job_name:
            return False
        log.info(f"Exec sub job {sub_job_name} begin.{self.get_log_comm()}.")
        try:
            ret = sub_job_dict.get(sub_job_name)()
        except Exception as ex:
            log.error(f"Exec sub job {sub_job_name} failed. {ex} {self.get_log_comm()}.")
            ret = False
        if not ret:
            log.error(f"Exec sub job {sub_job_name} failed.{self.get_log_comm()}.")
            log_detail = self.report_error_result(progress_file)
            report_job_details(self._pid,
                               SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                             logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value).dict(
                                   by_alias=True))
            progress_thread.join()
            os.remove(progress_file)
            return False
        write_progress_file('SUCCEED', progress_file)
        log_detail = LogDetail(logInfo="plugin_task_subjob_success_label", logInfoParam=[self._sub_job_id], logLevel=1)
        log.info(f"Exec sub job {sub_job_name} success.{self.get_log_comm()}.")
        report_job_details(self._pid, SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=100,
                                                    logDetail=[log_detail],
                                                    taskStatus=SubJobStatusEnum.COMPLETED.value).dict(by_alias=True))
        progress_thread.join()
        return True

    def sub_job_exec_check(self):
        log.info(f"start step-4-1  sub job check {self._job_id} {self._sub_job_id}")
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}")

        # 记录集群用户名
        user = get_cluster_user(self._cluster_name, self._tiup_path)
        user_path = os.path.join(self._cache_area, f"userid_{self._job_id}")
        userid_path = os.path.join(self._cache_area, f"userid_{self._job_id}", "uid")
        if not os.path.exists(userid_path):
            os.makedirs(userid_path)
        user_file = os.path.join(user_path, "user")
        write_file_append(f"{user}", user_file)

        # 校验目标集群状态正常
        if self.restore_type == "log":
            # 日志恢复不要求tidb节点在线
            ret_host = check_roles_up(self._cluster_name, self._tiup_path,
                                      [ClusterRequiredHost.PD, ClusterRequiredHost.TIKV, ClusterRequiredHost.TIFLASH])
        else:
            ret_host = check_roles_up(self._cluster_name, self._tiup_path,
                                      [ClusterRequiredHost.PD, ClusterRequiredHost.TIDB, ClusterRequiredHost.TIKV,
                                       ClusterRequiredHost.TIFLASH])
        down_role = ret_host.get("down")
        if down_role:
            write_progress_file(f"{down_role} down", progress_file)
            return False
        # 校验目标集群上日志备份是否关闭
        pd_id = self.get_pd_id()
        log_task_name = self.get_log_task_name(pd_id)
        if log_task_name:
            write_progress_file(TiDBConst.LOG_TASK_EXIST, progress_file)
            log.error("Log task exist!")
            return False
        return True

    def drop_table(self, tidb_id, meta_path, table_info_file):
        ip_port = tidb_id.split(":")
        ip = ip_port[0]
        port = int(ip_port[1])
        table_info_path = os.path.join(meta_path, table_info_file)
        log.info(f"table_info_path {table_info_path}")
        with open(table_info_path, "r", encoding='UTF-8') as f_content:
            table_info = json.loads(f_content.read())
        log.info(f"table_info {table_info}")
        for table_list in table_info:
            db_name = table_list.get("db", "")
            restore_db_name = self._target_obj_extend_info.get("databaseName", "")
            if restore_db_name and db_name != restore_db_name:
                continue
            table_list = table_list.get("tables", [])
            if not db_name:
                log.error("Failed get DB.")
                return False
            if not table_list:
                log.error("Failed get table list.")
                return False
            table_name = self._target_obj_extend_info.get("tableName", "")
            if not table_name:
                table_name = ",".join(table_list)
            if not check_params_valid(db_name, table_name):
                log.error(f"The db_name {db_name} or table_name {table_name} verification fails")
                return False
            drop_table_cmd = [f"use {db_name};", f"DROP TABLE IF EXISTS {table_name};"]
            ret, output = exec_mysql_sql(TiDBTaskType.RESTORE, self._pid, drop_table_cmd, ip, port)
            if not ret:
                log.error(f"Delete table {table_name} in db {db_name} Failed: {output}")
                return False
            log.debug(f"success drop table {table_name}")
        log.info(f"{self._sub_job_id} Succeed delete tables!")
        return True

    def drop_table_granularity(self, tidb_id, sub_objects):
        ip_port = tidb_id.split(":")
        ip = ip_port[0]
        port = int(ip_port[1])
        for obj in sub_objects:
            target_temp_table_name = obj.get("name", "")
            if not target_temp_table_name:
                log.error(f"The table name is empty in subObjects({sub_objects}).")
                return False
            sub_object = json.loads(target_temp_table_name.replace("'", "\""))
            log.info(sub_object)
            # 取表全路径
            table_path = sub_object["name"]
            # 解析 cluster / database / table这种格式的表名，从中提取出集群cluster名称，和表名(database.table格式)
            database_name, table_name = self.parse_single_database_and_table_name(table_path)
            if not database_name or not table_name:
                return False
            if not check_params_valid(database_name, table_name):
                log.error(f"The database_name {database_name} or table_name {table_name} verification fails")
                return False
            drop_table_cmd = [f"use {database_name};", f"DROP TABLE IF EXISTS {table_name};"]
            ret, output = exec_mysql_sql(TiDBTaskType.RESTORE, self._pid, drop_table_cmd, ip, port)
            if not ret:
                log.error(f"Delete table {table_name} in db {database_name} Failed: {output}")
                return False
            log.debug(f"success drop table {table_name}")
        log.info(f"{self._sub_job_id} Succeed delete granularity tables!")
        return True

    def get_tables_from_sub_objects(self, sub_objects):
        log.debug(sub_objects)
        tables_from_backup = {}
        for obj in sub_objects:
            target_temp_table_name = obj.get("name", "")
            if not target_temp_table_name:
                log.error(f"The table name is empty in subObjects({sub_objects}).")
                return {}
            sub_object = json.loads(target_temp_table_name.replace("'", "\""))
            log.debug(sub_object)
            # 取表全路径
            table_path = sub_object["name"]
            # 解析 cluster / database / table这种格式的表名，从中提取出集群cluster名称，和表名(database.table格式)
            database_name, table_name = self.parse_single_database_and_table_name(table_path)
            if database_name in tables_from_backup:
                tables_pre = tables_from_backup.get(database_name, [])
                tables_pre.append(table_name)
                try:
                    tables_from_backup[database_name] = tables_pre
                except Exception as err:
                    log.error(f"Dic update error{err}.")
            else:
                tables_from_backup[database_name] = [table_name]
        return tables_from_backup

    def get_tables_from_backup(self, restore_full_flag, table_info_file, sub_objects):
        # 获取副本中的表
        tables_from_backup = {}
        if restore_full_flag:
            # 从cache仓记录文件获取
            table_info_path = os.path.join(self._meta_path, table_info_file)
            log.info(f"table_info_path {table_info_path}")
            with open(table_info_path, "r", encoding='UTF-8') as f_content:
                table_info = json.loads(f_content.read())
            for table_list in table_info:
                database_name = table_list.get("db", "")
                table_list = table_list.get("tables", [])
                tables_from_backup[database_name] = table_list
        else:
            # 从pm下发参数获取
            tables_from_backup = self.get_tables_from_sub_objects(sub_objects)
        return tables_from_backup

    def get_conflict_tables(self, tidb_id, tables_from_backup):
        # 查询是否存在表冲突
        ip_port = tidb_id.split(":")
        ip = ip_port[0]
        port = int(ip_port[1])
        conflict_db_tables = {}
        for key, value in tables_from_backup.items():
            log.info(f"db: {key}, tb: {value}")
            # 副本中，该库没有表
            if not value:
                continue
            # 查询目标集群是否存在库
            list_cmd = [f'SHOW DATABASES LIKE "{key}";']
            ret, output = exec_mysql_sql(TiDBTaskType.RESTORE, self._pid, list_cmd, ip, port)
            if not ret:
                log.error("Exec SQL failed!")
            # 目标集群没有该库
            if not output:
                continue
            # 查询目标集群中库下的表
            list_cmd = [f"use {key};", "show tables;"]
            ret, output = exec_mysql_sql(TiDBTaskType.RESTORE, self._pid, list_cmd, ip, port)
            if not ret:
                log.error(f"Use database {key} failed!")
                continue
            table_list = []
            for item in output:
                table_list.append(str(item[0]))
            log.info(f"table_list: {table_list}")
            # 目标集群中，该库没有表
            if not table_list:
                continue
            conflict_tables = []
            for table in value:
                if table in table_list:
                    conflict_tables.append(table)
            # 无冲突的表
            if not conflict_tables:
                continue
            conflict_db_tables[key] = conflict_tables
        return conflict_db_tables

    def report_conflict_tables(self, conflict_db_tables):
        # 上报冲突的表
        conflict_tables_str = ""
        for key, value in conflict_db_tables.items():
            if conflict_tables_str:
                conflict_tables_str += "；\n"
            log.debug(f"db: {key}, tb: {value}")
            tables = "，".join(value)
            conflict_tables_str += "【" + key + "】: { " + tables + " }"
        conflict_tables_str_pre = "\n【数据库名】: {表名}\n"
        conflict_tables_str = conflict_tables_str_pre + conflict_tables_str
        log.debug(conflict_tables_str)

        log_detail = LogDetail(logInfo="agent_tidb_exist_conflicting_tables_label",
                               logInfoParam=[conflict_tables_str], logLevel=2)
        report_job_details(self._pid, SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=20,
                                                    logDetail=[log_detail],
                                                    taskStatus=SubJobStatusEnum.RUNNING.value).dict(by_alias=True))

    def report_delete_tables(self):
        # 上报删除冲突的表
        log_detail = LogDetail(logInfo="agent_tidb_delete_conflicting_tables_label",
                               logInfoParam=[self._sub_job_id], logLevel=2)
        report_job_details(self._pid, SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=20,
                                                    logDetail=[log_detail],
                                                    taskStatus=SubJobStatusEnum.RUNNING.value).dict(by_alias=True))

    def delete_tables(self, db_tables, tidb_id):
        ip_port = tidb_id.split(":")
        ip = ip_port[0]
        port = int(ip_port[1])
        for key, value in db_tables.items():
            db_name = key
            table_name = ",".join(value)
            if not check_params_valid(db_name, table_name):
                log.error(f"The db_name {db_name} or table_name {table_name} verification fails")
                continue
            drop_table_cmd = [f"use {db_name};", f"DROP TABLE IF EXISTS {table_name};"]
            ret, output = exec_mysql_sql(TiDBTaskType.RESTORE, self._pid, drop_table_cmd, ip, port)
            if not ret:
                log.error(f"Delete table {table_name} in db {db_name} Failed: {output}")

    def sub_job_exec_drop(self):
        log.info(f"start step-4-2 sub_job_exec_drop {self._job_id} {self._sub_job_id}")
        # 校验集群用户uid一致
        log.info("start step-4-2-1 check uid.")
        ret_uid = self.check_uid()
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}")
        if not ret_uid:
            write_progress_file(TiDBConst.USER_ID_CHECK_FAILED, progress_file)
            return False

        tidb_id = self.get_tidb_id()
        if not tidb_id:
            write_progress_file(TiDBConst.TIDB_DOWN, progress_file)
            return False
        log.debug(f"tidb_id: {tidb_id}")
        table_info_file = "tidb_bkp_info"
        restore_full_flag, sub_objects = self.is_restore_full()
        log.info(f"sub job exec drop full table flag: {restore_full_flag}")
        log.info(f"sub job exec drop sub_objects: {sub_objects}")

        # 获取备份副本中的表
        log.info("start step-4-2-2 get tables from backup copy.")
        tables_from_backup = self.get_tables_from_backup(restore_full_flag, table_info_file, sub_objects)
        if not tables_from_backup:
            log.info("No conflict tables.")
            return True
        # 获取目标集群与备份副本中冲突的表
        log.info("start step-4-2-3 get conflict tables.")
        conflict_db_tables = self.get_conflict_tables(tidb_id, tables_from_backup)
        if not conflict_db_tables:
            log.info("No conflict tables.")
            return True
        # 上报冲突的表
        log.info("start step-4-2-4 report conflict tables.")
        self.report_conflict_tables(conflict_db_tables)
        # 获取是否删除表的标记
        if_drop_table = self._json_param_object.get("job", {}).get("extendInfo", {}).get("shouldDeleteTable", "")
        if if_drop_table == IfDropTables.DROP_TABLES:
            # 上报删除表，删表后继续任务
            log.info("start step-4-2-5-1 report delete conflict tables.")
            self.report_delete_tables()
            log.info("start step-4-2-5-2  delete conflict tables.")
            self.delete_tables(conflict_db_tables, tidb_id)
            return True
        else:
            # 上报任务失败
            log.info("start step-4-2-5-1 conflict tables exist, restore failed.")
            progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}")
            write_progress_file(TiDBConst.CONFLICT_TABLES, progress_file)
            return False

    def drop_tables(self, restore_full_flag, sub_objects, table_info_file, tidb_id):
        if restore_full_flag:
            # 全量删表
            self.drop_table(tidb_id, self._meta_path, table_info_file)
        else:
            # 细粒度删表
            self.drop_table_granularity(tidb_id, sub_objects)

    def get_pd_id(self):
        pd_id = get_status_up_role_one_host(self._cluster_name, self._tiup_path, "pd")
        return pd_id

    def get_tidb_id(self):
        tidb_id = get_status_up_role_one_host(self._cluster_name, self._tiup_path, "tidb")
        return tidb_id

    def sub_job_exec_restore(self):
        log.info(f"start step-4-3 sub_job_exec_restore {self._job_id} {self._sub_job_id}")
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}")
        # 删除备份、恢复失败后可能生成的临时库
        tidb_id = self.get_tidb_id()
        self.drop_dbs(tidb_id, [TiDBDataBaseFilter.TIDB_BR_TEMP])

        # 在tiup节点执行恢复命令
        # 根据下发参数补齐

        restore_granularity = RESTORE_DICT.get(self.restore_gran)  # full/db/table/point
        restore_full_flag, sub_objects = self.is_restore_full()
        # 细粒度列表restoreSubObjects不为空, 则为细粒度恢复
        if not restore_full_flag:
            restore_granularity = "table"
        pd_id = self.get_pd_id()
        if not pd_id:
            write_progress_file(TiDBConst.PD_DOWN, progress_file)
            log.error("All pd hosts down!")
            return False

        restore_tidb_log_file = f"{self._cache_area}/restore_log.txt"
        restore_path = self.get_restore_path()
        if restore_granularity == "table":
            restore_cmd = self.build_table_restore_request(pd_id, restore_path)
        else:
            restore_cmd = f"{self._tiup_path} br restore {restore_granularity} --pd {pd_id} " \
                          f"--storage {restore_path} --log-file {restore_tidb_log_file}"
        restore_request_version_suffix = f" --check-requirements=true"
        restore_cmd += restore_request_version_suffix
        restore_cmd = f"su - {get_path_owner(self._tiup_path)} -c '{restore_cmd}'"
        log.debug(f"Restore_cmd: {restore_cmd}")
        return_code, out_info, std_err = execute_cmd_with_expect(restore_cmd, "", None)
        log.info(f"return_code:{return_code} out_info:{out_info}")
        if return_code != 0:
            log.error(f"Exec restore cmd failed! {std_err}")
            if TiDBConst.ERROR_START in out_info:
                error_msg = out_info.split(TiDBConst.ERROR_START)[1]
                write_progress_file(error_msg, progress_file)
            return False
        os.remove(restore_tidb_log_file)
        return True

    def build_table_restore_request(self, pd_id, restore_path):
        """
        用于细粒度恢复:获取要恢复命令
        返回值: 恢复命令
        """
        restore_request = ''
        restore_tidb_log_file = f"{self._cache_area}/restore_log.txt"
        restore_request_prefix = f"{self._tiup_path} br restore full --pd {pd_id}"
        restore_request += restore_request_prefix
        tables = self.get_restore_tables_from_sub_objects()
        for table in tables:
            restore_request += ' --filter ' + table
        restore_request_suffix = f" --storage {restore_path} --log-file {restore_tidb_log_file}"
        restore_request += restore_request_suffix
        log.debug(f"restore_request: {restore_request}")
        return restore_request

    def get_restore_path(self):
        """
        用于细粒度恢复:获取数据目录
        返回值: 数据目录，格式tidb_{copy_id}
        """
        copy_id = self.get_copy_id()
        restore_path = os.path.join(self._data_path, f"tidb_{copy_id}")
        if not os.path.exists(restore_path):
            log.error(f"copy info path not exist.")
            return ''
        log.info(f"get restore path success {restore_path}")
        return restore_path

    def get_restore_tables_from_sub_objects(self):
        """
        用于细粒度恢复:获取要恢复的目标表列表
        返回值: 表列表
        """
        restore_full_flag, sub_objects = self.is_restore_full()
        # 全量恢复，返回空数组
        if restore_full_flag:
            return []
        table_list_result = []
        for obj in sub_objects:
            target_temp_table_name = obj.get("name", "")
            if not target_temp_table_name:
                log.error(f"The table name is empty in subObjects({sub_objects}).")
                return []
            sub_object = json.loads(target_temp_table_name.replace("'", "\""))
            # 取表全路径
            table_path = sub_object["name"]
            # 解析 cluster / database / table这种格式的表名，从中提取出集群cluster名称，和表名(database.table格式)
            target_database, table_name = self.parse_single_cluster_and_table_name(table_path)
            if not target_database or not table_name:
                return []
            table_list_result.append(table_name)
        return table_list_result

    def parse_single_cluster_and_table_name(self, table_name):
        """
        主要用于解析cluster/database/table这种格式的表名，从中提取出集群cluster名称，和表名(database.table格式)
        返回值: 集群名称，库表名称
        """
        splited_list = table_name.split("/")
        if not splited_list or len(splited_list) != 3:
            log.error(f"The table({table_name}) is illegal, jobid:{self._job_id}")
            return "", []
        return splited_list[0], f"{splited_list[1]}.{splited_list[2]}"

    def parse_single_database_and_table_name(self, resource_path):
        """
        主要用于解析cluster/database/table这种格式的表名，从中提取出集群cluster名称，和表名(database.table格式)
        返回值: 库名称，表名称
        """
        target_cluster, resource_name = self.parse_single_cluster_and_table_name(resource_path)
        database_name, table_name = resource_name.split('.')
        return database_name, table_name

    def is_restore_full(self):
        """
        判断当前恢复类型是否是细粒度恢复
        返回值: 是否为细粒度恢复，恢复资源列表
        """
        sub_objects = self._json_param_object.get("job", {}).get("restoreSubObjects", {})
        # restoreSubObjects为空数组，此次备份为全量恢复
        if not sub_objects or len(sub_objects) == 0:
            log.error(f"Fail to get fine grained restore tables.")
            return True, []
        return False, sub_objects

    def get_copy_id(self):
        """
        用于获取要恢复的副本ID
        返回值: 全量/增量/日志副本ID, 归档副本对应的全备副本ID
        """
        job_info = self._json_param_object.get("job")
        if not job_info:
            log.error(f"No job info in param file, main task: {self._job_id}, pid: {self._pid}")
            return False
        copies_info = job_info.get("copies")
        if not copies_info or len(copies_info) == 0:
            log.error(f"No copy info in param file, main task: {self._job_id}, pid: {self._pid}")
            return False
        cur_restore_copy = copies_info[len(copies_info) - 1]
        # 获取恢复副本类型
        restore_copy_type = cur_restore_copy.get("type")
        log.info(f"get restore_copy_type: {restore_copy_type}")
        # 对象归档副本、磁带归档副本类型，从job/copies/extendInfo/extendInfo/copy_id取此次归档副本对应的全备副本ID
        if restore_copy_type == "s3Archive" or restore_copy_type == "tapeArchive":
            copy_id = cur_restore_copy.get("extendInfo", {}).get("extendInfo", {}).get("copy_id", "")
        else:
            copy_id = cur_restore_copy.get("extendInfo", {}).get("copy_id", "")
        log.info(f"get copy_id successfully {copy_id}.")
        return copy_id

    def sub_job_exec_pre(self):
        log.info(f"start step-4-1 mount sub job {self._job_id} {self._sub_job_id}")
        # 获取集群用户名
        user_file = os.path.join(self._cache_area, f"userid_{self._job_id}", "user")
        with open(user_file, "r", encoding='UTF-8') as file_stream:
            user = file_stream.read().strip()
        # 记录集群用户的id
        try:
            uid = pwd.getpwnam(user).pw_uid
            log.info(uid)
        except Exception as err:
            log.error(f"Failed get uid {self._sub_job_id}")
            return False
        userid_path = os.path.join(self._cache_area, f"userid_{self._job_id}", "uid")
        uid_file = os.path.join(userid_path, f"{self._sub_job_id}")
        write_file_append(f"{uid}", uid_file)
        return True

    def check_uid(self):
        userid_path = os.path.join(self._cache_area, f"userid_{self._job_id}", "uid")
        uids = set()
        for file in os.listdir(userid_path):
            file_path = os.path.join(userid_path, file)
            with open(file_path, "r", encoding='UTF-8') as file_stream:
                user_id = file_stream.read()
                uids.add(user_id)
        if len(uids) > 1:
            log.error(f"Uid check failed! {uids}")
        log.info(f"Uid check success!")
        return True

    def get_log_task_name(self, pd_id):
        get_log_task_name_cmd = f"{self._tiup_path} br log status --pd {pd_id} "
        get_log_task_name_cmd = f"su - {get_path_owner(self._tiup_path)} -c '{get_log_task_name_cmd}'"
        log.debug(f"Get log task cmd: {get_log_task_name_cmd}")
        return_code, out_info, err_info = execute_cmd(get_log_task_name_cmd)
        log.info(f"return_code:{return_code} out_info:{out_info} err_info:{err_info}")
        task_name = ""
        if "name: " in out_info:
            name_idx = out_info.index("name:") + len("name: ")
            name_stop_idx = name_idx
            for name_stop_idx in range(name_idx, len(out_info)):
                if out_info[name_stop_idx] == " ":
                    break
                name_stop_idx += 1
            task_name = out_info[name_idx:name_stop_idx - 1]
            log.debug(f"task_name:{task_name}")
            return task_name
        if return_code != CMDResult.SUCCESS:
            log.error(f"The execute start crontab cmd failed!")
            log.info(f"out_info: {out_info} err_info: {err_info}")
            return ""
        return task_name

    def stop_log_task(self, pd_id, log_task_name):
        stop_log_task_cmd = f"{self._tiup_path} br log stop --task-name={log_task_name} --pd {pd_id} "
        stop_log_task_cmd = f"su - {get_path_owner(self._tiup_path)} -c '{stop_log_task_cmd}'"
        log.debug(f"Stop log task cmd: {stop_log_task_cmd}")
        return_code, out_info, err_info = execute_cmd(stop_log_task_cmd)
        log.info(f"return_code:{return_code} out_info:{out_info} err_info:{err_info}")
        if return_code != CMDResult.SUCCESS:
            log.error(f"The execute start crontab cmd failed!")
            log.info(f"out_info: {out_info} err_info: {err_info}")
            return False
        return True

    def get_restore_database(self):
        db_name = ""
        table_info_path = os.path.join(self._meta_path, "tidb_bkp_info")
        log.info(f"table_info_path {table_info_path}")
        with open(table_info_path, "r", encoding='UTF-8') as f_content:
            table_info = json.loads(f_content.read())
        log.info(f"table_info {table_info}")
        for table_list in table_info:
            db_name = table_list.get("db", "")
        return db_name

    def get_restore_table(self):
        table_name = ""
        table_info_path = os.path.join(self._meta_path, "tidb_bkp_info")
        log.info(f"table_info_path {table_info_path}")
        with open(table_info_path, "r", encoding='UTF-8') as f_content:
            table_info = json.loads(f_content.read())
        log.info(f"table_info {table_info}")
        for table_list in table_info:
            table_name = table_list.get("tables", "")[0]
        return table_name

    def gen_sub_job_log(self):
        log.info("start step-3 exec_gen_sub_job_log")
        """
        执行生成子任务
        @return:
        """
        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self._pid}")
        sub_job_array = []

        log.info("gen_sub_job_log sub_job_01")
        job_type = SubJobPolicyEnum.FIXED_NODE.value
        job_name = TidbRestoreSubJobName.EXEC_CHECK
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_1
        node_id = self.tiup_uuid
        sub_job = self.build_sub_job(job_priority, job_type, job_name, node_id, None)
        sub_job_array.append(sub_job)

        log.info("gen_sub_job_log sub_job_02")
        log_restore_roles = (ClusterRequiredHost.TIKV, ClusterRequiredHost.TIFLASH)
        cluster_info = json.loads(self._cluster_info_str)
        for node in cluster_info:
            if node.get("role", "") not in log_restore_roles:
                continue
            job_type = SubJobPolicyEnum.FIXED_NODE.value
            job_name = TidbRestoreSubJobName.EXEC_LOG_MOUNT
            job_priority = SubJobPriorityEnum.JOB_PRIORITY_2
            node_id = node.get("hostManagerResourceUuid", "")
            sub_job = self.build_sub_job(job_priority, job_type, job_name, node_id, None)
            sub_job_array.append(sub_job)

        log.info("gen_sub_job_log sub_job_03")
        job_type = SubJobPolicyEnum.FIXED_NODE.value
        job_name = TidbRestoreSubJobName.EXEC_LOG_DROP
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_3
        node_id = self.tiup_uuid
        sub_job = self.build_sub_job(job_priority, job_type, job_name, node_id, None)
        sub_job_array.append(sub_job)

        log.info("gen_sub_job_log sub_job_04")
        job_type = SubJobPolicyEnum.FIXED_NODE.value
        job_name = TidbRestoreSubJobName.EXEC_LOG_RESTORE
        job_priority = SubJobPriorityEnum.JOB_PRIORITY_4
        node_id = self.tiup_uuid
        sub_job = self.build_sub_job(job_priority, job_type, job_name, node_id, None)
        sub_job_array.append(sub_job)
        log.info(f"Sub-job-log splitting succeeded.sub-job :{sub_job_array}")
        output_execution_result_ex(file_path, sub_job_array)
        return True

    def sub_job_exec_log_mount(self):
        log.info(f"start step-4-2  sub job mount log {self._job_id} {self._sub_job_id}")
        # 获取集群用户名
        user_file = os.path.join(self._cache_area, f"userid_{self._job_id}", "user")
        with open(user_file, "r", encoding='UTF-8') as file_stream:
            user = file_stream.read().strip()
        # 记录集群用户的id
        try:
            uid = pwd.getpwnam(user).pw_uid
            log.info(uid)
        except Exception as err:
            log.error(f"Failed get uid {self._sub_job_id}")
            return False
        userid_path = os.path.join(self._cache_area, f"userid_{self._job_id}", "uid")
        uid_file = os.path.join(userid_path, f"{self._sub_job_id}")
        write_file_append(f"{uid}", uid_file)
        return True

    def get_dbs(self):
        get_db_fail_warn = False
        tidb_id = self.get_tidb_id()
        if not tidb_id:
            log_detail = LogDetail(logInfo="agent_tidb_failed_fetch_databases_on_target_cluster_label",
                                   logInfoParam=[self._sub_job_id], logLevel=2)
            report_job_details(self._pid, SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=20,
                                                        logDetail=[log_detail],
                                                        taskStatus=SubJobStatusEnum.RUNNING.value).dict(
                by_alias=True))
            return []
        ip_port = tidb_id.split(":")
        ip = ip_port[0]
        port = int(ip_port[1])
        list_cmd = [f"show databases;"]
        ret, output = exec_mysql_sql(TiDBTaskType.RESTORE, self._pid, list_cmd, ip, port)
        if not ret:
            log.error("Get database list failed!")
            # 上报获取数据库失败警告，不影响任务继续运行
            log_detail = LogDetail(logInfo="agent_tidb_failed_fetch_databases_on_target_cluster_label",
                                   logInfoParam=[self._sub_job_id], logLevel=2)
            report_job_details(self._pid, SubJobDetails(taskId=self._job_id, subTaskId=self._sub_job_id, progress=20,
                                                        logDetail=[log_detail],
                                                        taskStatus=SubJobStatusEnum.RUNNING.value).dict(
                by_alias=True))
            return []
        db_list = []
        db_filter = {
            TiDBDataBaseFilter.MYSQL, TiDBDataBaseFilter.METRICS_SCHEMA, TiDBDataBaseFilter.INFORMATION_SCHEMA,
            TiDBDataBaseFilter.PERFORMANCE_SCHEMA
        }
        for item in output:
            if not str(item[0]) in db_filter:
                db_list.append(str(item[0]))
        return db_list

    def drop_dbs(self, tidb_id, db_list):
        if not db_list:
            return True
        if not check_params_valid(*db_list):
            log.error(f"The db_list {db_list} verification fails")
            return False
        if not tidb_id:
            log.error("No tidb_id!")
            return False
        ip_port = tidb_id.split(":")
        ip = ip_port[0]
        port = int(ip_port[1])
        drop_db_cmd = []
        for db in db_list:
            drop_db_cmd.append(f"DROP DATABASE IF EXISTS {db};")
        log.info(drop_db_cmd)
        ret, output = exec_mysql_sql(TiDBTaskType.RESTORE, self._pid, drop_db_cmd, ip, port)
        if not ret:
            log.error("Drop database list failed!")
            return False
        return True

    def sub_job_exec_log_drop(self):
        log.info("start step-4-2 sub job log drop")

        log.info("Start to check uid.")
        ret_uid = self.check_uid()
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}")
        if not ret_uid:
            write_progress_file(TiDBConst.USER_ID_CHECK_FAILED, progress_file)
            return False

        # 删除目标集群中所有数据库
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}")

        log.info("start step-4-2 sub job drop dbs")
        db_list = self.get_dbs()
        tidb_id = self.get_tidb_id()
        ret = self.drop_dbs(tidb_id, db_list)
        if not ret:
            write_progress_file(TiDBConst.DROP_DB_FAILED, progress_file)
            return False
        return True

    def sub_job_exec_log_restore(self):
        log.info("start step-4-4 sub job log restore")
        progress_file = os.path.join(self._cache_area, f"progress_{self._sub_job_id}")

        # 合并日志备份
        data_path_log = self.get_repository_paths(self._json_param_object, RepositoryDataTypeEnum.DATA_REPOSITORY)
        log_path_list = self.get_repository_paths(self._json_param_object, RepositoryDataTypeEnum.LOG_REPOSITORY)
        log.info(f"step-4-4 data_path_log： {data_path_log}, log_path_list： {log_path_list}")
        ret = Restore.merge_log(log_path_list)
        if not ret:
            log.error("Merge log failed!")
            return False

        # 组装恢复参数
        pd_id = self.get_pd_id()
        if not pd_id:
            write_progress_file(TiDBConst.PD_DOWN, progress_file)
            return False
        log_backup_path = log_path_list[0]
        data_backup_path = data_path_log[0]

        files_log = os.listdir(log_backup_path)
        data_path_files = os.listdir(data_backup_path)
        if TiDBConst.BACKUP_META not in files_log:
            log_backup_path = os.path.join(log_backup_path, files_log[0])
        for file in data_path_files:
            if self._copy_id in file:
                data_backup_path = os.path.join(data_backup_path, file)
        restored_timestamp = self._json_param_object.get("job", {}).get("extendInfo", {}).get("restoreTimestamp", "")
        log.info(restored_timestamp)

        if not restored_timestamp:
            log.info("No restored_ts")
            log_restore_cmd = f"{self._tiup_path} br restore point --pd={pd_id} --storage={log_backup_path}" \
                              f" --full-backup-storage={data_backup_path} --check-requirements=true"
        else:
            datetime_str = datetime.datetime.fromtimestamp(int(restored_timestamp))
            restored_ts = datetime_str.astimezone(datetime.timezone.utc).strftime('%Y-%m-%d %H:%M:%S%z')
            restored_ts = f"\"{restored_ts}\""
            log.info(f"datetime_str:{datetime_str}, restored_ts: {restored_ts}.")

            log_restore_cmd = f"{self._tiup_path} br restore point --pd={pd_id} --storage={log_backup_path}" \
                              f" --full-backup-storage={data_backup_path} " \
                              f" --check-requirements=true --restored-ts {restored_ts}"
        log_restore_cmd = f"su - {get_path_owner(self._tiup_path)} -c '{log_restore_cmd}'"
        return_code, out_info, std_err = execute_cmd_with_expect(log_restore_cmd, "", None)
        log.info(f"return_code: {return_code} out_info:{out_info}")
        if return_code != 0:
            log.error(f"Exec PITR cmd command failed! std_err:{std_err}.")
            # 集群不为空，导致恢复失败场景
            if TiDBConst.DATABASE_EXIST in out_info:
                write_progress_file(TiDBConst.DATABASE_EXIST, progress_file)
            elif TiDBConst.ERROR_START in out_info:
                error_msg = out_info.split(TiDBConst.ERROR_START)[1]
                write_progress_file(error_msg, progress_file)
            # 删除恢复失败后可能生成的临时库
            tidb_id = self.get_tidb_id()
            self.drop_dbs(tidb_id, [TiDBDataBaseFilter.TIDB_BR_TEMP])
            return False
        return True
