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
import datetime
import json
import os
import time
from concurrent.futures import ThreadPoolExecutor

from common.util.exec_utils import su_exec_rm_cmd
from common.common import check_command_injection_exclude_quote, output_result_file, output_execution_result_ex, \
    read_tmp_json_file, report_job_details
from common.common_models import SubJobDetails, SubJobModel, LogDetail, ActionResult
from common.const import BackupTypeEnum, ExecuteResultEnum, RepositoryDataTypeEnum, SubJobStatusEnum, SubJobTypeEnum, \
    SubJobPriorityEnum, ReportDBLabel, SubJobPolicyEnum, DBLogLevel
from common.file_common import get_user_info
from common.util.scanner_utils import scan_dir_size

from goldendb.handle.backup.goldendb_sqlite_service import GoldenDBSqliteService
from goldendb.handle.backup.log.goldendb_log_backup_tool_service import GoldenDBLogBackupToolService
from goldendb.handle.backup.parse_backup_params import get_goldendb_structure, get_master_or_slave_policy, \
    check_goldendb_structure, write_file, query_size_and_speed, exec_cmd_spawn, get_backup_param, get_copy_result_info
from goldendb.handle.common.const import ErrorCode, GoldenDBMetaInfo, GoldenDBCode, Report, SubJobName, CMDResult, \
    MasterSlavePolicy, GoldenDBJsonConst, GoldenDBNodeType, ManagerPriority
from goldendb.handle.common.goldendb_common import cp_active_folder, cp_result_info, get_backup_path, count_files, \
    get_bkp_type_from_result_info, get_bkp_result_names_in_cluster_id_dir, mount_bind_path, verify_path_trustlist, \
    umount_bind_path, get_repository_path, mkdir_chmod_chown_dir_recursively, umount_bind_backup_paths, \
    get_task_path_from_result_info, format_capacity, check_task_on_all_managers, exec_task_on_all_managers, \
    get_bkp_root_dir_via_role, get_agent_uuids, report_err_via_output_file, get_recognized_err_from_sts_file, \
    report_err_via_rpc, update_agent_sts_general_after_exec, update_agent_sts
from goldendb.handle.common.goldendb_param import JsonParam
from goldendb.handle.resource.resource_info import GoldenDBResourceInfo
from goldendb.logger import log
from goldendb.schemas.glodendb_schemas import TaskInfo, StatusInfo

MountParameters = collections.namedtuple('MountParameters', ['req_id', 'job_id', 'sub_id', 'data_path', 'file_content'])

BackupSubJobParameters = collections.namedtuple('BackupSubJobParameters',
                                                ['req_id', 'role_name', 'file_content', 'job_id', 'sub_id',
                                                 'sla_policy'])

ReportBkpProcParameters = collections.namedtuple('ReportBkpProcParameters',
                                                 ['req_id', 'job_id', 'process', 'file_content', 'original_size'])

BackupCmdParameters = collections.namedtuple('BackupCmdParameters',
                                             ['req_id', 'role_name', 'file_content', 'sla_policy', 'job_id', 'sub_id',
                                              'data_path'])


class GoldenDBBackupService(object):
    """
    备份任务相关接口
    """

    @staticmethod
    def allow_backup_in_local_node(req_id, job_id, sub_job_id):
        """
        功能描述：是否允许本地运行, 业务目前不需要实现, 主任务执行
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        @sub_job_id： 子任务ID
        返回值：
        """
        log.info(f'Step 1: allow_backup_in_local_node, req_id: {req_id}, job_id: {job_id}, sub_job_id: {sub_job_id}.')
        response = ActionResult(code=ExecuteResultEnum.SUCCESS)
        try:
            file_content = JsonParam.parse_param_with_jsonschema(req_id)
        except Exception as exception:
            log.error(exception, exc_info=True)
            return
        master_or_slave = get_master_or_slave_policy(file_content)
        cluster_structure = get_goldendb_structure(file_content)
        if master_or_slave == MasterSlavePolicy.SLAVE and not check_goldendb_structure(master_or_slave,
                                                                                       cluster_structure):
            log_detail = LogDetail(logInfo="goldendb_backup_pre_check_sla_failed_label", logLevel=DBLogLevel.WARN.value)
            report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_job_id, progress=100,
                                                     logDetail=[log_detail], taskStatus=SubJobStatusEnum.RUNNING.value))
            master_or_slave = MasterSlavePolicy.MASTER

        agent_uuids = get_agent_uuids(file_content)
        # 检查管理节点
        no_manager = all([node[2] not in agent_uuids for node in cluster_structure.manager_nodes])
        if no_manager:
            report_err_via_output_file(req_id, job_id, sub_job_id, "manager nodes all offline", )
            return
        log.info(f'Step 1: get manager nodes success, req_id: {req_id}, job_id: {job_id}, sub_job_id: {sub_job_id}.')
        # 检查GTM节点
        no_gtm = all([node[2] not in agent_uuids for node in cluster_structure.gtm_nodes])
        if no_gtm:
            report_err_via_output_file(req_id, job_id, sub_job_id, "gtms nodes all offline", )
            return
        log.info(f'Step 1: get gtm nodes success, req_id: {req_id}, job_id: {job_id}, sub_job_id: {sub_job_id}.')
        # 检查数据节点
        data_nodes = []
        for group in cluster_structure.data_nodes.values():
            if master_or_slave == MasterSlavePolicy.MASTER:
                data_nodes.append(group[MasterSlavePolicy.MASTER][0])
            else:
                data_nodes.extend(group[MasterSlavePolicy.SLAVE])
        # 只要有一个节点在线就下发任务
        no_dn = all([data_node[2] not in agent_uuids for data_node in data_nodes])
        if no_dn:
            report_err_via_output_file(req_id, job_id, sub_job_id, "data nodes all offline")
            return
        output_result_file(req_id, response.dict(by_alias=True))
        log.info(f"Step 1: get data nodes success, execute AllowBackupInLocalNode interface success.")

    @staticmethod
    def check_backup_job_type(req_id, job_id):
        """
        功能描述：检查增量备份是否转全量
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        返回值：
        """

        def check_backup_type(content):
            # 检查本次增量文件系统中是否包含副本记录
            data_path = get_repository_path(content, RepositoryDataTypeEnum.DATA_REPOSITORY)
            meta_path = get_repository_path(content, RepositoryDataTypeEnum.META_REPOSITORY)
            goldendb_structure = get_goldendb_structure(content)
            cluster_id = goldendb_structure.cluster_id
            if not verify_path_trustlist(data_path):
                log.error(f"Invalid src path: {data_path}.")
                return False
            # 文件名，有后缀
            results_info_file = GoldenDBBackupService.get_bkp_result_info_file(cluster_id, data_path)
            if not results_info_file:
                log.error(f"results_info_file not exist please check copy.")
                return False
            # 检查meta中记录的上次备份副本集群结构与当前集群结构是否相同
            goldendb_info_file = os.path.join(meta_path, GoldenDBMetaInfo.GOLDENDBINFO)
            if not os.path.exists(goldendb_info_file):
                log.error(f"goldendb_info_file file: {goldendb_info_file} not exist please check copy.")
                return False
            if not verify_path_trustlist(goldendb_info_file):
                log.error(f"Invalid src path: {goldendb_info_file}.")
                return False
            log.info('goldendb_info_file path exist')
            with open(goldendb_info_file, "r", encoding='UTF-8') as file:
                old_cluster_info = file.read().strip()
            cluster_info = content.get("job", {}).get("protectObject", {}).get("extendInfo", {}).get("clusterInfo",
                                                                                                     "")
            if old_cluster_info != cluster_info:
                log.error(f"goldendb_info change.")
                return False
            return True

        log.info(f'Step 2-1: start execute check_backup_job_type, pid: {req_id}, job_id:{job_id}')
        try:
            file_content = JsonParam.parse_param_with_jsonschema(req_id)
        except Exception as exception:
            log.error(exception, exec_info=True)
            return False
        backup_type = file_content.get("job", {}).get("jobParam", {}).get("backupType", BackupTypeEnum.FULL_BACKUP)
        if not backup_type:
            return False
        if backup_type == BackupTypeEnum.FULL_BACKUP:
            return True
        if not check_backup_type(file_content):
            response = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                    bodyErr=ErrorCode.ERROR_INCREMENT_TO_FULL,
                                    message="Can not apply this type backup job")
            output_result_file(req_id, response.dict(by_alias=True))
            log.info(f'Step 2-1: finish execute check_backup_job_type filed, pid: {req_id}, job_id:{job_id}.')
            return False
        response = ActionResult(code=ExecuteResultEnum.SUCCESS)
        output_result_file(req_id, response.dict(by_alias=True))
        log.info(f'Step 2-1: finish execute check_backup_job_type, pid: {req_id}, job_id:{job_id}.')
        return True

    @staticmethod
    def backup_pre_job(req_id, job_id):
        """
        功能描述：备份前置任务, 主任务执行, 当前资源的主备节点是否符合策略要求
        参数：
         @req_id： 请求ID
         @job_id： 主任务任务ID
        返回值：
        """
        log.info(f'Step 3: execute backup_pre_job, req_id: {req_id}, job_id: {job_id}.')
        response = ActionResult(code=ExecuteResultEnum.SUCCESS)
        try:
            JsonParam.parse_param_with_jsonschema(req_id)
        except Exception as exception:
            log.error(exception, exc_info=True)
            GoldenDBBackupService.set_error_response(response)
            output_result_file(req_id, response.dict(by_alias=True))
            return False
        output_result_file(req_id, response.dict(by_alias=True))
        GoldenDBBackupService.report_complete(job_id, req_id, "", 0)
        log.info(f'Step 3: finish to execute backup_pre_job,req_id:{req_id} job_id:{job_id}.')
        return True

    @staticmethod
    def backup_prerequisite_progress(req_id, job_id, sub_id):
        """
        功能描述：备份前置任务进度上报
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        @sub_id：子任务ID
        返回值：
        """
        log.info(
            f'Start execute backup_prerequisite_job_progress, req_id: {req_id}, job_id: {job_id},  sub_id: {sub_id}.')
        progress = SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100,
                                 taskStatus=SubJobStatusEnum.COMPLETED.value)
        output_result_file(req_id, progress.dict(by_alias=True))

    @staticmethod
    def backup_gen_sub_job(req_id, job_id):
        """
        功能描述：备份扫描任务
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        返回值：
        """
        log.info(f'Step 4: start to execute backup_gen_sub_job, req_id: {req_id}, job_id: {job_id}.')
        try:
            file_content = JsonParam.parse_param_with_jsonschema(req_id)
        except Exception as exception:
            log.error(exception, exc_info=True)
            return

        backup_type = file_content.get("job", {}).get("jobParam", {}).get("backupType", BackupTypeEnum.FULL_BACKUP)
        if backup_type == BackupTypeEnum.LOG_BACKUP:
            try:
                goldendb_backup_tool = GoldenDBLogBackupToolService(file_content=file_content, job_id=job_id,
                                                                    req_id=req_id)
                goldendb_backup_tool.gen_sub_jobs()
            except Exception as exception:
                log.error(exception, exc_info=True)
                response = ActionResult(code=ExecuteResultEnum.SUCCESS)
                GoldenDBBackupService.set_error_response(response)
                GoldenDBBackupService.report_error_job_details(job_id, req_id, response, job_id)
        else:
            GoldenDBBackupService.full_and_inc_gen_sub_jobs(file_content, job_id, req_id)

    @staticmethod
    def full_and_inc_gen_sub_jobs(file_content, job_id, req_id):
        agent_uuids = get_agent_uuids(file_content)
        response = []
        master_or_slave = get_master_or_slave_policy(file_content)  # return master or slave
        cluster_structure = get_goldendb_structure(file_content)
        if master_or_slave == MasterSlavePolicy.SLAVE and not check_goldendb_structure(master_or_slave,
                                                                                       cluster_structure):
            log_detail = LogDetail(logInfo="goldendb_backup_pre_check_sla_failed_label", logLevel=DBLogLevel.WARN.value)
            report_job_details(req_id, SubJobDetails(taskId=job_id, progress=100, logDetail=[log_detail],
                                                     taskStatus=SubJobStatusEnum.RUNNING.value))
            master_or_slave = MasterSlavePolicy.MASTER
        # 添加所有在线的管理节点
        exec_nodes = [node for node in cluster_structure.manager_nodes if node[2] in agent_uuids]
        # 添加所有在线的GTM节点
        exec_nodes.extend([node for node in cluster_structure.gtm_nodes if node[2] in agent_uuids])
        # 添加所有在线的GTM节点
        data_nodes = []
        for group in cluster_structure.data_nodes.values():
            if master_or_slave == MasterSlavePolicy.MASTER:
                data_nodes.append(group[MasterSlavePolicy.MASTER][0])
            else:
                data_nodes.extend(group[MasterSlavePolicy.SLAVE])
        exec_nodes.extend([node for node in data_nodes if node[2] in agent_uuids])
        # 向本次任务所有执行节点下发子任务
        manager_node_priority = ManagerPriority.priority
        meta_path = get_repository_path(file_content, RepositoryDataTypeEnum.META_REPOSITORY)
        backup_status_file = os.path.join(meta_path, f'{job_id}_exec_backup_status.json')
        backup_status = {}
        for exec_node in exec_nodes:
            user_name, _, agent_id, goldendb_node_type = exec_node
            job_info = f"{user_name} {goldendb_node_type} {master_or_slave} {agent_id}"
            response.append(
                SubJobModel(jobId=job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB, execNodeId=agent_id,
                            jobName=SubJobName.MOUNT, jobInfo=job_info, policy=SubJobPolicyEnum.FIXED_NODE,
                            jobPriority=SubJobPriorityEnum.JOB_PRIORITY_1).dict(by_alias=True))
            if goldendb_node_type == GoldenDBNodeType.ZX_MANAGER_NODE:
                # 设置每个管理节点执行备份的初始状态及优先级
                sts_info = StatusInfo(priority=manager_node_priority, status=SubJobStatusEnum.RUNNING.value)
                backup_status.update(
                    {agent_id: {'priority': sts_info.priority, 'status': sts_info.status, 'log_info': sts_info.log_info,
                                'log_detail_param': sts_info.log_detail_param, 'log_detail': sts_info.log_detail}})
                response.append(
                    SubJobModel(jobId=job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB, execNodeId=agent_id,
                                jobName=SubJobName.EXEC_BACKUP, jobInfo=job_info, policy=SubJobPolicyEnum.FIXED_NODE,
                                jobPriority=SubJobPriorityEnum.JOB_PRIORITY_2).dict(by_alias=True))
                manager_node_priority += 1
        output_execution_result_ex(backup_status_file, backup_status)
        # 如果没有子任务，则报错
        if not response:
            log.error(f'Generate zero sub job, job id: {job_id}.')
            raise Exception(f'Generate zero sub job, job id: {job_id}.')
        # 加入查询信息子任务，不添加的话不会调用queryCopy方法，不会上报给UBC
        response.append(
            SubJobModel(jobId=job_id, jobType=SubJobTypeEnum.BUSINESS_SUB_JOB, policy=SubJobPolicyEnum.ANY_NODE,
                        jobPriority=SubJobPriorityEnum.JOB_PRIORITY_4, jobName='queryCopy').dict(by_alias=True))
        log.info(f'Step 4: finish to execute backup_gen_sub_job, req_id: {req_id}, job_id: {job_id}.')
        output_result_file(req_id, response)

    @staticmethod
    def backup(req_id, job_id, sub_id):
        """
        功能描述：执行备份任务
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        @sub_id：子任务ID
        返回值：
        """
        response = ActionResult(code=ExecuteResultEnum.SUCCESS)
        log.info(f'Step 5: execute backup, req_id: {req_id}, job_id: {job_id},sub_id:{sub_id}.')
        try:
            file_content = JsonParam.parse_param_with_jsonschema(req_id)
        except Exception as exception:
            log.error(exception, exc_info=True)
            GoldenDBBackupService.set_error_response(response)
            GoldenDBBackupService.report_error_job_details(job_id, req_id, response, sub_id)
            return

        backup_type = file_content.get("job", {}).get("jobParam", {}).get("backupType", BackupTypeEnum.FULL_BACKUP)
        if backup_type == BackupTypeEnum.LOG_BACKUP:
            try:
                goldendb_backup_tool = GoldenDBLogBackupToolService(file_content=file_content, job_id=job_id,
                                                                    req_id=req_id, sub_id=sub_id)
                goldendb_backup_tool.backup()
            except Exception as exception:
                log.error(exception, exc_info=True)
                GoldenDBBackupService.set_error_response(response)
                GoldenDBBackupService.report_error_job_details(job_id, req_id, response, sub_id)
        else:
            GoldenDBBackupService.exec_data_bkp_sub_with_report(file_content, job_id, req_id, response, sub_id)
            # 上报未捕获的异常
            output_result_file(req_id, response.dict(by_alias=True))
            GoldenDBBackupService.report_error_job_details(job_id, req_id, response, sub_id)

    @staticmethod
    def exec_data_bkp_sub_with_report(file_content, job_id, req_id, response, sub_id):
        # 获取管理节点备份状态文件
        meta_path = get_repository_path(file_content, RepositoryDataTypeEnum.META_REPOSITORY)
        bkp_sts_file = os.path.join(meta_path, f'{job_id}_exec_backup_status.json')
        sub_job_name = file_content.get("subJob", {}).get("jobName")
        job_infos = file_content.get("subJob", {}).get("jobInfo").strip().split(" ")
        # 检查参数
        if len(job_infos) != 4:
            log.error(f'Step 5: failed to get job info, job_id: {job_id}.')
            GoldenDBBackupService.set_error_response(response)
            GoldenDBBackupService.report_error_job_details(job_id, req_id, response, sub_id)
        role_name, role_type, sla_policy, agent_id = job_infos
        data_path = get_repository_path(file_content, RepositoryDataTypeEnum.DATA_REPOSITORY)
        if not verify_path_trustlist(data_path):
            log.error(f"Step 5: invalid src path, {data_path}, job_id: {job_id},sub_id:{sub_id}.")
            GoldenDBBackupService.set_error_response(response)
            GoldenDBBackupService.report_error_job_details(job_id, req_id, response, sub_id)
        log.info(f"Step 5: start {sub_job_name} on {agent_id}, req_id: {req_id}, job_id: {job_id}, sub_id: {sub_id}.")
        # 执行子任务
        if sub_job_name == SubJobName.MOUNT:
            mount_parameters = MountParameters(req_id, job_id, sub_id, data_path, file_content)
            if not GoldenDBBackupService.exec_mount_sub_job(mount_parameters):
                log.error(f"Step 5: failed to mount bind backup path, job_id: {job_id}, sub_id: {sub_id}.")
                GoldenDBBackupService.set_error_response(response)
        elif sub_job_name == SubJobName.EXEC_BACKUP:
            bkp_sub_parameters = BackupSubJobParameters(req_id, role_name, file_content, job_id, sub_id, sla_policy)
            if not GoldenDBBackupService.exec_backup_sub_job(bkp_sub_parameters):
                log.error(f"Step 5: failed to exec backup sub job on {agent_id}, job_id: {job_id}.")
                ret = GoldenDBBackupService.check_backup_on_all_managers(agent_id, bkp_sts_file, job_id, req_id,
                                                                         sub_id)
                if not ret:
                    log.error(f"Step 5: failed to exec backup sub job on all managers, job_id: {job_id}.")
                    log_detail, log_detail_param, log_info = get_recognized_err_from_sts_file(agent_id, bkp_sts_file,
                                                                                              "Backup")
                    log_details = [log_detail, log_detail_param, log_info]
                    report_err_via_rpc(req_id, job_id, sub_id, log_details)
                else:
                    log.info(f"Step 5: check backup sub job on all managers success, job_id: {job_id}.")
                    GoldenDBBackupService.report_complete(job_id, req_id, sub_id, SubJobTypeEnum.BUSINESS_SUB_JOB)
            else:
                GoldenDBBackupService.report_complete(job_id, req_id, sub_id, SubJobTypeEnum.BUSINESS_SUB_JOB)
        log.info(
            f"Step 5: {sub_job_name} finished on {agent_id}, req_id: {req_id}, job_id: {job_id}, sub_id: {sub_id}.")

    @staticmethod
    def check_backup_on_all_managers(agent_id, bkp_sts_file, job_id, req_id, sub_id):
        # 判断所有管理节点的备份结果
        task_infos = TaskInfo(pid=req_id, jobId=job_id, subJobId=sub_id, taskType="Backup",
                              logComm=f"pid: {req_id}, job_id: {job_id}, sub job id: {sub_id}")
        try:
            ret = check_task_on_all_managers(agent_id, bkp_sts_file, "Step 5-2: exec backup", task_infos)
        except Exception as ex:
            log.error(f"Step 5-2: {agent_id} check backup failed, message: {ex}, job_id: {job_id}.")
            ret = False
        return ret

    @staticmethod
    def backup_post_job(req_id, job_id):
        """
        功能描述：执行备份后置任务任务
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        返回值：
        """
        log.info(f'Step 7: start execute backup_post_job, req_id: {req_id}, job_id: {job_id}.')
        umount_bind_backup_paths(job_id)

        response = ActionResult(code=ExecuteResultEnum.SUCCESS)
        output_result_file(req_id, response.dict(by_alias=True))
        log.info(f'Step 7: finish execute backup_post_job, req_id: {req_id}, job_id: {job_id}.')

    @staticmethod
    def backup_post_job_progress(req_id, job_id, sub_id):
        """
        功能描述：备份后置任务任务进度上报
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        @sub_id：子任务ID
        返回值：
        """
        log.info(f'execute backup_post_job_progress, req_id: {req_id}, job_id: {job_id},  sub_id: {sub_id}.')
        progress = SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100,
                                 taskStatus=SubJobStatusEnum.COMPLETED.value)
        output_result_file(req_id, progress.dict(by_alias=True))
        return ExecuteResultEnum.SUCCESS

    @staticmethod
    def query_backup_copy(req_id, job_id):
        """
        功能描述：查询上报副本
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        @sub_id：子任务ID
        返回值：
        """
        log.info(f'Step 6: execute to query_backup_copy, pid: {req_id}, job_id: {job_id}.')
        try:
            file_content = JsonParam.parse_param_with_jsonschema(req_id)
        except Exception as exception:
            log.error(exception, exc_info=True)
            return
        backup_type = file_content.get("job", {}).get("jobParam", {}).get("backupType", BackupTypeEnum.FULL_BACKUP)
        if not backup_type:
            return
        goldendb_structure = get_goldendb_structure(file_content)
        cluster_id = goldendb_structure.cluster_id
        if backup_type == BackupTypeEnum.LOG_BACKUP:
            goldendb_backup_tool = GoldenDBLogBackupToolService(file_content=file_content, job_id=job_id,
                                                                req_id=req_id)
            goldendb_backup_tool.query_backup_copy()
        else:
            data_path = get_repository_path(file_content, RepositoryDataTypeEnum.DATA_REPOSITORY)
            if not verify_path_trustlist(data_path):
                log.error(f"Invalid src path: {data_path}.")
                return
            time_info = get_copy_result_info(data_path, cluster_id)
            backup_time_str = time_info[0].split('.')[1]
            dt = datetime.datetime.strptime(backup_time_str, '%Y%m%d%H%M%S')
            backup_time = int(dt.timestamp())
            cache_path = get_repository_path(file_content, RepositoryDataTypeEnum.CACHE_REPOSITORY)
            version_file = os.path.join(cache_path, f"bkp_{job_id}_version.json")
            version_json = read_tmp_json_file(version_file)
            version = version_json.get("version")
            copy_id = file_content.get("job", {}).get("copy", [])[0].get("id", "")
            copy_info = {
                "extendInfo": {"cluster_id": cluster_id, "time_info": time_info, "backup_time": backup_time,
                               "backup_end_time": backup_time, "copy_id": copy_id, "version": version}
            }
            output_result_file(req_id, copy_info)

    @staticmethod
    def report_complete(job_id, req_id, sub_id, job_type):
        if not sub_id and job_type == SubJobTypeEnum.PRE_SUB_JOB:
            log_detail = LogDetail(logInfo="plugin_execute_prerequisit_task_success_label",
                                   logLevel=DBLogLevel.INFO.value)
            report_job_details(req_id,
                               SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, logDetail=[log_detail],
                                             taskStatus=SubJobStatusEnum.COMPLETED.value))
        elif job_type == SubJobTypeEnum.BUSINESS_SUB_JOB:
            log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS, logInfoParam=[sub_id],
                                   logLevel=DBLogLevel.INFO.value)
            report_job_details(req_id,
                               SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, logDetail=[log_detail],
                                             taskStatus=SubJobStatusEnum.COMPLETED.value))

    @staticmethod
    def set_error_response(response):
        response.code = GoldenDBCode.FAILED.value
        response.body_err = GoldenDBCode.FAILED.value

    @staticmethod
    def exec_mount_sub_job(mount_parameters):
        log.info(f'Step 5-1: start execute mount sub job.')
        file_content = mount_parameters.file_content
        data_path = mount_parameters.data_path
        sub_id = mount_parameters.sub_id
        req_id = mount_parameters.req_id
        job_id = mount_parameters.job_id
        job_infos = file_content.get("subJob", {}).get("jobInfo", " ").strip().split(" ")
        if len(job_infos) != 4:
            log.error(f'Step 5-1: failed to get job info, job_id: {job_id}.')
            return False
        role_name, role_type, _, _ = job_infos
        backup_path = get_backup_path(role_name, role_type, file_content, GoldenDBJsonConst.PROTECTOBJECT)
        goldendb_structure = get_goldendb_structure(file_content)
        cluster_id = goldendb_structure.cluster_id
        ini_bkp_root_dir = get_bkp_root_dir_via_role(role_type, backup_path, job_id)
        if role_type == GoldenDBNodeType.GTM_NODE:
            data_bkp_mnt_dir = os.path.join(data_path, f'DBCluster_{cluster_id}', 'LOGICAL_BACKUP')
            prod_bkp_mnt_dir = os.path.join(ini_bkp_root_dir, f'DBCluster_{cluster_id}', 'LOGICAL_BACKUP')
        else:
            data_bkp_mnt_dir = os.path.join(data_path, f'DBCluster_{cluster_id}', 'DATA_BACKUP')
            prod_bkp_mnt_dir = os.path.join(ini_bkp_root_dir, f'DBCluster_{cluster_id}', 'DATA_BACKUP')
        # 解挂载生产端的备份根目录
        umount_bind_path(prod_bkp_mnt_dir)
        group_name, _ = get_user_info(role_name)
        if not mkdir_chmod_chown_dir_recursively(data_bkp_mnt_dir, 0o770, role_name, group_name, True):
            log.error("Fail to make a data path, job_id: {job_id}.")
            return False
        if not mkdir_chmod_chown_dir_recursively(prod_bkp_mnt_dir, 0o770, role_name, group_name, True):
            log.error("Fail to make a back path, job_id: {job_id}.")
            return False
        if not mount_bind_path(data_bkp_mnt_dir, prod_bkp_mnt_dir, job_id):
            log.error(f"Mount data_path failed, job_id: {job_id}.")
            return False
        log.info("Step 5-1:finish Mount bind all backup path success, job_id: {job_id}.")
        GoldenDBBackupService.report_complete(job_id, req_id, sub_id, SubJobTypeEnum.BUSINESS_SUB_JOB)
        return True

    @staticmethod
    def exec_backup_sub_job(backup_parameters):
        job_id, sub_id, req_id, file_content, role_name, sla_policy = (
            backup_parameters.job_id, backup_parameters.sub_id, backup_parameters.req_id,
            backup_parameters.file_content, backup_parameters.role_name, backup_parameters.sla_policy
        )
        meta_path = get_repository_path(file_content, RepositoryDataTypeEnum.META_REPOSITORY)
        backup_status_file = os.path.join(meta_path, f'{job_id}_exec_backup_status.json')
        log.info(f"Step 5-2: start to backup, pid: {req_id}, job_id: {job_id}, sub job id: {sub_id}")
        task_infos = TaskInfo(pid=req_id, jobId=job_id, subJobId=sub_id, taskType="Backup",
                              jsonParam=file_content,
                              logComm=f"pid: {req_id}, job_id: {job_id}, sub job id: {sub_id}")
        result = exec_task_on_all_managers(GoldenDBBackupService.get_exec_bkp_result, backup_status_file,
                                           f"Step 5-2: exec backup ", task_infos, backup_parameters)
        return result

    @staticmethod
    def get_exec_bkp_result(agent_id, backup_status_file, backup_parameters):
        """
        功能描述：从单个管理节点中获取备份结果，根据该结果，更新恢复状态文件。
        1. 执行成功
        2. dbtool原生异常已在执行后完成更新，只需处理其余异常。
        """
        job_id = backup_parameters.job_id
        backup_result = GoldenDBBackupService.exec_bkp_on_manager(backup_parameters)
        update_agent_sts_general_after_exec(agent_id, backup_status_file, "Backup", backup_result, f"job id: {job_id}")
        return backup_result

    @staticmethod
    def get_id_status_via_priority(backup_status_file, priority, job_id):
        results = read_tmp_json_file(backup_status_file)
        if not results:
            log.error(f'Step 5-2: job_id:{job_id}, read backup status file failed.')
            return "", SubJobStatusEnum.FAILED.value
        for agent_id, value in results.items():
            cur_priority = value.get("priority")
            if priority == cur_priority:
                log.info(
                    f'Step 5-2: job_id:{job_id}, agent_id: {agent_id}, '
                    f'priority: {priority}, status: {value.get("status")}.'
                )
                return agent_id, value.get("status")
        log.error(f'Step 5-2: job_id:{job_id}, get_id_status_via_priority failed.')
        return "", SubJobStatusEnum.FAILED.value

    @staticmethod
    def exec_bkp_on_manager(backup_parameters):
        """
        功能描述：在单个管理节点中执行备份
        """
        job_id, sub_id, req_id, file_content, role_name, sla_policy = (
            backup_parameters.job_id, backup_parameters.sub_id, backup_parameters.req_id,
            backup_parameters.file_content, backup_parameters.role_name, backup_parameters.sla_policy
        )
        log.info(f'Step 5-2: start to execute backup, job_id:{job_id}, sub_id:{sub_id}, req_id:{req_id}.')
        # 获取集群版本
        version = GoldenDBResourceInfo.get_cluster_version(role_name)
        cache_path = get_repository_path(file_content, RepositoryDataTypeEnum.CACHE_REPOSITORY)
        version_file = os.path.join(cache_path, f"bkp_{job_id}_version.json")
        output_execution_result_ex(version_file, {"version": version})
        data_path = get_repository_path(file_content, RepositoryDataTypeEnum.DATA_REPOSITORY)
        if not verify_path_trustlist(data_path):
            log.error(f"Invalid src path: {data_path}, job_id:{job_id}, sub_id:{sub_id}, req_id:{req_id}.")
            return False
        original_size = 0
        ret, size = scan_dir_size(job_id, data_path)
        if ret:
            original_size = size
        process = SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=0, taskStatus=SubJobStatusEnum.RUNNING)
        log_detail = LogDetail(logInfo=ReportDBLabel.BACKUP_SUB_START_COPY, logInfoParam=[process.sub_task_id],
                               logLevel=DBLogLevel.INFO.value)
        process.log_detail = [log_detail]
        report_job_details(req_id, process)
        # 清空上报进度
        process.log_detail = None
        pool = ThreadPoolExecutor(max_workers=1, thread_name_prefix='goldendb-backup')
        message = str(int((time.time())))
        write_file(os.path.join(cache_path, f'T{job_id}'), message)

        backup_cmd_parameters = BackupCmdParameters(req_id, role_name, file_content, sla_policy, job_id, sub_id,
                                                    data_path)
        # 执行备份指令，完成后，检查命令回显以及文件系统数据仓中的副本文件
        copy_feature = pool.submit(GoldenDBBackupService.exec_backup_cmd, backup_cmd_parameters)
        while not copy_feature.done():
            time.sleep(Report.REPORT_INTERVAL)
            report_bkp_proc_parameters = ReportBkpProcParameters(req_id, job_id, process, file_content, original_size)
            if not GoldenDBBackupService.report_backup_process(report_bkp_proc_parameters):
                log.error(f'Full backup failed: backup root error.')
                pool.shutdown()
                return False
        if not copy_feature.result()[0]:
            return False
        data_size = copy_feature.result()[1] - original_size
        process.data_size = data_size
        process.task_status = SubJobStatusEnum.COMPLETED.value
        process.progress = 100
        log_detail = LogDetail(logInfo=ReportDBLabel.BACKUP_SUB_JOB_SUCCESS,
                               logInfoParam=[process.sub_task_id, count_files(data_path),
                                             format_capacity(copy_feature.result()[1])], logLevel=DBLogLevel.INFO.value)
        process.log_detail = [log_detail]
        report_job_details(req_id, process)
        log.info(f'Step 5-2: success to execute backup, job_id: {job_id}, sub_id: {sub_id}, req_id: {req_id}.')
        return True

    @staticmethod
    def check_bkp_cmd_echo(copy_feature, job_id, sub_id):
        """
        功能描述：检查备份指令回显内容，返回可识别的异常，其余异常，默认为ErrorCode.ERROR_INTERNAL。
        """
        log_detail_param = None
        if int(copy_feature[0]) != int(CMDResult.SUCCESS):
            log_detail = ErrorCode.ERROR_INTERNAL
            log_info = ReportDBLabel.SUB_JOB_FALIED
            log.error(f"Step 5: failed to exec dbtool cmd, job_id: {job_id}, sub_id: {sub_id}.")
            if "response message: " in copy_feature[1]:
                log.error(f"Step 5: failed to exec dbtool cmd, err in response, job_id: {job_id}, sub_id: {sub_id}.")
                message = copy_feature[1].split("response message: ")[1].replace("\\r\\n", " ").rstrip(
                    "'").strip()
                log.debug(f"message: {message}")
                log_detail = ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL
                log_detail_param = ["Backup", message]
            return False, log_detail, log_detail_param, log_info
        log.info(f"Step 5: exec dbtool cmd success, job_id: {job_id}, sub_id: {sub_id}.")
        # 检查正确，无错误码，无错误参数，无错误标签
        return True, None, None, ReportDBLabel.SUB_JOB_SUCCESS

    @staticmethod
    def check_dn_xtream_results(file_content):
        """
        功能描述：根据备份结果文件内容，获取各数据节点的备份结果文件，检查文件系统中该文件是否存在。
        返回可识别的异常，其余异常，默认为ErrorCode.ERROR_INTERNAL。
        """
        data_path = get_repository_path(file_content, RepositoryDataTypeEnum.DATA_REPOSITORY)
        goldendb_structure = get_goldendb_structure(file_content)
        cluster_id = goldendb_structure.cluster_id
        # 获取备份结果文件路径，文件名，有后缀
        results_info_file = GoldenDBBackupService.get_bkp_result_info_file(cluster_id, data_path)
        if not results_info_file:
            # 文件系统中，备份结果文件名为空，异常
            log.error(f'check_dn_xtream_results results_info_path failed, no backup_resultsinfo.')
            _, task_id = get_copy_result_info(data_path, cluster_id)
            err_path = os.path.join(data_path, f'DBCluster_{cluster_id}', f"{cluster_id}_backup_resultsinfo.{task_id}")
            log_detail_param = [err_path]
            return False, ErrorCode.ERR_BKP_CHECK, log_detail_param, ReportDBLabel.COPY_VERIFICATION_FALIED
        results_info_path = os.path.join(data_path, f"DBCluster_{cluster_id}", results_info_file)
        if os.path.exists(results_info_path):
            with open(results_info_path, encoding='utf-8') as file:
                # 读取备份结果文件内容
                contents = file.readlines()
        else:
            # 文件系统中，备份结果文件路径不存在，异常
            log.error(f'check_dn_xtream_results results_info_path failed.')
            log_detail_param = [results_info_path]
            return False, ErrorCode.ERR_BKP_CHECK, log_detail_param, ReportDBLabel.COPY_VERIFICATION_FALIED
        log.debug(f'check_dn_xtream_results contents: {contents}.')
        record_info_list = []
        for record in contents:
            record_info = record.split()
            record_info_list.append(record_info)
        log.debug(f'check_dn_xtream_results record_info_list :{record_info_list}.')
        # 判断备份结果文件列出的数据节点文件在文件系统中是否存在
        for record_info in record_info_list:
            ret, err_path = GoldenDBBackupService.check_dn_xtream_result(record_info, data_path, cluster_id)
            if not ret:
                # 文件系统中，数据节点副本缺失，异常
                log.error(f'check_dn_xtream_results check_dn_xtream_result failed, not exist: {err_path}.')
                log_detail_param = [err_path]
                return False, ErrorCode.ERR_BKP_CHECK, log_detail_param, ReportDBLabel.COPY_VERIFICATION_FALIED
        # 检查正确，无错误码，无错误参数，无错误标签
        return True, None, None, ReportDBLabel.SUB_JOB_SUCCESS

    @staticmethod
    def get_bkp_result_info_file(cluster_id, data_path):
        # 备份根目录/DBCluster_{cluster_id}/DATA_BACKUP/{task_id}/ResultInfo/{cluster_id}_backup_resultsinfo_1.{timestamp}
        results_info_file = ""
        expect_file_name = f"{cluster_id}_backup_"
        bkp_result_dir = os.path.join(data_path, f'DBCluster_{cluster_id}')
        if os.path.exists(bkp_result_dir):
            cluster_bkp_result_infos = [f for f in os.listdir(bkp_result_dir) if f.startswith(expect_file_name)]
            if len(cluster_bkp_result_infos) > 0:
                cluster_bkp_result_infos.sort()
                results_info_file = cluster_bkp_result_infos[-1]
        return results_info_file

    @staticmethod
    def check_dn_xtream_result(record_info, data_path, cluster_id):
        """
        功能描述：检查数据节点生成的备份结果文件，在文件系统中是否存在。
        备份根目录/DBCluster_{cid}/DATA_BACKUP/{tid}/Data/Node_{groupid}_{roomid}_{ip}_{port}/{ip}_{bkp_type}_{tid}.xtream
        """
        # 在备份结果文件中，3表示副本信息
        if int(record_info[0]) == 3:
            _, task_id = get_copy_result_info(data_path, cluster_id)
            dn_result_pattern = f"/DBCluster_{cluster_id}/DATA_BACKUP/{task_id}/Data/"
            xbstream_list = list(filter(lambda x: dn_result_pattern in x, record_info))
            if xbstream_list:
                dn_result_relative_path = xbstream_list[0].split(dn_result_pattern)[1]
                dn_xbstream_path = os.path.join(data_path, f"DBCluster_{cluster_id}", "DATA_BACKUP", f"{task_id}",
                                                "Data", dn_result_relative_path)
                if not verify_path_trustlist(dn_xbstream_path):
                    log.error(f"Invalid src path: {dn_xbstream_path}.")
                    return False, dn_xbstream_path
                if not os.path.exists(dn_xbstream_path):
                    log.error(f'check_dn_bkp_path xbstream_path not exist: {dn_xbstream_path}.')
                    return False, dn_xbstream_path
        return True, ''

    @staticmethod
    def write_backup_result(cluster_structure, data_path, file_content):
        # 写clusterInfo
        cluster_info = file_content["job"]["protectObject"]["extendInfo"]["clusterInfo"]
        meta_path = get_repository_path(file_content, RepositoryDataTypeEnum.META_REPOSITORY)
        output_file = os.path.join(meta_path, GoldenDBMetaInfo.GOLDENDBINFO)
        if not verify_path_trustlist(output_file):
            raise Exception(f"Invalid src path: {output_file}.")
        if os.path.exists(output_file):
            # 存在，则删除文件
            os.remove(output_file)
        write_file(output_file, cluster_info)
        # 写copyInfo
        resultinfo_name, task_id = (get_copy_result_info(data_path, cluster_structure.cluster_id))
        copy_info = {
            "cluster_id": cluster_structure.cluster_id,
            "resultinfo_name": resultinfo_name,
            "task_id": task_id
        }
        output_file_copy = os.path.join(meta_path, "copy_info")
        if os.path.exists(output_file_copy):
            # 存在，则删除文件
            os.remove(output_file_copy)
        write_file(output_file_copy, json.dumps(copy_info))
        # 将分片数量写入copymetadata.sqlite
        GoldenDBSqliteService.write_metadata_to_sqlite_file(meta_path, str(cluster_structure.group_number))

    @staticmethod
    def exec_umount_sub_job(req_id, job_id, sub_id, file_content):
        log.info(f'Step 5-3: start execute umount sub job.')
        job_infos = file_content.get("subJob", {}).get("jobInfo", " ").strip().split(" ")
        if len(job_infos) != 4:
            log.error(f'Step 5-3: failed to get job info, job_id: {job_id}.')
            return False
        role_name, role_type, sla_policy, _ = job_infos
        backup_params = get_backup_param(req_id, file_content, sla_policy)
        cluster_id = backup_params.cluster_id
        backup_path = get_backup_path(role_name, role_type, file_content, GoldenDBJsonConst.PROTECTOBJECT)
        log.debug(f'Step 5-3: start execute umount sub job.')

        ini_bkp_root_dir = get_bkp_root_dir_via_role(role_type, backup_path, job_id)
        if role_type == GoldenDBNodeType.GTM_NODE:
            prod_bkp_mnt_dir = os.path.join(ini_bkp_root_dir, f'DBCluster_{cluster_id}', 'LOGICAL_BACKUP')
        else:
            prod_bkp_mnt_dir = os.path.join(ini_bkp_root_dir, f'DBCluster_{cluster_id}', 'DATA_BACKUP')

        if not umount_bind_path(prod_bkp_mnt_dir):
            log.error(f"Failed to exec umount bind path for {role_name}, {role_type}.")
            return False

        log.info("Step 5-3:finish execute umount sub job.")
        GoldenDBBackupService.report_complete(job_id, req_id, sub_id, SubJobTypeEnum.BUSINESS_SUB_JOB)
        return True

    @staticmethod
    def report_error_job_details(job_id, req_id, response, sub_id):
        if response.code != GoldenDBCode.SUCCESS.value:
            log.info(f'report_sub_job_details: {response}.')
            log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_FALIED, logInfoParam=[sub_id],
                                   logLevel=DBLogLevel.ERROR.value)
            report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100,
                                                     logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value))

    @staticmethod
    def report_backup_process(report_bkp_proc_parameters):
        file_content = report_bkp_proc_parameters.file_content
        req_id = report_bkp_proc_parameters.req_id
        process = report_bkp_proc_parameters.process
        job_id = report_bkp_proc_parameters.job_id
        original_size = report_bkp_proc_parameters.original_size
        data_path = get_repository_path(file_content, RepositoryDataTypeEnum.DATA_REPOSITORY)
        if not verify_path_trustlist(data_path):
            log.error(f"Invalid src path: {data_path}.")
            return False
        if not os.path.exists(data_path):
            report_job_details(req_id, process)
            return True
        time_file = os.path.join(
            get_repository_path(file_content, RepositoryDataTypeEnum.CACHE_REPOSITORY),
            f'T{job_id}')
        size, speed = query_size_and_speed(time_file, data_path, original_size, job_id)
        process.progress = 50
        process.speed = speed
        file_num = count_files(data_path)
        log_detail = LogDetail(logInfo=ReportDBLabel.BACKUP_SUB_JOB_SUCCESS,
                               logInfoParam=[process.sub_task_id, file_num, format_capacity(size)],
                               logLevel=DBLogLevel.INFO.value)
        process.log_detail = [log_detail]
        report_job_details(req_id, process)
        return True

    @staticmethod
    def exec_backup_cmd(backup_cmd_parameters):
        file_content = backup_cmd_parameters.file_content
        req_id = backup_cmd_parameters.req_id
        sla_policy = backup_cmd_parameters.sla_policy
        role_name = backup_cmd_parameters.role_name
        data_path = backup_cmd_parameters.data_path
        job_id = backup_cmd_parameters.job_id
        sub_id = backup_cmd_parameters.sub_id
        backup_params = get_backup_param(req_id, file_content, sla_policy)
        cluster_id = backup_params.cluster_id
        backup_type = backup_params.backup_type_str
        master_or_slave = backup_params.master_or_slave
        cluster_user = backup_params.cluster_user
        job_infos = file_content.get("subJob", {}).get("jobInfo", " ").strip().split(" ")
        if len(job_infos) != 4:
            log.error(f'Step 5-2: failed to get job info, job_id: {job_id}.')
            return False, 0
        _, role_type, _, agent_id = job_infos
        meta_path = get_repository_path(file_content, RepositoryDataTypeEnum.META_REPOSITORY)
        bkp_sts_file = os.path.join(meta_path, f'{job_id}_exec_backup_status.json')
        backup_path = get_backup_path(role_name, role_type, file_content, GoldenDBJsonConst.PROTECTOBJECT)
        # 从管理节点配置文件中取备份根目录
        prod_bkp_root = get_bkp_root_dir_via_role(role_type, backup_path, job_id)
        if GoldenDBBackupService.bkp_cmd_injection(backup_type, cluster_id, cluster_user, master_or_slave, role_name):
            log.error("Command injection detected in backup command!")
            return False, 0
        # 获取本次备份前，生产端的备份结果文件的文件名
        pre_bkp_files = get_bkp_result_names_in_cluster_id_dir(prod_bkp_root, file_content)
        backup_cmd = f'su - {role_name} -c "dbtool -mds -backup -binlogBackup -c={cluster_id} {backup_type} ' \
                     f'{master_or_slave} -user={cluster_user} -password"'
        # 需要输密码的命令
        status, out_str = exec_cmd_spawn(backup_cmd, req_id)
        log.info(f'result: {(status, out_str)}.')
        if int(status) == int(CMDResult.SUCCESS):
            # 复制活跃事务文件夹至文件系统, 复制备份结果文件至文件系统, 并删除冗余增量文件
            if not GoldenDBBackupService.handle_files_after_bkp(prod_bkp_root, data_path, cluster_id, file_content,
                                                                pre_bkp_files):
                return False, 0
        else:
            log.error(f"Backup failed with result: {(status, out_str)}.")
            return False, 0
        ret, log_detail, log_detail_param, log_info = GoldenDBBackupService.check_res_after_bkp((status, out_str),
                                                                                                job_id, sub_id,
                                                                                                file_content)
        if not ret:
            sts_info = StatusInfo(status=SubJobStatusEnum.FAILED.value, logDetail=log_detail,
                                  logDetailParam=log_detail_param, logInfo=log_info)
            update_agent_sts(agent_id, bkp_sts_file, sts_info)
            return False, 0
        cluster_structure = get_goldendb_structure(file_content)
        GoldenDBBackupService.write_backup_result(cluster_structure, data_path, file_content)
        # 备份后备份目录大小。和备份前记录的文件目录大小对比， 可比较出实际备份的内容大小
        _, size = scan_dir_size(job_id, data_path)
        return True, size

    @staticmethod
    def handle_files_after_bkp(prod_bkp_root, data_path, cluster_id, file_content, pre_bkp_files):
        """
        复制活跃事务文件夹至文件系统, 复制备份结果文件至文件系统, 并删除冗余增量文件
        :param prod_bkp_root: goldendb备份路径
        :param data_path: 文件系统数据仓路径
        :param cluster_id: 集群id
        :param file_content: pm下发参数
        :param pre_bkp_files: 本次备份前，goldendb生产端中所有备份结果文件
        :return:
        """
        cluster_structure = get_goldendb_structure(file_content)
        manager_os_user = cluster_structure.manager_nodes[0][0]
        if not cp_active_folder(prod_bkp_root, data_path, manager_os_user):
            log.error('Copy active tx info folder failed.')
            return False
        # 文件名，有后缀
        bkp_resultinfo_file = GoldenDBBackupService.get_bkp_result_info_file(cluster_id, prod_bkp_root)
        # 将生产端备份结果文件，复制到文件系统数据仓
        if not cp_result_info(cluster_id, bkp_resultinfo_file, prod_bkp_root, data_path):
            log.error('Copy backup result info file failed.')
            return False
        # 删除文件系统冗余文件
        if not GoldenDBBackupService.rm_all_redundant_incr_result_infos(cluster_id, file_content, pre_bkp_files):
            log.error('Delete redundant incremental backup info failed.')
            return False
        return True

    @staticmethod
    def check_res_after_bkp(exec_bkp_outputs, job_id, sub_id, file_content):
        """
        功能描述：备份指令执行后，检查备份结果：1）备份指令回显内容；2）通过备份结果文件内容，检查备份数据是否完整。

        返回值:
        bool, int, list, string:
        检查成功，返回True, None, None, ReportDBLabel.SUB_JOB_SUCCESS；检查失败，返回False，对应的错误码，参数，标签。
        """
        ret, log_detail, log_detail_param, log_info = GoldenDBBackupService.check_bkp_cmd_echo(
            (exec_bkp_outputs[0], exec_bkp_outputs[1]), job_id, sub_id)
        if not ret:
            log.error(f'check_bkp_cmd_echo failed: {exec_bkp_outputs[0]}.')
            return False, log_detail, log_detail_param, log_info

        ret, log_detail, log_detail_param, log_info = GoldenDBBackupService.check_dn_xtream_results(file_content)
        if not ret:
            log.error(f'Backup failed, check_backup_data failed.')
            return False, log_detail, log_detail_param, log_info
        return True, None, None, ReportDBLabel.SUB_JOB_SUCCESS

    @staticmethod
    def bkp_cmd_injection(backup_type, cluster_id, cluster_user, master_or_slave, role_name):
        if check_command_injection_exclude_quote(backup_type):
            log.error("Command injection detected in backup_type!")
            return True
        if check_command_injection_exclude_quote(cluster_id):
            log.error("Command injection detected in cluster_id!")
            return True
        if not cluster_id.isnumeric():
            log.error(f"cluster_id is invalid!")
            return True
        if check_command_injection_exclude_quote(cluster_user):
            log.error("Command injection detected in cluster_user!")
            return True
        if check_command_injection_exclude_quote(master_or_slave):
            log.error("Command injection detected in master_or_slave!")
            return True
        if check_command_injection_exclude_quote(role_name):
            log.error("Command injection detected in role_name!")
            return True
        return False

    @staticmethod
    def rm_all_redundant_incr_result_infos(cluster_id, file_content, pre_bkp_files):
        """
        当前备份完成后，文件系统中会包含生产环境里的所有文件，其中包括本次备份前已存在的文件，需要将其中冗余的增量备份文件删除
        如果本次备份为增量备份，遍历文件系统中的备份结果文件，并删除冗余的增量备份结果文件

        参数:
        cluster_id: 集群ID
        file_content: 本次备份任务下发的结构体，包含了备份类型等信息
        pre_bkp_files: 本次备份前，生产环境中的文件列表

        返回值:
        bool: 如果删除成功，返回True，否则返回False

        异常描述:
        如果路径不在白名单中，会抛出错误
        """
        # 获取数据仓库路径
        data_path = get_repository_path(file_content, RepositoryDataTypeEnum.DATA_REPOSITORY)
        if not verify_path_trustlist(data_path):
            log.error(f"Invalid src path: {data_path}.")
            return False
        # 获取本次备份任务的类型，默认为全量备份
        backup_type = file_content.get("job", {}).get("jobParam", {}).get("backupType", BackupTypeEnum.FULL_BACKUP)
        # 全量备份不需要处理
        if backup_type != BackupTypeEnum.FULL_BACKUP:
            cluster_id_path = os.path.join(data_path, f'DBCluster_{cluster_id}')
            cluster_bkp_result_infos = [f for f in os.listdir(cluster_id_path) if f.startswith(f'{cluster_id}_back')]
            for result_info in cluster_bkp_result_infos:
                # 删除冗余的增量备份文件
                if not GoldenDBBackupService.rm_redundant_incr_result_info(cluster_id_path, result_info, pre_bkp_files):
                    return False
        return True

    @staticmethod
    def rm_redundant_incr_result_info(cluster_id_path, result_info, pre_bkp_files):
        """
        判断输入的文件是否为冗余文件，并删除。
        如果该文件备份前已存在于生产端，且该文件为增量备份任务生成，需要在文件系统中，删除该文件。

        参数:
        cluster_id_path: 文件系统中的集群ID路径
        result_info: 文件系统中的备份结果文件
        pri_bkp_files: 本次备份前，生产环境中的文件列表

        返回值:
        如果删除成功，返回True，否则返回False

        异常描述:
        如果无法备份任务文件夹，或者删除冗余增量备份任务文件夹或结果文件失败，将记录错误日志并返回False
        """
        # 判断该文件是否为本次备份前存在的文件
        if result_info in pre_bkp_files:
            # 获取该备份结果信息在文件系统中的全路径：数据仓/DBCluster_{cid}/{cid}_backup_resultsinfo.{tid}
            result_info_path = os.path.join(cluster_id_path, result_info)
            # 获取当前文件的备份类型
            bkp_file_type = get_bkp_type_from_result_info(result_info_path)
            if bkp_file_type == 'INCREMENTAL':
                # 读文件系统中的备份结果文件，获取对应的增量备份任务（时间戳）路径：数据仓/DBCluster_{cid}/DATA_BACKUP/{tid}
                ret, task_path = get_task_path_from_result_info(result_info_path, cluster_id_path)
                if not ret:
                    log.error(f'Can not backup task folders.')
                    return False
                # 在文件系统中，删除冗余增量备份task id（时间戳）文件夹
                if not su_exec_rm_cmd(task_path):
                    log.error(f'Remove redundant incremental backup task folders failed.')
                    return False
                # 在文件系统中，删除冗余增量备份结果文件
                if not su_exec_rm_cmd(result_info_path):
                    log.error(f'Remove redundant incremental backup info files failed.')
                    return False
        return True
