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

import os
import time
from concurrent.futures import ThreadPoolExecutor

from common.common import output_result_file, output_execution_result_ex, report_job_details
from common.common_models import SubJobDetails, SubJobModel, LogDetail, ActionResult, RepositoryPath, ScanRepositories
from common.const import BackupTypeEnum, ExecuteResultEnum, RepositoryDataTypeEnum, SubJobStatusEnum, SubJobTypeEnum, \
    SubJobPriorityEnum, ReportDBLabel, SubJobPolicyEnum, DBLogLevel
from common.job_const import JobNameConst
from common.util.exec_utils import su_exec_rm_cmd, exec_mkdir_cmd
from common.util.scanner_utils import scan_dir_size

from goldendb.handle.backup.parse_backup_params import get_goldendb_structure, get_master_or_slave_policy, write_file, \
    check_goldendb_structure, query_size_and_speed, get_backup_param_for_cm_backup, get_backup_param, \
    get_bkp_result_info_file
from goldendb.handle.common.const import GoldenDBMetaInfo, SubJobName, ManagerPriority, MasterSlavePolicy, \
    GoldenDBNodeType, GoldendbLabel, Report
from goldendb.handle.common.goldendb_common import verify_path_trustlist, get_repository_path, get_agent_uuids, \
    get_all_data_nodes, report_subjob_via_rpc, count_files, format_capacity, \
    get_recognized_err_from_sts_file, rm_redundant_incr_result_info, cp_active_folder, cp_result_info
from goldendb.logger import log
from goldendb.schemas.glodendb_schemas import TaskInfo, StatusInfo


class GoldenDBDataBackupToolService(object):

    def __init__(self, file_content=None, job_id=None, pid=None, sub_id=None):
        if not file_content:
            log.error("Pare params obj is null.")
            raise Exception("Parse params obj is null.")
        self._pid = pid
        self._job_id = job_id
        self._sub_id = sub_id
        self._log_comm = f"pid: {self._pid}, job_id: {self._job_id}, sub job id: {self._sub_id}"
        self._file_content = file_content
        self._structure_info_ = get_goldendb_structure(self._file_content)
        self._cid = self._structure_info_.cluster_id
        self._master_or_slave = get_master_or_slave_policy(self._file_content)  # return master or slave
        self._protect_obj_extend_info = self._file_content.get("job", {}).get("protectObject", {}).get("extendInfo", {})
        self._data_path = get_repository_path(self._file_content, RepositoryDataTypeEnum.DATA_REPOSITORY)
        self._meta_path = get_repository_path(self._file_content, RepositoryDataTypeEnum.META_REPOSITORY)
        self._cache_path = get_repository_path(file_content, RepositoryDataTypeEnum.CACHE_REPOSITORY)
        self._bkp_type = file_content.get("job", {}).get("jobParam", {}).get("backupType", BackupTypeEnum.FULL_BACKUP)
        log.debug(f"Param: {self._file_content}")

    def check_backup_type(self):
        # 检查本次增量文件系统中是否包含副本记录
        cluster_id = self._structure_info_.cluster_id
        if not verify_path_trustlist(self._data_path):
            log.error(f"Invalid src path: {self._data_path}, {self._log_comm}.")
            return False
        # 文件名，有后缀
        results_info_file = get_bkp_result_info_file(cluster_id, self._data_path)
        if not results_info_file:
            log.error(f"results_info_file not exist please check copy, {self._log_comm}.")
            return False
        # 检查meta中记录的上次备份副本集群结构与当前集群结构是否相同
        goldendb_info_file = os.path.join(self._meta_path, GoldenDBMetaInfo.GOLDENDBINFO)
        if not os.path.exists(goldendb_info_file):
            log.error(f"goldendb_info_file file: {goldendb_info_file} not exist please check copy, {self._log_comm}.")
            return False
        if not verify_path_trustlist(goldendb_info_file):
            log.error(f"Invalid src path: {goldendb_info_file}, {self._log_comm}.")
            return False
        log.info(f'goldendb_info_file path exist, {self._log_comm}.')
        with open(goldendb_info_file, "r", encoding='UTF-8') as file:
            old_cluster_info = file.read().strip()
        cur_cluster_info = self._protect_obj_extend_info.get("clusterInfo", "")
        if old_cluster_info != cur_cluster_info:
            log.error(f"goldendb_info change, {self._log_comm}.")
            return False
        return True

    def gen_sub_jobs(self):
        """
        功能描述：生成数据备份（全量备份，增量备份）子任务
        1）管理节点，数据节点，gtm节点生成挂载任务
        2）管理节点生成执行备份任务，并在meta仓中，生成任务状态文件
        """
        agent_uuids = get_agent_uuids(self._file_content)
        sub_jobs = []
        if self._master_or_slave == MasterSlavePolicy.SLAVE and not check_goldendb_structure(self._master_or_slave,
                                                                                             self._structure_info_):
            log_detail = LogDetail(logInfo=GoldendbLabel.pre_check_sla, logLevel=DBLogLevel.WARN.value)
            report_job_details(self._pid, SubJobDetails(taskId=self._job_id, progress=100, logDetail=[log_detail],
                                                        taskStatus=SubJobStatusEnum.RUNNING.value))
            master_or_slave = MasterSlavePolicy.MASTER
        # 添加所有在线的管理节点
        exec_nodes = [node for node in self._structure_info_.manager_nodes if node[2] in agent_uuids]
        # 添加所有在线的GTM节点
        exec_nodes.extend([node for node in self._structure_info_.gtm_nodes if node[2] in agent_uuids])
        # 获取所有数据节点
        data_nodes = get_all_data_nodes(self._structure_info_)
        # 添加所有在线的数据节点
        exec_nodes.extend([node for node in data_nodes if node[2] in agent_uuids])
        # 向本次任务所有执行节点生成子任务（1，挂载；2，备份），并在meta仓中生成管理节点的备份任务状态文件。
        self.gen_sub_on_managers(exec_nodes, sub_jobs)
        # 如果没有子任务，则报错
        if not sub_jobs:
            log.error(f'Generate zero sub job, {self._log_comm}.')
            raise Exception(f'Generate zero sub job, {self._log_comm}.')
        # 加入查询信息子任务，不添加的话不会调用queryCopy方法，不会上报给UBC
        sub_jobs.append(
            SubJobModel(jobId=self._job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value,
                        jobName=SubJobName.QUERY_COPY,
                        jobPriority=SubJobPriorityEnum.JOB_PRIORITY_3.value,
                        policy=SubJobPolicyEnum.ANY_NODE.value).dict(by_alias=True))
        log.info(f'Step 4: finish to execute backup_gen_sub_job, {self._log_comm}.')
        output_result_file(self._pid, sub_jobs)

    def gen_sub_on_managers(self, exec_nodes, sub_jobs):
        manager_node_priority = ManagerPriority.priority
        backup_status_file = os.path.join(self._meta_path, f'{self._job_id}_exec_backup_status.json')
        backup_status = {}
        for exec_node in exec_nodes:
            user_name, _, agent_id, goldendb_node_type = exec_node
            job_info = f"{user_name} {goldendb_node_type} {self._master_or_slave} {agent_id}"
            sub_jobs.append(
                SubJobModel(jobId=self._job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value, jobName=SubJobName.MOUNT,
                            jobPriority=SubJobPriorityEnum.JOB_PRIORITY_1.value, execNodeId=agent_id,
                            policy=SubJobPolicyEnum.FIXED_NODE.value, jobInfo=job_info).dict(by_alias=True))
            if goldendb_node_type == GoldenDBNodeType.ZX_MANAGER_NODE:
                # 设置每个管理节点执行备份的初始状态及优先级
                sts_info = StatusInfo(priority=manager_node_priority, status=SubJobStatusEnum.RUNNING.value)
                backup_status.update(
                    {agent_id: {'priority': sts_info.priority, 'status': sts_info.status, 'log_info': sts_info.log_info,
                                'log_detail_param': sts_info.log_detail_param, 'log_detail': sts_info.log_detail}})
                # dbtool 6.1.03.04版本后，备份模块从mds迁移到cm模块，备份方式从同步改为异步
                sub_jobs.append(SubJobModel(jobId=self._job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value,
                                            jobName=SubJobName.EXEC_BACKUP,
                                            jobPriority=SubJobPriorityEnum.JOB_PRIORITY_2.value, execNodeId=agent_id,
                                            policy=SubJobPolicyEnum.FIXED_NODE.value, jobInfo=job_info).dict(
                    by_alias=True))
                manager_node_priority += 1
        output_execution_result_ex(backup_status_file, backup_status)

    def backup(self):
        # 获取管理节点备份状态文件
        bkp_sts_file = os.path.join(self._meta_path, f'{self._job_id}_exec_backup_status.json')
        sub_job_name = self._file_content.get("subJob", {}).get("jobName")
        job_infos = self._file_content.get("subJob", {}).get("jobInfo").strip().split(" ")
        sub_job_model = SubJobModel(jobId=self._job_id, subJobId=self._sub_id)
        # 检查参数
        if len(job_infos) != 4:
            log.error(f'Step 5: failed to get job info, {self._log_comm}.')
            report_subjob_via_rpc(self._pid, sub_job_model, LogDetail())
            return
        role_name, role_type, sla_policy, agent_id = job_infos
        if not verify_path_trustlist(self._data_path):
            log.error(f"Step 5: invalid src path, {self._data_path}, {self._log_comm}.")
            report_subjob_via_rpc(self._pid, sub_job_model, LogDetail())
            return
        log.info(f"Step 5: start {sub_job_name} on {agent_id}, {self._log_comm}.")
        # 执行子任务
        if sub_job_name == SubJobName.MOUNT:
            pass
        elif sub_job_name == SubJobName.EXEC_BACKUP:
            pass
        log.info(
            f"Step 5: {sub_job_name} finished on {agent_id}, {self._log_comm}.")

    def report_backup_process(self, original_size, original_cnts, sub_job_details):
        sub_job_name = self._file_content.get("subJob", {}).get("jobName")
        if not verify_path_trustlist(self._data_path):
            log.error(f"Invalid src path: {self._data_path}.")
            return False
        if not os.path.exists(self._data_path):
            log.info(f"No data path: {self._data_path}, {sub_job_details}.")
            report_job_details(self._pid, sub_job_details)
            return True
        if sub_job_name == SubJobName.MOUNT:
            log.info(f"Reporting {sub_job_name}, {self._log_comm}.")
            log_detail = LogDetail(logInfo=ReportDBLabel.BACKUP_SUB_START_PREPARE,
                                   logInfoParam=[self._sub_id], logLevel=DBLogLevel.INFO.value)
        elif sub_job_name == SubJobName.EXEC_BACKUP:
            log.info(f"Reporting {sub_job_name}, {self._log_comm}.")
            time_file = os.path.join(self._cache_path, f'T{self._sub_id}')
            size, speed = query_size_and_speed(time_file, self._data_path, original_size, self._job_id)
            sub_job_details.progress = 50
            sub_job_details.speed = speed
            sub_job_details.data_size = size - original_size
            file_num = count_files(self._data_path) - original_cnts
            log_detail = LogDetail(logInfo=ReportDBLabel.BACKUP_SUB_JOB_SUCCESS,
                                   logInfoParam=[self._sub_id, str(file_num), format_capacity(size - original_size)],
                                   logLevel=DBLogLevel.INFO.value)
        else:
            log.info(f"Reporting {sub_job_name}, {self._log_comm}.")
            log_detail = LogDetail(logInfo=ReportDBLabel.BACKUP_SUB_START_COPY,
                                   logInfoParam=[self._sub_id], logLevel=DBLogLevel.INFO.value)
        sub_job_details.log_detail = [log_detail]
        report_job_details(self._pid, sub_job_details)
        log.info(f"Report success {sub_job_details}, {self._log_comm}.")
        return True

    def handle_files_after_bkp(self, prod_bkp_root):
        """
        复制活跃事务文件夹至文件系统, 复制备份结果文件至文件系统。
        参数: prod_bkp_root: goldendb生产端管理节点的备份路径
        返回值: bool: 如果删除成功，返回True，否则返回False
        """
        manager_os_user = self._structure_info_.manager_nodes[0][0]
        if not cp_active_folder(prod_bkp_root, self._data_path, manager_os_user):
            log.error(f'Copy active tx info folder failed, {self._log_comm}.')
            return False
        # 文件名，有后缀
        bkp_resultinfo_file = get_bkp_result_info_file(self._cid, prod_bkp_root)
        # 将生产端备份结果文件，复制到文件系统数据仓
        if not cp_result_info(self._cid, bkp_resultinfo_file, prod_bkp_root, self._data_path):
            log.error(f'Copy backup result info file failed, {self._log_comm}.')
            return False
        return True

    def rm_incr_redundants(self, pre_bkp_files):
        """
        当前备份完成后，文件系统中会包含生产环境里的所有文件，其中包括本次备份前已存在的文件，需要将其中冗余的增量备份文件删除
        如果本次备份为增量备份，遍历文件系统中的备份结果文件，并删除冗余的增量备份结果文件
        参数: pre_bkp_files: 本次备份前，生产环境中的文件列表
        返回值: bool: 如果删除成功，返回True，否则返回False
        """
        cid = self._structure_info_.cluster_id
        # 删除文件系统冗余文件，全量备份不需要处理
        if self._bkp_type != BackupTypeEnum.FULL_BACKUP:
            cid_path = os.path.join(self._data_path, f'DBCluster_{cid}')
            cluster_bkp_result_infos = [f for f in os.listdir(cid_path) if f.startswith(f'{cid}_back')]
            for result_info in cluster_bkp_result_infos:
                # 删除冗余的增量备份文件
                rm_redundant_incr_result_info(cid_path, result_info, pre_bkp_files)
                log.info(f"Remove redundant incremental files under {cid_path} finished, {self._log_comm}.")
        return True
