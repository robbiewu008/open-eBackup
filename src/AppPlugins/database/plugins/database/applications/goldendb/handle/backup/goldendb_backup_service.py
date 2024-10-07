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
    read_tmp_json_file
from common.common_models import SubJobDetails, LogDetail
from common.const import BackupTypeEnum, ExecuteResultEnum, RepositoryDataTypeEnum, SubJobStatusEnum, \
    SubJobPriorityEnum, ReportDBLabel
from common.file_common import get_user_info
from common.util.scanner_utils import scan_dir_size

from goldendb.handle.backup.goldendb_sqlite_service import GoldenDBSqliteService
from goldendb.handle.backup.log.goldendb_log_backup_tool_service import GoldenDBLogBackupToolService
from goldendb.handle.backup.parse_backup_params import get_goldendb_structure, get_master_or_slave_policy, \
    check_goldendb_structure, report_job_details, write_file, query_size_and_speed, \
    exec_cmd_spawn, get_backup_param, get_copy_result_info, get_agent_uuids
from goldendb.handle.common.const import ErrorCode, GoldenDBMetaInfo, LogLevel, SubJobType, GoldenDBCode, \
    MasterSlavePolicy, ExecutePolicy, SubJobName, GoldenDBJsonConst, CMDResult, Report, GoldenDBNodeType
from goldendb.handle.common.goldendb_common import cp_active_folder, cp_result_info, get_backup_path, count_files, \
    get_bkp_type_from_result_info, get_bkp_files_in_cluster_id_folder, mount_bind_path, verify_path_trustlist, \
    umount_bind_path, get_repository_path, mkdir_chmod_chown_dir_recursively, umount_bind_backup_paths, \
    get_task_path_from_result_info, format_capacity
from goldendb.handle.common.goldendb_param import JsonParam
from goldendb.handle.resource.resource_info import GoldenDBResourceInfo
from goldendb.logger import log
from goldendb.schemas.glodendb_schemas import ActionResponse, SubJob

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
        log.info(
            f'step 1: execute allow_backup_in_local_node, req_id: {req_id}, job_id: {job_id}, sub_job_id: {sub_job_id}')
        response = ActionResponse()
        try:
            file_content = JsonParam.parse_param_with_jsonschema(req_id)
        except Exception as exception:
            log.error(exception, exc_info=True)
            return
        master_or_slave = get_master_or_slave_policy(file_content)
        cluster_structure = get_goldendb_structure(file_content)
        if master_or_slave == MasterSlavePolicy.SLAVE and not check_goldendb_structure(master_or_slave,
                                                                                       cluster_structure):
            log_detail = LogDetail(logInfo="goldendb_backup_pre_check_sla_failed_label", logLevel=LogLevel.WARN)
            report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_job_id, progress=100,
                                                     logDetail=[log_detail], taskStatus=SubJobStatusEnum.RUNNING.value))
            master_or_slave = MasterSlavePolicy.MASTER

        agent_uuids = get_agent_uuids(file_content)
        # 执行节点添加管理节点首节点，当前搭建环境只有单个管理节点
        exec_nodes = [cluster_structure.manager_nodes[0]]
        # 添加GTM所有节点
        exec_nodes.extend(cluster_structure.gtm_nodes)
        # 执行节点执行数据节点
        for group in cluster_structure.data_nodes.values():
            if master_or_slave == MasterSlavePolicy.MASTER:
                exec_nodes.append(group[MasterSlavePolicy.MASTER][0])
            else:
                exec_nodes.extend(group[MasterSlavePolicy.SLAVE])
        for exec_node in exec_nodes:
            agent_uuid = exec_node[2]
            if agent_uuid not in agent_uuids:
                response = ActionResponse(code=ExecuteResultEnum.INTERNAL_ERROR,
                                          bodyErr=ErrorCode.ERR_ENVIRONMENT,
                                          message=f"{agent_uuid} is offline")
                log_detail = LogDetail(logInfo="plugin_generate_subjob_fail_label", logLevel=LogLevel.ERROR)
                report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_job_id, progress=100,
                                                         logDetail=[log_detail],
                                                         taskStatus=SubJobStatusEnum.FAILED.value))
                output_result_file(req_id, response.dict(by_alias=True))
                return
        output_result_file(req_id, response.dict(by_alias=True))
        log.info(f"step 1: execute AllowBackupInLocalNode interface success")

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
            results_info_file = GoldenDBBackupService.get_file_name(cluster_id, data_path)
            if not results_info_file:
                log.error(f"results_info_file not exist please check copy")
                return False
            # 检查meta中记录的上次备份副本集群结构与当前集群结构是否相同
            goldendb_info_file = os.path.join(meta_path, GoldenDBMetaInfo.GOLDENDBINFO)
            if not os.path.exists(goldendb_info_file):
                log.error(f"goldendb_info_file file: {goldendb_info_file} not exist please check copy")
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
                log.error(f"goldendb_info change")
                return False
            return True

        log.info(f'step 2-1: start execute check_backup_job_type, pid: {req_id}, job_id:{job_id}')
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
            response = ActionResponse(code=ExecuteResultEnum.INTERNAL_ERROR,
                                      bodyErr=ErrorCode.ERROR_INCREMENT_TO_FULL,
                                      message="Can not apply this type backup job")
            output_result_file(req_id, response.dict(by_alias=True))
            log.info(f'step 2-1: finish execute check_backup_job_type filed, pid: {req_id}, job_id:{job_id}')
            return False
        response = ActionResponse(code=ExecuteResultEnum.SUCCESS)
        output_result_file(req_id, response.dict(by_alias=True))
        log.info(f'step 2-1: finish execute check_backup_job_type, pid: {req_id}, job_id:{job_id}')
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
        log.info(f'step 3: execute backup_pre_job,req_id:{req_id} job_id:{job_id}')
        response = ActionResponse()
        try:
            JsonParam.parse_param_with_jsonschema(req_id)
        except Exception as exception:
            log.error(exception, exc_info=True)
            GoldenDBBackupService.set_error_response(response)
            output_result_file(req_id, response.dict(by_alias=True))
            return False
        output_result_file(req_id, response.dict(by_alias=True))
        GoldenDBBackupService.report_complete(job_id, req_id, "", 0)
        log.info(f'step 3: finish to execute backup_pre_job,req_id:{req_id} job_id:{job_id}')
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
            f'start execute backup_prerequisite_job_progress, req_id: {req_id}, job_id: {job_id},  sub_id: {sub_id}')
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
        log.info(f'step 4: start to execute backup_gen_sub_job, req_id: {req_id}, job_id: {job_id}')
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
                response = ActionResponse()
                GoldenDBBackupService.set_error_response(response)
                GoldenDBBackupService.report_error_job_details(job_id, req_id, response, job_id)
        else:
            GoldenDBBackupService.full_and_inc_gen_sub_jobs(file_content, job_id, req_id)

    @staticmethod
    def full_and_inc_gen_sub_jobs(file_content, job_id, req_id):
        response = []
        master_or_slave = get_master_or_slave_policy(file_content)  # return master or slave
        cluster_structure = get_goldendb_structure(file_content)
        if master_or_slave == MasterSlavePolicy.SLAVE and not check_goldendb_structure(master_or_slave,
                                                                                       cluster_structure):
            log_detail = LogDetail(logInfo="goldendb_backup_pre_check_sla_failed_label", logLevel=LogLevel.WARN)
            report_job_details(req_id, SubJobDetails(taskId=job_id, progress=100, logDetail=[log_detail],
                                                     taskStatus=SubJobStatusEnum.RUNNING.value))
            master_or_slave = MasterSlavePolicy.MASTER
        # 执行节点添加管理节点首节点，当前搭建环境只有单个管理节点
        exec_nodes = [cluster_structure.manager_nodes[0]]
        # 添加GTM所有节点
        exec_nodes.extend(cluster_structure.gtm_nodes)
        # 执行节点执行数据节点
        for group in cluster_structure.data_nodes.values():
            if master_or_slave == MasterSlavePolicy.MASTER:
                exec_nodes.append(group[MasterSlavePolicy.MASTER][0])
            else:
                exec_nodes.extend(group[MasterSlavePolicy.SLAVE])

        # 向本次任务所有执行节点下发子任务
        for exec_node in exec_nodes:
            user_name = exec_node[0]
            agent_id = exec_node[2]
            goldendb_node_type = exec_node[3]
            job_info = f"{user_name} {goldendb_node_type} {master_or_slave}"
            response.append(
                SubJob(jobId=job_id, execNodeId=agent_id, jobName=SubJobName.MOUNT, jobInfo=job_info,
                       jobPriority=SubJobPriorityEnum.JOB_PRIORITY_1).dict(by_alias=True))
            if goldendb_node_type == GoldenDBNodeType.ZX_MANAGER_NODE:
                response.append(
                    SubJob(jobId=job_id, execNodeId=agent_id, jobName=SubJobName.EXEC_BACKUP,
                           jobInfo=job_info,
                           jobPriority=SubJobPriorityEnum.JOB_PRIORITY_2).dict(by_alias=True))
        # 如果没有子任务，则报错
        if not response:
            log.error(f'job id: {job_id}, generate zero sub job')
            raise Exception(f'job id: {job_id}, generate zero sub job')
        # 加入查询信息子任务，不添加的话不会调用queryCopy方法，不会上报给UBC
        response.append(
            SubJob(jobId=job_id, policy=ExecutePolicy.ANY_NODE, jobName='queryCopy',
                   jobPriority=SubJobPriorityEnum.JOB_PRIORITY_4).dict(by_alias=True))
        log.info(f'step 4: finish to execute backup_gen_sub_job, req_id: {req_id}, job_id: {job_id}')
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
        response = ActionResponse()
        log.info(f'step 5: execute backup, req_id: {req_id}, job_id: {job_id},sub_id:{sub_id}')
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
            sub_job_name = file_content["subJob"]["jobName"]
            role_name, role_type, sla_policy = file_content["subJob"]["jobInfo"].split(" ")
            data_path = get_repository_path(file_content, RepositoryDataTypeEnum.DATA_REPOSITORY)
            if not verify_path_trustlist(data_path):
                log.error(f"Invalid src path: {data_path}.")
                return
            if sub_job_name == SubJobName.MOUNT:
                mount_parameters = MountParameters(req_id, job_id, sub_id, data_path, file_content)
                if not GoldenDBBackupService.exec_mount_sub_job(mount_parameters):
                    log.error("Failed to mount bind backup path")
                    GoldenDBBackupService.set_error_response(response)
            elif sub_job_name == SubJobName.EXEC_BACKUP:
                bkp_sub_parameters = BackupSubJobParameters(req_id, role_name, file_content, job_id, sub_id, sla_policy)
                if not GoldenDBBackupService.exec_backup_sub_job(bkp_sub_parameters):
                    log.error("Failed to exec backup sub job")
                    GoldenDBBackupService.set_error_response(response)

            output_result_file(req_id, response.dict(by_alias=True))
            GoldenDBBackupService.report_error_job_details(job_id, req_id, response, sub_id)

    @staticmethod
    def backup_post_job(req_id, job_id):
        """
        功能描述：执行备份后置任务任务
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        返回值：
        """
        log.info(f'step 7: start execute backup_post_job, req_id: {req_id}, job_id: {job_id}')
        umount_bind_backup_paths(job_id)

        response = ActionResponse()
        output_result_file(req_id, response.dict(by_alias=True))
        log.info(f'step 7: finish execute backup_post_job, req_id: {req_id}, job_id: {job_id}')

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
        log.info(f'execute backup_post_job_progress, req_id: {req_id}, job_id: {job_id},  sub_id: {sub_id}')
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
        log.info(f'step 6: execute to query_backup_copy, pid: {req_id}, job_id: {job_id}')
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
        if not sub_id and job_type == SubJobType.PRE_SUB_JOB:
            log_detail = LogDetail(logInfo="plugin_execute_prerequisit_task_success_label", logLevel=LogLevel.INFO)
            report_job_details(req_id,
                               SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, logDetail=[log_detail],
                                             taskStatus=SubJobStatusEnum.COMPLETED.value))
        elif job_type == SubJobType.BUSINESS_SUB_JOB:
            log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS, logInfoParam=[sub_id], logLevel=LogLevel.INFO)
            report_job_details(req_id,
                               SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, logDetail=[log_detail],
                                             taskStatus=SubJobStatusEnum.COMPLETED.value))

    @staticmethod
    def set_error_response(response):
        response.code = GoldenDBCode.FAILED.value
        response.body_err = GoldenDBCode.FAILED.value

    @staticmethod
    def exec_mount_sub_job(mount_parameters):
        log.info(f'step 5-1: start execute mount sub job')
        file_content = mount_parameters.file_content
        data_path = mount_parameters.data_path
        sub_id = mount_parameters.sub_id
        req_id = mount_parameters.req_id
        job_id = mount_parameters.job_id
        role_name, role_type, _ = file_content["subJob"]["jobInfo"].split(" ")
        backup_path = get_backup_path(role_name, role_type, file_content, GoldenDBJsonConst.PROTECTOBJECT)
        goldendb_structure = get_goldendb_structure(file_content)
        cluster_id = goldendb_structure.cluster_id
        data_path_bkp = os.path.join(data_path, f'DBCluster_{cluster_id}', 'DATA_BACKUP')
        backup_path_bkp = os.path.join(backup_path, f'DBCluster_{cluster_id}', 'DATA_BACKUP')
        # 解挂载备份根data_backup目录
        umount_bind_path(backup_path_bkp)
        group_name, _ = get_user_info(role_name)
        if not mkdir_chmod_chown_dir_recursively(data_path_bkp, 0o770, role_name, group_name, True):
            log.error("fail to make a data path.")
            return False
        if not mkdir_chmod_chown_dir_recursively(backup_path_bkp, 0o770, role_name, group_name, True):
            log.error("fail to make  a back path.")
            return False
        if not mount_bind_path(data_path_bkp, backup_path_bkp, job_id):
            log.error(f"Mount data_path failed")
            return False
        log.info("step 5-1:finish Mount bind all backup path success")
        log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS, logInfoParam=[sub_id], logLevel=LogLevel.INFO)
        report_job_details(req_id,
                           SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, logDetail=[log_detail],
                                         taskStatus=SubJobStatusEnum.COMPLETED.value))
        return True

    @staticmethod
    def exec_backup_sub_job(backup_parameters):
        job_id, sub_id, req_id, file_content, role_name, sla_policy = (
            backup_parameters.job_id, backup_parameters.sub_id, backup_parameters.req_id,
            backup_parameters.file_content, backup_parameters.role_name, backup_parameters.sla_policy
        )
        log.info(f'step 5-2: start to execute backup, job_id:{job_id},sub_id:{sub_id},req_id:{req_id}')
        # 获取集群版本
        version = GoldenDBResourceInfo.get_cluster_version(role_name)
        cache_path = get_repository_path(file_content, RepositoryDataTypeEnum.CACHE_REPOSITORY)
        version_file = os.path.join(cache_path, f"bkp_{job_id}_version.json")
        output_execution_result_ex(version_file, {"version": version})
        data_path = get_repository_path(file_content, RepositoryDataTypeEnum.DATA_REPOSITORY)
        if not verify_path_trustlist(data_path):
            log.error(f"Invalid src path: {data_path}.")
            return False
        original_size = 0
        ret, size = scan_dir_size(job_id, data_path)
        if ret:
            original_size = size
        process = SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=0, taskStatus=SubJobStatusEnum.RUNNING)
        log_detail = LogDetail(logInfo=ReportDBLabel.BACKUP_SUB_START_COPY, logInfoParam=[process.sub_task_id],
                               logLevel=LogLevel.INFO)
        process.log_detail = [log_detail]
        report_job_details(req_id, process)
        # 清空上报进度
        process.log_detail = None
        pool = ThreadPoolExecutor(max_workers=1, thread_name_prefix='goldendb-backup')
        message = str(int((time.time())))
        write_file(os.path.join(
            get_repository_path(file_content, RepositoryDataTypeEnum.CACHE_REPOSITORY), f'T{job_id}'), message)

        backup_cmd_parameters = BackupCmdParameters(req_id, role_name, file_content, sla_policy, job_id, sub_id,
                                                    data_path)
        copy_feature = pool.submit(GoldenDBBackupService.exec_backup_cmd, backup_cmd_parameters)
        while not copy_feature.done():
            time.sleep(Report.REPORT_INTERVAL)
            report_bkp_proc_parameters = ReportBkpProcParameters(req_id, job_id, process, file_content, original_size)
            if not GoldenDBBackupService.report_backup_process(report_bkp_proc_parameters):
                log.error(f'full backup failed: backup root error')
                pool.shutdown()
                return False
        if not copy_feature.result()[0]:
            return False
        data_size = copy_feature.result()[1] - original_size
        process.data_size = data_size
        process.task_status = SubJobStatusEnum.COMPLETED
        process.progress = 100
        log_detail = LogDetail(logInfo=ReportDBLabel.BACKUP_SUB_JOB_SUCCESS,
                               logInfoParam=[process.sub_task_id, count_files(data_path),
                                             format_capacity(copy_feature.result()[1])], logLevel=LogLevel.INFO)
        process.log_detail = [log_detail]
        report_job_details(req_id, process)
        log.info(f'step 5-2:finish  start to execute backup, job_id:{job_id},sub_id:{sub_id},req_id:{req_id}')
        return True

    @staticmethod
    def check_backup_result(copy_feature, req_id, job_id, sub_id, action="Backup"):
        if int(copy_feature[0]) != int(CMDResult.SUCCESS):
            if "response message: " in copy_feature[1]:
                message = copy_feature[1].split("response message: ")[1].replace("\\r\\n", " ").rstrip(
                    "'").strip()
                log.debug(f"message :{message}")
                log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_FALIED, logInfoParam=[sub_id],
                                       logLevel=LogLevel.ERROR, logDetail=ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL,
                                       logDetailParam=[action, message])
            else:
                log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_FALIED, logInfoParam=[sub_id],
                                       logLevel=LogLevel.ERROR, logDetail=0)
            report_job_details(req_id,
                               SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, logDetail=[log_detail],
                                             taskStatus=SubJobStatusEnum.FAILED.value))
            return False
        return True

    @staticmethod
    def check_backup_data(file_content):
        # 检查备份数据是否完整
        data_path = get_repository_path(file_content, RepositoryDataTypeEnum.DATA_REPOSITORY)
        if not verify_path_trustlist(data_path):
            log.error(f"Invalid src path: {data_path}.")
            return False
        goldendb_structure = get_goldendb_structure(file_content)
        cluster_id = goldendb_structure.cluster_id
        results_info_file = GoldenDBBackupService.get_file_name(cluster_id, data_path)
        if not results_info_file:
            log.error(f'check_backup_data results_info_path no backup_resultsinfo.')
            return False
        results_info_path = os.path.join(data_path, f"DBCluster_{cluster_id}", results_info_file)
        if os.path.exists(results_info_path):
            with open(results_info_path, encoding='utf-8') as file:
                # 读取文件
                contents = file.readlines()
        else:
            log.error(f'check_backup_data results_info_path ')
            return False
        log.debug(f'check_backup_data contents :{contents}')
        record_info_list = []
        for record in contents:
            record_info = record.split()
            record_info_list.append(record_info)
        log.debug(f'check_backup_data record_info_list :{record_info_list}')
        for record_info in record_info_list:
            if not GoldenDBBackupService.check_backup_path_exit(record_info, data_path):
                return False
        return True

    @staticmethod
    def get_file_name(cluster_id, data_path):
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
    def check_backup_path_exit(record_info, data_path):
        if int(record_info[0]) == 3:
            xbstream_list = list(filter(lambda x: "/backup_root/" in x, record_info))
            if xbstream_list:
                xbstream_path = os.path.join(data_path, xbstream_list[0].split('backup_root')[1][1:])
                if not verify_path_trustlist(xbstream_path):
                    log.error(f"Invalid src path: {xbstream_path}.")
                    return False
                if not os.path.exists(xbstream_path):
                    log.error(f'check_backup_data xbstream_path not exist: {xbstream_path}')
                    return False
        return True

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
        log.info(f'step 5-3: start execute umount sub job')
        role_name, role_type, sla_policy = file_content["subJob"]["jobInfo"].split(" ")
        backup_params = get_backup_param(req_id, file_content, sla_policy)
        cluster_id = backup_params.cluster_id
        backup_path = get_backup_path(role_name, role_type, file_content, GoldenDBJsonConst.PROTECTOBJECT)
        log.debug(f'step 5-3: start execute umount sub job')

        if not umount_bind_path(os.path.join(backup_path, f'DBCluster_{cluster_id}', 'DATA_BACKUP')):
            log.error(f"Failed to exec umount bind path")
            return False
        log.info("step 5-3:finish execute umount sub job")
        log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS, logInfoParam=[sub_id], logLevel=LogLevel.INFO)
        report_job_details(req_id,
                           SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, logDetail=[log_detail],
                                         taskStatus=SubJobStatusEnum.COMPLETED.value))
        return True

    @staticmethod
    def report_error_job_details(job_id, req_id, response, sub_id):
        if response.code != GoldenDBCode.SUCCESS.value:
            log.info(f'report_sub_job_details:{response}')
            log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_FALIED, logInfoParam=[sub_id], logLevel=LogLevel.ERROR)
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
                               logLevel=LogLevel.INFO)
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
        _, role_type, _ = file_content["subJob"]["jobInfo"].split(" ")
        backup_path = get_backup_path(role_name, role_type, file_content, GoldenDBJsonConst.PROTECTOBJECT)

        if GoldenDBBackupService.bkp_cmd_injection(backup_type, cluster_id, cluster_user, master_or_slave, role_name):
            log.error("command injection detected in backup command!")
            return False, 0
        pre_bkp_files = get_bkp_files_in_cluster_id_folder(backup_path, file_content)
        backup_cmd = f'su - {role_name} -c "dbtool -mds -backup -binlogBackup -c={cluster_id} {backup_type} ' \
                     f'{master_or_slave} -user={cluster_user} -password"'
        # 需要输密码的命令
        status, out_str = exec_cmd_spawn(backup_cmd, req_id)
        log.info(f'result: {(status, out_str)}')
        # 复制活跃事务文件夹至文件系统, 复制备份结果文件至文件系统, 并删除冗余增量文件
        if not GoldenDBBackupService.handle_files_after_bkp(backup_path, data_path, cluster_id, file_content,
                                                            pre_bkp_files):
            return False, 0
        if not GoldenDBBackupService.check_res_after_bkp((status, out_str), req_id, job_id, sub_id, file_content):
            return False, 0
        if not verify_path_trustlist(data_path):
            log.error(f"Invalid src path: {data_path}.")
            return False, 0
        cluster_structure = get_goldendb_structure(file_content)
        GoldenDBBackupService.write_backup_result(cluster_structure, data_path, file_content)
        # 备份后备份目录大小。和备份前记录的文件目录大小对比， 可比较出实际备份的内容大小
        _, size = scan_dir_size(job_id, data_path)
        return True, size

    @staticmethod
    def handle_files_after_bkp(backup_path, data_path, cluster_id, file_content, pre_bkp_files):
        """
        复制活跃事务文件夹至文件系统, 复制备份结果文件至文件系统, 并删除冗余增量文件
        :param backup_path: goldendb备份路径
        :param data_path: 文件系统路径
        :param cluster_id: 集群id
        :param file_content: pm下发参数
        :param pre_bkp_files: 本次备份前，goldendb中的所有备份结果文件
        :return:
        """
        cluster_structure = get_goldendb_structure(file_content)
        manager_os_user = cluster_structure.manager_nodes[0][0]
        if not cp_active_folder(backup_path, data_path, manager_os_user):
            log.error('copy active tx info folder failed')
            return False
        bkp_resultinfo_file = GoldenDBBackupService.get_file_name(cluster_id, backup_path)
        if not cp_result_info(cluster_id, bkp_resultinfo_file, backup_path, data_path):
            log.error('copy backup result info file failed')
            return False
        if not GoldenDBBackupService.delete_redundant_inc_folders(cluster_id, file_content, pre_bkp_files):
            log.error('delete redundant incremental backup info failed')
            return False
        return True

    @staticmethod
    def check_res_after_bkp(exec_bkp_outputs, req_id, job_id, sub_id, file_content):
        if not GoldenDBBackupService.check_backup_result((exec_bkp_outputs[0], exec_bkp_outputs[1]), req_id, job_id,
                                                         sub_id):
            log.error(f'check_backup_result failed: {exec_bkp_outputs[0]}')
            return False
        if not GoldenDBBackupService.check_backup_data(file_content):
            log.error(f'backup failed, check_backup_data failed')
            return False
        return True

    @staticmethod
    def bkp_cmd_injection(backup_type, cluster_id, cluster_user, master_or_slave, role_name):
        if check_command_injection_exclude_quote(backup_type):
            log.error("command injection detected in backup_type!")
            return True
        if check_command_injection_exclude_quote(cluster_id):
            log.error("command injection detected in cluster_id!")
            return True
        if not cluster_id.isnumeric():
            log.error(f"cluster_id is invalid!")
            return True
        if check_command_injection_exclude_quote(cluster_user):
            log.error("command injection detected in cluster_user!")
            return True
        if check_command_injection_exclude_quote(master_or_slave):
            log.error("command injection detected in master_or_slave!")
            return True
        if check_command_injection_exclude_quote(role_name):
            log.error("command injection detected in role_name!")
            return True
        return False

    @staticmethod
    def delete_redundant_inc_folders(cluster_id, file_content, pri_bkp_files):
        data_path = get_repository_path(file_content, RepositoryDataTypeEnum.DATA_REPOSITORY)
        if not verify_path_trustlist(data_path):
            log.error(f"Invalid src path: {data_path}.")
            return False
        backup_type = file_content.get("job", {}).get("jobParam", {}).get("backupType", BackupTypeEnum.FULL_BACKUP)
        if backup_type != BackupTypeEnum.FULL_BACKUP:
            cluster_id_path = os.path.join(data_path, f'DBCluster_{cluster_id}')
            cluster_bkp_result_infos = [f for f in os.listdir(cluster_id_path) if f.startswith(f'{cluster_id}_back')]
            for result_info in cluster_bkp_result_infos:
                if not GoldenDBBackupService.remove_redundant_files(cluster_id_path, result_info, pri_bkp_files):
                    return False
        return True

    @staticmethod
    def remove_redundant_files(cluster_id_path, result_info, pri_bkp_files):
        if result_info in pri_bkp_files:
            result_info_path = os.path.join(cluster_id_path, result_info)
            bkp_type = get_bkp_type_from_result_info(result_info_path)
            if bkp_type == 'INCREMENTAL':
                ret, task_path = get_task_path_from_result_info(result_info_path, cluster_id_path)
                if not ret:
                    log.error(f'can not backup task folders.')
                    return False
                # 删除冗余增量备份task id文件夹
                if not su_exec_rm_cmd(task_path):
                    log.error(f'remove redundant incremental backup task folders failed.')
                    return False
                # 删除冗余增量备份结果文件
                if not su_exec_rm_cmd(result_info_path):
                    log.error(f'remove redundant incremental backup info files failed.')
                    return False
        return True
