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
import os
import time
from concurrent.futures import ThreadPoolExecutor

from goldendb.handle.resource.resource_info import GoldenDBResourceInfo
from goldendb.logger import log
from common.file_common import get_user_info
from common.common import output_result_file, invoke_rpc_tool_interface, execute_cmd_list, output_execution_result_ex, \
    read_tmp_json_file
from common.common_models import SubJobDetails, LogDetail
from common.const import SubJobPriorityEnum, SubJobStatusEnum, RepositoryDataTypeEnum, RpcParamKey, CMDResult, \
    CopyDataTypeEnum, ReportDBLabel
from common.util.exec_utils import exec_cat_cmd, exec_cp_dir_no_user, su_exec_rm_cmd
from common.util.validators import ValidatorEnum
from goldendb.handle.backup.parse_backup_params import get_goldendb_structure, report_job_details, write_file, \
    check_goldendb_structure
from goldendb.handle.common.const import SubJobName, MasterSlavePolicy, Report, LogLevel, GoldenDBJsonConst, \
    ExecutePolicy, GoldenDBNodeType, ErrorCode, LastCopyType
from goldendb.handle.common.goldendb_common import get_repository_path, su_exec_cmd, verify_path_trustlist, \
    get_backup_path, mkdir_chmod_chown_dir_recursively
from goldendb.handle.common.goldendb_param import JsonParam
from goldendb.schemas.glodendb_schemas import SubJob
from openGauss.common.common import safe_get_dir_size


class GoldenDBLogBackupToolService:
    def __init__(self, file_content=None, job_id=None, req_id=None, sub_id=None):
        self._cluster_structure = get_goldendb_structure(file_content)
        self._file_content = file_content
        self._job_id = job_id
        self._req_id = req_id
        self._sub_id = sub_id
        # job_info默认1个空格， 不然解包失败
        self._role_name, self._node_type = file_content.get("subJob", {}).get("jobInfo", " ").split(" ")
        self._copy_id = file_content.get("job", {}).get("copy", [])[0].get("id", "")
        self._sub_job_name = ""
        self._cache_data_path = get_repository_path(file_content, RepositoryDataTypeEnum.CACHE_REPOSITORY)
        if not verify_path_trustlist(self._cache_data_path):
            log.error(f"Invalid path: {self._cache_data_path}, {self.get_log_comm()}")
            raise Exception(f'job id: {job_id}, invalid path: {self._cache_data_path}.')
        self._log_data_path = get_repository_path(file_content, RepositoryDataTypeEnum.LOG_REPOSITORY)
        if not verify_path_trustlist(self._log_data_path):
            log.error(f"Invalid path: {self._log_data_path}, {self.get_log_comm()}")
            raise Exception(f'job id: {job_id}, invalid path: {self._log_data_path}.')

        self._sub_job_dict = {
            SubJobName.EXEC_LOG_BACKUP: self.execute_log_backup,
            SubJobName.EXEC_COPY_BINLOG: self.exec_copy_log_backup_result_job,
            SubJobName.EXEC_REPORT_DATA_SIZE: self.exec_report_data_size_job
        }

    def gen_sub_jobs(self):
        """
        生成日志备份子任务
        1、在管理节点执行日志备份命令
        2、将日志备份结果拷贝到log仓
        """
        response = []
        # 1：首先要在管理节点执行日志备份
        # 当前搭建环境只有单个管理节点
        sub_job = self.__create_sub_job(self._cluster_structure.manager_nodes[0],
                                        SubJobName.EXEC_LOG_BACKUP,
                                        SubJobPriorityEnum.JOB_PRIORITY_1)
        response.append(sub_job)

        # 2：执行拷贝任务
        # 执行节点添加管理节点首节点，当前搭建环境只有单个管理节点
        exec_nodes = [self._cluster_structure.manager_nodes[0]]
        # 添加GTM所有节点
        exec_nodes.extend(self._cluster_structure.gtm_nodes)
        # 添加数据节点
        for group in self._cluster_structure.data_nodes.values():
            exec_nodes.extend(group[MasterSlavePolicy.MASTER])
            if check_goldendb_structure(MasterSlavePolicy.SLAVE, self._cluster_structure):
                exec_nodes.extend(group[MasterSlavePolicy.SLAVE])
        log.debug(f"exec copy nodes:{json.dumps(exec_nodes)}, {self.get_log_comm()}")

        for exec_node in exec_nodes:
            copy_job = self.__create_sub_job(exec_node, SubJobName.EXEC_COPY_BINLOG, SubJobPriorityEnum.JOB_PRIORITY_2)
            response.append(copy_job)

        # 3：生成上报需要的data_size和speed
        sub_job = self.__create_sub_job(self._cluster_structure.manager_nodes[0],
                                        SubJobName.EXEC_REPORT_DATA_SIZE,
                                        SubJobPriorityEnum.JOB_PRIORITY_3)
        response.append(sub_job)

        # 如果没有子任务，则报错
        if not response:
            log.error(f'generate zero sub job{self.get_log_comm()}')
            raise Exception(f'job id: {self._job_id}, generate zero sub job')
        # 加入查询信息子任务，不添加的话不会调用queryCopy方法，不会上报给UBC
        response.append(
            SubJob(jobId=self._job_id, policy=ExecutePolicy.ANY_NODE, jobName='queryCopy',
                   jobPriority=SubJobPriorityEnum.JOB_PRIORITY_4).dict(by_alias=True))
        log.info(f'step 4: finish to execute backup_gen_sub_job, {self.get_log_comm()}')
        output_result_file(self._req_id, response)

    def backup(self):
        """
        执行备份子任务
        """
        # 上报子任务开始
        self.__report_running()

        # 执行子任务
        sub_job_name = JsonParam.get_sub_job_name(self._file_content)
        log.info(f'sub_job_name: {sub_job_name}, {self.get_log_comm()}')
        if not sub_job_name:
            log.error(f"sub_job_name is empty")
            return
        self._sub_job_name = sub_job_name

        try:
            sub_job_ret = self._sub_job_dict.get(sub_job_name)()
            if not sub_job_ret:
                self.__report_error()
            else:
                log.info(f'step 5-2: finish to execute copy log backup result job, {self.get_log_comm()}')
                self.__report_success()
        except Exception as err:
            log.error(f"do {sub_job_name} fail: {err}, {self.get_log_comm()}")
            self.__report_error()

    def execute_log_backup(self):
        """
        执行日志备份子任务
        """
        log.info(f'step 5-1: start to execute log backup, {self.get_log_comm()}')
        self.__sub_job_pre_binlog()
        # 获取集群版本
        version = GoldenDBResourceInfo.get_cluster_version(self._role_name)
        version_file = os.path.join(self._cache_data_path, f"bkp_{self._job_id}_version.json")
        output_execution_result_ex(version_file, {"version": version})
        pool = ThreadPoolExecutor(max_workers=1, thread_name_prefix='goldendb-log-backup')

        copy_feature = pool.submit(self.__exec_log_backup_cmd)
        while not copy_feature.done():
            time.sleep(Report.REPORT_INTERVAL)
            log.info(f'step 5-1: log backup cmd is running, {self.get_log_comm()}')
        if not copy_feature.result()[2]:
            return False
        log.info(f'step 5-1:finish to execute log backup cmd, {self.get_log_comm()}')
        return True

    def exec_copy_log_backup_result_job(self):
        """
        执行拷贝备份结果子任务
        """
        log.info(f'step 5-2: start to execute copy log backup result job， node type:{self._node_type}, '
                 f'{self.get_log_comm()}')

        if GoldenDBNodeType.ZX_MANAGER_NODE == self._node_type:
            ret = self.__copy_from_manager_node()
        elif GoldenDBNodeType.GTM_NODE == self._node_type:
            ret = self.__copy_from_gtm_node()
        elif GoldenDBNodeType.DATA_NODE == self._node_type:
            ret = self.__copy_from_data_node()
        else:
            log.error(f"node type[{self._node_type}] is invalid, {self.get_log_comm()}")
            log.error(f'step 5-2: Failed to execute copy log backup result job, {self.get_log_comm()}')
            return False

        if not ret:
            log.error(f'step 5-2: Failed to execute copy log backup result job, {self.get_log_comm()}')

        return ret

    def exec_report_data_size_job(self):
        log.info(f'step 5-3: start to report data size and speed， node type:{self._node_type}, '
                 f'{self.get_log_comm()}')

        # 数据量所有子任务只上报一次，因为UBC会叠加每个子任务上报的值, 所以在这个子任务中一次性上报
        # 这里直接返回True，实际计算在__report_success方法里
        return True

    def query_backup_copy(self):
        """
        上报副本信息子任务
        """
        cluster_id = self._cluster_structure.cluster_id
        end_time = self.__get_binlog_backup_end_time()
        last_time = self.__get_log_last_backup_time()
        version_file = os.path.join(self._cache_data_path, f"bkp_{self._job_id}_version.json")
        version_json = read_tmp_json_file(version_file)
        version = version_json.get("version")
        copy_info = {
            "extendInfo": {"cluster_id": cluster_id, "endTime": end_time, "beginTime": last_time,
                           "associatedCopies": [], "logDirName": self._log_data_path, "version": version}
        }
        output_result_file(self._req_id, copy_info)

    def get_log_comm(self):
        return f"pid:{self._req_id} jobId:{self._job_id} subjobId:{self._sub_id}"

    def __create_target_path(self):
        cluster_id = self._cluster_structure.cluster_id
        # /Database_xxxxxx_LogRepository/DBCluster_3/LOGICAL_BACKUP
        target_data_dir = os.path.join(self._log_data_path, f"DBCluster_{cluster_id}", "LOGICAL_BACKUP")
        # /home/goldendb/${role_name}/backup_root
        backup_path = get_backup_path(self._role_name, self._node_type, self._file_content,
                                      GoldenDBJsonConst.PROTECTOBJECT)
        group_name, _ = get_user_info(self._role_name)
        if not mkdir_chmod_chown_dir_recursively(target_data_dir, 0o770, self._role_name, group_name, True):
            log.error(f"fail to make a data path, {self.get_log_comm()}")
            return False, "", ""
        return True, target_data_dir, backup_path

    def __copy_from_data_node(self):
        """
        从数据节点拷贝日志备份结果
        """
        return self.__copy_file_by_time("Binlog")

    def __copy_from_gtm_node(self):
        """
        从gtm节点拷贝Sequence
        """
        last_sequence_time = self.__get_last_sequence_time()

        if last_sequence_time > 0:
            # sequence 每5分钟生成一次，等待下一次文件生成后，再进行备份, 为保险起见，等待6分钟
            need_sleep_time = 6 * 60 - (int(time.time()) - int(last_sequence_time))
        elif last_sequence_time < 0:
            # 等待6分钟
            need_sleep_time = 6 * 60
        else:
            # 不等待
            need_sleep_time = 0

        if need_sleep_time < 0:
            # 不等待
            need_sleep_time = 0

        log.error(f"need_sleep_time:{need_sleep_time}, {self.get_log_comm()}")
        time.sleep(need_sleep_time)
        return self.__copy_file_by_time("Sequence")

    def __copy_from_manager_node(self):
        """
        从关联节点拷贝Active_TX_Info
        """
        cluster_id = self._cluster_structure.cluster_id
        mkdir_ret, target_data_dir, backup_path = self.__create_target_path()
        if not mkdir_ret:
            return False

        # 备份meta数据
        log.info(f"start copy meta data from manager node, {self.get_log_comm()}")
        # meta数据地址 /home/goldendb/zxmanager/backup_root/DBCluster_3/LOGICAL_BACKUP/MetaData
        meta_data_src_dir = os.path.join(backup_path, f"DBCluster_{cluster_id}", "LOGICAL_BACKUP", "MetaData")
        if not self.__copy_dir(meta_data_src_dir, target_data_dir):
            return False

        # 备份活跃事务表
        log.info(f"start copy Active_TX_Info data from manager node, {self.get_log_comm()}")
        # 活跃事务表数据地址 /home/goldendb/zxmanager/backup_root/Active_TX_Info
        active_tx_info_dir = os.path.join(backup_path, "Active_TX_Info")
        tx_in_data_path = os.path.join(self._log_data_path, "Active_TX_Info")
        if self.__copy_dir(active_tx_info_dir, tx_in_data_path):
            self._remove_redundant_files(tx_in_data_path)
            return True
        else:
            return False

    def __copy_dir(self, src_dir, target_data_dir):
        if os.path.exists(src_dir):
            ret = exec_cp_dir_no_user(src_dir, target_data_dir, is_check_white_list=False, is_overwrite=True)
            if not ret:
                log.error(f"fail to copy data from {self._node_type} to log repository, {self.get_log_comm()}")
                return False
            log.info(f"copy data from {self._node_type}  success, {self.get_log_comm()}")
        else:
            log.info(f"src dir[{src_dir}] is not exist, {self.get_log_comm()}")
        return True

    def __copy_file_by_time(self, source_path):
        """
        按文件时间拷贝
        """
        log.info(f"5-2-1 start copy {source_path} data from data node, {self.get_log_comm()}")
        cluster_id = self._cluster_structure.cluster_id
        mkdir_ret, target_data_dir, backup_path = self.__create_target_path()
        if not mkdir_ret:
            return False

        # 日志备份结果目录 /home/zxdb5/backup_root/DBCluster_3/LOGICAL_BACKUP/
        logical_backup_dir = os.path.join(backup_path, f"DBCluster_{cluster_id}", "LOGICAL_BACKUP")
        if not os.path.isdir(logical_backup_dir):
            return True

        last_time = self.__get_log_last_backup_time()
        # 拷贝从上次备份到现在的日志文件（多拷贝10min日志，避免数据不完整）
        time_period_min = (int(time.time()) - int(last_time)) / 60 + 10

        # time_period_min数字类型， logical_backup_dir, target_node_path拼接过程均有校验
        # 切换到LOGICAL_BACKUP目录
        os.chdir(logical_backup_dir)
        # 检查LOGICAL_BACKUP/Binlog下的所有文件时间，满足要求的复制到target_data_dir, 使用-p保持文件原有属性，--parent保持原有路径。
        cp_cmd_list = [
            f"find {source_path} -mmin -{time_period_min} -type f ",
            "xargs -i /bin/cp --parent -pr {} " + target_data_dir
        ]
        log.info(f"backup_binlog cp_cmd_list is : {cp_cmd_list}")
        return_code, out_info, err_info = execute_cmd_list(cp_cmd_list)
        if return_code != CMDResult.SUCCESS:
            log.error(f'execute copy binlog cmd failed, message: {out_info}, err: {err_info}, '
                      f'{self.get_log_comm()}')
            return False
        log.info(f"5-2-1 finish copy {source_path} data from data node, {self.get_log_comm()}")
        return True

    def __get_last_sequence_time(self):
        cluster_id = self._cluster_structure.cluster_id
        mkdir_ret, target_data_dir, backup_path = self.__create_target_path()
        sequence_backup_file = os.path.join(backup_path, f"DBCluster_{cluster_id}", "LOGICAL_BACKUP", "Sequence",
                                            "sequence_backup.json")
        # /home/goldendb/zxgtm1/backup_root/DBCluster_3/LOGICAL_BACKUP/Sequence/sequence_backup.json
        if os.path.exists(sequence_backup_file):
            ret, result = exec_cat_cmd(sequence_backup_file, encoding='UTF-8')
            if not ret:
                return -1  # 等待6分钟
            sequence_backup_info = json.loads(result)
            last_backup_time = sequence_backup_info.get("ClusterMeta", {})[0].get("LastBackupTime", "")

            if last_backup_time:
                return int(datetime.datetime.strptime(last_backup_time, '%Y-%m-%d-%H:%M:%S').timestamp())
            else:
                return -1  # 等待6分钟
        return 0  # 不等待

    def __report_error(self):
        log.error(f"Exec sub job {self._sub_job_name} failed.{self.get_log_comm()}.")
        log_detail_param = [self._sub_id]
        log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_FALIED, logInfoParam=[self._sub_id],
                               logLevel=LogLevel.ERROR.value, logDetailParam=log_detail_param)

        report_job_details(self._req_id,
                           SubJobDetails(taskId=self._job_id, subTaskId=self._sub_id, progress=100,
                                         logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value))

    def __report_running(self):
        log_detail = LogDetail(logInfo=ReportDBLabel.BACKUP_SUB_START_COPY, logInfoParam=[self._sub_id],
                               logLevel=LogLevel.INFO)
        report_job_details(self._req_id,
                           SubJobDetails(taskId=self._job_id, subTaskId=self._sub_id, progress=0,
                                         logDetail=[log_detail], taskStatus=SubJobStatusEnum.RUNNING.value))

    def __report_success(self):
        log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS, logInfoParam=[self._sub_id],
                               logLevel=LogLevel.INFO)

        job_detail = SubJobDetails(taskId=self._job_id, subTaskId=self._sub_id, progress=100, logDetail=[log_detail],
                                   taskStatus=SubJobStatusEnum.COMPLETED.value)

        data_size, speed = self.__query_size_and_speed_binlog()
        job_detail.speed = speed
        # 数据量所有子任务只上报一次，因为UBC会叠加每个子任务上报的值
        if self._sub_job_name == SubJobName.EXEC_REPORT_DATA_SIZE:
            job_detail.data_size = data_size
        report_job_details(self._req_id, job_detail)

    def __create_sub_job(self, execute_node, job_name, priority):
        user_name = execute_node[0]
        agent_id = execute_node[2]
        goldendb_node_type = execute_node[3]
        job_info = f"{user_name} {goldendb_node_type}"
        return SubJob(jobId=self._job_id, execNodeId=agent_id, jobName=job_name, jobInfo=job_info,
                      jobPriority=priority).dict(by_alias=True)

    def __exec_log_backup_cmd(self):
        cluster_id = self._cluster_structure.cluster_id

        backup_cmd = "dbtool -cm -realtime-backup-binlog -clusterid={cluster_id}"
        # 需要输密码的命令
        ret, out_str = su_exec_cmd(self._role_name, backup_cmd,
                                   param_list=[[("cluster_id", cluster_id, ValidatorEnum.CHAR_CHK_COMMON)]])
        log.info(f'result: {(ret, out_str)}, {self.get_log_comm()}')

        if not self.__check_backup_result(ret, out_str):
            log.error(f'check_backup_result failed: {out_str}, {self.get_log_comm()}')
            return "", "", False
        return ret, out_str, True

    def __check_backup_result(self, ret, out_str):
        if not ret:
            if "response message: " in out_str:
                message = out_str.split("response message: ")[1].replace("\\r\\n", " ").rstrip(
                    "'").strip()
                log.debug(f"message :{message}")
                log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_FALIED, logInfoParam=[self._sub_id],
                                       logLevel=LogLevel.ERROR, logDetail=ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL,
                                       logDetailParam=["LogBackup", message])
            else:
                log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_FALIED, logInfoParam=[self._sub_id],
                                       logLevel=LogLevel.ERROR, logDetail=0)
            report_job_details(self._req_id,
                               SubJobDetails(taskId=self._job_id, subTaskId=self._sub_id, progress=100,
                                             logDetail=[log_detail],
                                             taskStatus=SubJobStatusEnum.FAILED.value))
            return False
        return True

    def __sub_job_pre_binlog(self):
        """
        之前日志备份命令前，记录备份开始的时间，以及备份目录原大小。用于副本可恢复的最后时间，以及计算备份速度。
        """
        start_time = str(int((time.time())))
        start_time_path = os.path.join(self._cache_data_path, f'T{self._job_id}')
        write_file(start_time_path, start_time)
        log.info(
            f"binlog success to write backup start time {start_time} to {start_time_path}, {self.get_log_comm()}")

        path_all = os.path.abspath(os.path.join(self._log_data_path))

        # 录入初始仓库大小
        original_size = safe_get_dir_size(path_all)

        original_data_size_path = os.path.join(self._cache_data_path, f'D{self._job_id}')
        write_file(original_data_size_path, str(original_size))
        log.info(f"binlog success to write backup start data size {original_size} to {original_data_size_path}, "
                 f"{self.get_log_comm()}")

    def __get_binlog_backup_end_time(self):
        start_time_file = os.path.join(self._cache_data_path, f'T{self._job_id}')
        # 读取日志备份开始时间，作为最迟可恢复时间。
        ret, backup_end_time = exec_cat_cmd(start_time_file, encoding='UTF-8')
        if not ret:
            log.error(f"Time file read failed, {self.get_log_comm()}")
            raise Exception(f"Read GoldenDB log backup start time file failed")
        return int(backup_end_time)

    def __query_size_and_speed_binlog(self):
        start_time_file = os.path.join(self._cache_data_path, f'T{self._job_id}')
        original_data_file = os.path.join(self._cache_data_path, f'D{self._job_id}')
        data_path = os.path.abspath(os.path.join(self._log_data_path))

        size = 0
        speed = 0
        # 读取初始时间
        ret, start_time = exec_cat_cmd(start_time_file, encoding='UTF-8')
        if not ret:
            log.error(f"Time file read failed, {self.get_log_comm()}")
            return size, speed

        start_time = start_time.strip()

        # 读取初始大小
        ret, original_data_size = exec_cat_cmd(original_data_file, encoding='UTF-8')
        if not ret:
            log.error(f"data file read failed, {self.get_log_comm()}")
            return size, speed
        original_data_size = original_data_size.strip()

        log.info(f"query_size_and_speed, start_time: {start_time}, original_data_size: {original_data_size}, "
                 f"{self.get_log_comm()}")
        size = safe_get_dir_size(data_path)
        new_time = int((time.time()))
        datadiff = int((size - int(original_data_size)))
        timediff = new_time - int(start_time)
        if timediff == 0:
            log.info(f"query_size_and_speed, timediff is {timediff}, {self.get_log_comm()}")
            return datadiff, speed
        try:
            speed = datadiff / timediff
        except Exception:
            log.error("Error while calculating speed! time difference is 0!, {self.get_log_comm()}")
            return 0, 0
        log.info(
            f"query_size_and_speed, datadiff: {datadiff}, timediff: {timediff}, speed: {speed}, {self.get_log_comm()}")
        return datadiff, speed

    def __get_log_last_backup_time(self):
        # 以上次日志备份作为起点，如果找不到上一次日志备份，则以上一次数据备份作为起点
        log.info(f"start get_log_backup_start_time, {self.get_log_comm()}")
        last_copy_info = self.__get_last_copy_info(3)
        if not last_copy_info:
            log.warning(f"This is the first log backup, {self.get_log_comm()}")
            last_copy_info = self.__get_last_copy_info(1)
        if not last_copy_info:
            log.error(f"Fail to get previous copy info, {self.get_log_comm()}")
            raise Exception("last copy not exist")
        log.info(f"last log copy info, id is: {last_copy_info.get('id', 'Null')}, {self.get_log_comm()}")

        last_copy_type = last_copy_info.get("type", "")
        log.info(f"get_last_any_copy_type copy_type is {last_copy_type}, {self.get_log_comm()}")
        if not last_copy_type:
            return ""

        extend_info = last_copy_info.get("extendInfo")
        if last_copy_type == CopyDataTypeEnum.LOG_COPY:
            last_time = extend_info.get("endTime", "")
        else:
            last_time = extend_info.get("backup_time")
        log.info(f'timestamp, last_copy info:{last_time}, {self.get_log_comm()}')
        return last_time

    def __get_last_copy_info(self, copy_type: int):
        # 获取上次数据备份副本信息
        log.info(f"start get_last_copy_info copy_type {copy_type}, {self.get_log_comm()}")
        last_copy_type = LastCopyType.last_copy_type_dict.get(copy_type)
        input_param = {
            RpcParamKey.APPLICATION: self._file_content.get("job", {}).get("protectObject"),
            RpcParamKey.TYPES: last_copy_type,
            RpcParamKey.COPY_ID: "",
            RpcParamKey.JOB_ID: self._job_id
        }
        log.info(f"start get_last_copy_info, {self.get_log_comm()}")
        try:
            result = invoke_rpc_tool_interface(self._job_id, RpcParamKey.QUERY_PREVIOUS_CPOY, input_param)
        except Exception as err_info:
            log.error(f"Get last copy info fail.{err_info}, {self.get_log_comm()}")
            return {}
        return result

    def _remove_redundant_files(self, tx_in_data_path):
        # 删除不是DBCluster_{cluster_id}开头的文件。 主要是删除其他cluster_id的相关文件.
        # 恢复时会把这些文件复制到生产端， 防止将非本次恢复的实例的文件复制到生产端，因此删除非要备份的实例的文件。
        for root, _, files in os.walk(tx_in_data_path):
            for file in files:
                if not file.startswith(f"DBCluster_{self._cluster_structure.cluster_id}"):
                    su_exec_rm_cmd(os.path.join(root, file))
