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

from common.util.exec_utils import su_exec_rm_cmd, exec_mkdir_cmd
from common.common import check_command_injection_exclude_quote, output_result_file, output_execution_result_ex, \
    read_tmp_json_file, report_job_details, execute_cmd
from common.common_models import SubJobDetails, SubJobModel, LogDetail, ActionResult, RepositoryPath, ScanRepositories
from common.const import BackupTypeEnum, ExecuteResultEnum, RepositoryDataTypeEnum, SubJobStatusEnum, ReportDBLabel, \
    DBLogLevel, CMDResult, RepositoryNameEnum
from common.file_common import get_user_info
from common.job_const import JobNameConst
from common.util.scanner_utils import scan_dir_size

from goldendb.handle.backup.exec_backup import GoldenDBDataBackupToolService
from goldendb.handle.backup.goldendb_sqlite_service import GoldenDBSqliteService
from goldendb.handle.backup.log.goldendb_log_backup_tool_service import GoldenDBLogBackupToolService
from goldendb.handle.backup.parse_backup_params import get_goldendb_structure, get_master_or_slave_policy, write_file, \
    check_goldendb_structure, query_size_and_speed, exec_cmd_spawn, get_backup_param, get_copy_result_info, \
    get_backup_param_for_cm_backup, get_bkp_task_id, get_bkp_result_info_file
from goldendb.handle.common.const import ErrorCode, GoldenDBMetaInfo, Report, SubJobName, ErrPattern, GoldendbLabel, \
    MasterSlavePolicy, GoldenDBJsonConst, GoldenDBNodeType
from goldendb.handle.common.goldendb_common import cp_active_folder, cp_result_info, get_backup_path, count_files, \
    get_bkp_type_from_result_info, get_bkp_result_names_in_cluster_id_dir, mount_bind_path, verify_path_trustlist, \
    umount_bind_path, get_repository_path, mkdir_chmod_chown_dir_recursively, umount_bind_backup_paths, \
    get_task_path_from_result_info, format_capacity, check_task_on_all_managers, get_agent_uuids, update_agent_sts, \
    get_etc_ini_path, report_action_result, get_recognized_err_from_sts_file, exec_task_on_all_managers, \
    update_agent_sts_general_after_exec, report_subjob_via_rpc, check_exec_on_cm, check_dbtool_start, \
    get_err_log_detail_from_echo, get_dbtool_cm_result
from goldendb.handle.common.goldendb_param import JsonParam
from goldendb.handle.resource.resource_info import GoldenDBResourceInfo
from goldendb.logger import log
from goldendb.schemas.glodendb_schemas import TaskInfo, StatusInfo

MountParameters = collections.namedtuple('MountParameters', ['req_id', 'job_id', 'sub_id', 'data_path', 'file_content'])

BackupSubJobParameters = collections.namedtuple('BackupSubJobParameters',
                                                ['req_id', 'role_name', 'role_type', 'file_content', 'job_id', 'sub_id',
                                                 'sla_policy'])

ReportBkpProcParameters = collections.namedtuple('ReportBkpProcParameters',
                                                 ['req_id', 'job_id', 'sub_id', 'process', 'file_content',
                                                  'original_size', 'original_cnts'])

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
        try:
            file_content = JsonParam.parse_param_with_jsonschema(req_id)
        except Exception as exception:
            log.error(exception, exc_info=True)
            return
        master_or_slave = get_master_or_slave_policy(file_content)
        cluster_structure = get_goldendb_structure(file_content)
        if master_or_slave == MasterSlavePolicy.SLAVE and not check_goldendb_structure(master_or_slave,
                                                                                       cluster_structure):
            log_detail = LogDetail(logInfo=GoldendbLabel.pre_check_sla, logLevel=DBLogLevel.WARN.value)
            report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_job_id, progress=100,
                                                     logDetail=[log_detail], taskStatus=SubJobStatusEnum.RUNNING.value))
            master_or_slave = MasterSlavePolicy.MASTER

        agent_uuids = get_agent_uuids(file_content)
        # 检查管理节点
        no_manager = all([node[2] not in agent_uuids for node in cluster_structure.manager_nodes])
        if no_manager:
            report_action_result(req_id, job_id, sub_job_id, False, "Step 1: manager nodes all offline")
            return
        log.info(f'Step 1: get manager nodes success, req_id: {req_id}, job_id: {job_id}, sub_job_id: {sub_job_id}.')
        # 检查GTM节点
        no_gtm = all([node[2] not in agent_uuids for node in cluster_structure.gtm_nodes])
        if no_gtm:
            report_action_result(req_id, job_id, sub_job_id, False, "Step 1: gtm nodes all offline")
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
            report_action_result(req_id, job_id, sub_job_id, False, "Step 1: data nodes all offline")
            return
        report_action_result(req_id, job_id, sub_job_id, True, "Step 1: AllowBackupInLocalNode success")
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
            log.info(f'Step 2-1: No check_backup_job_type for full backup, pid: {req_id}, job_id: {job_id}.')
            return True
        goldendb_backup_tool = GoldenDBDataBackupToolService(file_content=file_content, job_id=job_id, pid=req_id)
        if not goldendb_backup_tool.check_backup_type():
            response = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                    bodyErr=ErrorCode.ERROR_INCREMENT_TO_FULL,
                                    message="Can not apply this type backup job")
            output_result_file(req_id, response.dict(by_alias=True))
            log.error(f'Step 2-1: finish execute check_backup_job_type failed, pid: {req_id}, job_id: {job_id}.')
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
        ret = True
        try:
            JsonParam.parse_param_with_jsonschema(req_id)
        except Exception as exception:
            ret = False
            log.error(exception, exc_info=True)
        log_detail = LogDetail(logInfo=ReportDBLabel.PRE_REQUISIT_SUCCESS) if ret else LogDetail(
            logInfo=ReportDBLabel.PRE_REQUISIT_FAILED, log_detail=ErrorCode.ERROR_INTERNAL)
        report_subjob_via_rpc(req_id, SubJobModel(jobId=job_id), log_detail)
        log.info(f'Step 3: finish to execute backup_pre_job, req_id: {req_id} job_id: {job_id}.')
        return ret

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
            report_action_result(req_id, job_id, "", False, "Step 4, gen sub job failed")
            return

        backup_type = file_content.get("job", {}).get("jobParam", {}).get("backupType", BackupTypeEnum.FULL_BACKUP)
        if backup_type == BackupTypeEnum.LOG_BACKUP:
            try:
                backup_tool = GoldenDBLogBackupToolService(file_content=file_content, job_id=job_id, req_id=req_id)
                backup_tool.gen_sub_jobs()
            except Exception as exception:
                log.error(exception, exc_info=True)
                report_action_result(req_id, job_id, "", False, f"Step 4, {backup_type} gen_sub_job failed")
        else:
            try:
                backup_tool = GoldenDBDataBackupToolService(file_content=file_content, job_id=job_id, pid=req_id)
                backup_tool.gen_sub_jobs()
            except Exception as exception:
                log.error(exception, exc_info=True)
                report_action_result(req_id, job_id, "", False, f"Step 4, {backup_type} gen_sub_job failed")

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
        sub_job_model = SubJobModel(jobId=job_id, subJobId=sub_id)
        log.info(f'Step 5: execute backup, req_id: {req_id}, job_id: {job_id},sub_id:{sub_id}.')
        try:
            file_content = JsonParam.parse_param_with_jsonschema(req_id)
        except Exception as exception:
            log.error(exception, exc_info=True)
            report_subjob_via_rpc(req_id, sub_job_model, LogDetail())
            return

        backup_type = file_content.get("job", {}).get("jobParam", {}).get("backupType", BackupTypeEnum.FULL_BACKUP)
        if backup_type == BackupTypeEnum.LOG_BACKUP:
            try:
                goldendb_backup_tool = GoldenDBLogBackupToolService(file_content=file_content, job_id=job_id,
                                                                    req_id=req_id, sub_id=sub_id)
                goldendb_backup_tool.backup()
            except Exception as exception:
                log.error(exception, exc_info=True)
                report_subjob_via_rpc(req_id, sub_job_model, LogDetail())
        else:
            try:
                GoldenDBBackupService.exec_data_bkp_sub_with_report(req_id, job_id, sub_id, file_content)
            except Exception as exception:
                log.error(exception, exc_info=True)
                report_subjob_via_rpc(req_id, sub_job_model, LogDetail())

    @staticmethod
    def exec_data_bkp_sub_with_report(req_id, job_id, sub_id, file_content):
        # 获取管理节点备份状态文件
        meta_path = get_repository_path(file_content, RepositoryDataTypeEnum.META_REPOSITORY)
        bkp_sts_file = os.path.join(meta_path, f'{job_id}_exec_backup_status.json')
        sub_job_name = file_content.get("subJob", {}).get("jobName")
        job_infos = file_content.get("subJob", {}).get("jobInfo").strip().split(" ")
        sub_job_model = SubJobModel(jobId=job_id, subJobId=sub_id)
        # 检查参数
        if len(job_infos) != 4:
            log.error(f'Step 5: failed to get job info, job_id: {job_id}.')
            report_subjob_via_rpc(req_id, sub_job_model, LogDetail())
            return
        role_name, role_type, sla_policy, agent_id = job_infos
        data_path = get_repository_path(file_content, RepositoryDataTypeEnum.DATA_REPOSITORY)
        if not verify_path_trustlist(data_path):
            log.error(f"Step 5: invalid src path, {data_path}, job_id: {job_id},sub_id:{sub_id}.")
            report_subjob_via_rpc(req_id, sub_job_model, LogDetail())
            return
        log.info(f"Step 5: start {sub_job_name} on {agent_id}, req_id: {req_id}, job_id: {job_id}, sub_id: {sub_id}.")
        # 执行子任务
        if sub_job_name == SubJobName.MOUNT:
            mount_parameters = MountParameters(req_id, job_id, sub_id, data_path, file_content)
            ret, key_log_details = GoldenDBBackupService.exec_mount_sub_job(mount_parameters)
            if ret:
                log.info(f"Step 5-1: mount success, req_id: {req_id}, job_id: {job_id}, sub_id: {sub_id}.")
                report_subjob_via_rpc(req_id, sub_job_model, LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS))
            else:
                log.error(f"Step 5: mount failed, req_id: {req_id}, job_id: {job_id}, sub_id: {sub_id}.")
                report_subjob_via_rpc(req_id, sub_job_model, key_log_details)
        elif sub_job_name == SubJobName.EXEC_BACKUP:
            bkp_sub_parameters = BackupSubJobParameters(req_id, role_name, role_type, file_content, job_id, sub_id,
                                                        sla_policy)
            if not GoldenDBBackupService.exec_backup_sub_job(bkp_sub_parameters):
                log.error(f"Step 5: failed to exec backup sub job on {agent_id}, job_id: {job_id}.")
                ret = GoldenDBBackupService.check_backup_on_all_managers(agent_id, bkp_sts_file, job_id, req_id,
                                                                         sub_id)
                if ret:
                    log.info(f"Step 5: check backup sub job on all managers success, job_id: {job_id}.")
                    report_subjob_via_rpc(req_id, sub_job_model, LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS))
                else:
                    log.error(f"Step 5: failed to exec backup sub job on all managers, job_id: {job_id}.")
                    key_log_details = get_recognized_err_from_sts_file(agent_id, bkp_sts_file, JobNameConst.BACKUP)
                    report_subjob_via_rpc(req_id, sub_job_model, key_log_details)
            else:
                log.info(f"Step 5-2: execSubJob success, req_id: {req_id}, job_id: {job_id}, sub_id: {sub_id}.")
                report_subjob_via_rpc(req_id, sub_job_model, LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS))
        log.info(
            f"Step 5: {sub_job_name} finished on {agent_id}, req_id: {req_id}, job_id: {job_id}, sub_id: {sub_id}.")

    @staticmethod
    def exec_mount_sub_job(mount_parameters):
        req_id = mount_parameters.req_id
        job_id = mount_parameters.job_id
        sub_id = mount_parameters.sub_id
        data_path = mount_parameters.data_path
        file_content = mount_parameters.file_content
        log.info(f'Step 5-1: start execute mount sub job, req_id:{req_id}, job_id:{job_id}, sub_id:{sub_id}.')
        original_size = 0
        ret, size = scan_dir_size(job_id, data_path)
        if ret:
            original_size = size
        original_cnts = count_files(data_path)
        sub_job_proc = SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=0, taskStatus=SubJobStatusEnum.RUNNING)
        log_detail = LogDetail(logInfo=ReportDBLabel.BACKUP_SUB_START_PREPARE, logInfoParam=[sub_job_proc.sub_task_id],
                               logLevel=DBLogLevel.INFO.value)
        sub_job_proc.log_detail = [log_detail]
        report_job_details(req_id, sub_job_proc)
        # 启动个线程 30秒报一次
        pool = ThreadPoolExecutor(max_workers=1, thread_name_prefix='goldendb-mount')
        message = str(int((time.time())))
        cache_path = get_repository_path(file_content, RepositoryDataTypeEnum.CACHE_REPOSITORY)
        write_file(os.path.join(cache_path, f'T{sub_id}'), message)
        log.info(f'Step 5-1: execute mount sub job: {sub_job_proc}, req_id:{req_id}, job_id:{job_id}, sub_id:{sub_id}.')
        copy_feature = pool.submit(GoldenDBBackupService.exec_mount, mount_parameters)
        while not copy_feature.done():
            time.sleep(Report.REPORT_INTERVAL)
            proc_params = ReportBkpProcParameters(req_id, job_id, sub_id, sub_job_proc, file_content, original_size,
                                                  original_cnts)
            if not GoldenDBBackupService.report_backup_process(proc_params):
                log.error(f'Mount failed, req_id: {req_id}, job_id: {job_id}, sub_id: {sub_id}.')
                pool.shutdown()
                key_log_details = LogDetail(logInfo=ReportDBLabel.SUB_JOB_FALIED, logInfoParam=[sub_id],
                                            logLevel=DBLogLevel.ERROR.value)
                return False, key_log_details
        if not copy_feature.result()[0]:
            log.error(f'Step 5-1: failed, {copy_feature.result()}, req_id:{req_id}, job_id:{job_id}, sub_id:{sub_id}.')
            return copy_feature.result()
        # 停线程，上报key_log_details
        pool.shutdown()
        log.info(f'Step 5-1: success, {copy_feature.result()}, req_id:{req_id}, job_id:{job_id}, sub_id:{sub_id}.')
        return copy_feature.result()

    @staticmethod
    def check_backup_on_all_managers(agent_id, bkp_sts_file, job_id, req_id, sub_id):
        # 判断所有管理节点的备份结果
        task_infos = TaskInfo(pid=req_id, jobId=job_id, subJobId=sub_id, taskType=JobNameConst.BACKUP,
                              logComm=f"pid: {req_id}, job_id: {job_id}, sub job id: {sub_id}")
        try:
            ret = check_task_on_all_managers(agent_id, bkp_sts_file, "Step 5-2: exec backup", task_infos)
        except Exception as ex:
            log.error(f"Step 5-2: {agent_id} check backup failed, message: {ex}, job_id: {job_id}.")
            ret = False
        return ret

    @staticmethod
    def query_scan_repositories(req_id, job_id, sub_id):
        # E6000适配
        log.info(f"Step 7: query_scan_repositories, req_id: {req_id}, job_id: {job_id}, sub_id: {sub_id}.")
        try:
            file_content = JsonParam.parse_param_with_jsonschema(req_id)
        except Exception as exp:
            log.error(exp, exc_info=True)
            return

        backup_type = file_content.get("job", {}).get("jobParam", {}).get("backupType", BackupTypeEnum.FULL_BACKUP)
        if backup_type == BackupTypeEnum.LOG_BACKUP:
            log.info(f"Step 7: query_scan_repositories for log bkp, job_id: {job_id}, sub_id: {sub_id}.")
            log_path = get_repository_path(file_content, RepositoryDataTypeEnum.LOG_REPOSITORY)
            # log仓的meta区 /Database_{resource_id}_LogRepository_su{num}/{ip}/meta/{job_id}
            meta_copy_path = os.path.join(os.path.dirname(log_path), RepositoryNameEnum.META, job_id)
            # log仓的data区 /Database_{resource_id}_LogRepository_su{num}/{ip}/{job_id}
            data_path = log_path
            # /Database_{resource_id}_LogRepository_su{num}/{ip}
            save_path = os.path.dirname(log_path)
        else:
            log.info(f"Step 7: query_scan_repositories for data bkp, job_id: {job_id}, sub_id: {sub_id}.")

            # meta/Database_{resource_id}_InnerDirectory_su{num}/source_policy_{job_id}/Context_Global_MD/{ip}
            meta_copy_path = get_repository_path(file_content, RepositoryDataTypeEnum.META_REPOSITORY)
            # data/Database_{resource_id}_InnerDirectory_su{num}/source_policy_{job_id}/Context/{ip}
            data_path = get_repository_path(file_content, RepositoryDataTypeEnum.DATA_REPOSITORY)
            # meta/Database_{resource_id}_InnerDirectory_su{num}/source_policy_{job_id}/Context_Global_MD/{ip}
            save_path = get_repository_path(file_content, RepositoryDataTypeEnum.META_REPOSITORY)
        if not os.path.exists(meta_copy_path):
            exec_mkdir_cmd(meta_copy_path, mode=0x777)
        log_meta_copy_repo = RepositoryPath(repositoryType=RepositoryDataTypeEnum.META_REPOSITORY,
                                            scanPath=meta_copy_path)
        log_data_repo = RepositoryPath(repositoryType=RepositoryDataTypeEnum.DATA_REPOSITORY, scanPath=data_path)
        scan_repos = ScanRepositories(scanRepoList=[log_data_repo, log_meta_copy_repo], savePath=save_path)
        log.info(f"Step 7: query_scan_repositories success, repos: {scan_repos},job_id: {job_id}, sub_id: {sub_id}.")
        output_result_file(req_id, scan_repos.dict(by_alias=True))

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
        report_action_result(req_id, job_id, '', True, 'Step 7: backup_post_job success')
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
    def exec_mount(mount_parameters):
        """
        执行子任务的挂载操作

        参数:
        mount_parameters: 包含挂载所需的参数，如文件内容, 数据路径, 任务ID等

        返回值:
        1）执行结果 (bool): 成功执行挂载操作返回True, 失败则返回False
        2）日志详情 (LogDetail): 关键字段如下, log_info: str, 标签; log_detail_param: list, 错误参数; log_detail: int, 错误码;
        """
        log.info(f'Step 5-1: start execute mount sub job.')
        file_content = mount_parameters.file_content
        data_path = mount_parameters.data_path
        job_id = mount_parameters.job_id
        job_infos = file_content.get("subJob", {}).get("jobInfo", " ").strip().split(" ")
        # job_infos调用前已校验
        role_name, role_type, _, _ = job_infos
        backup_path = get_backup_path(role_name, role_type, file_content, GoldenDBJsonConst.PROTECTOBJECT)
        goldendb_structure = get_goldendb_structure(file_content)
        cluster_id = goldendb_structure.cluster_id
        rel_dir = 'LOGICAL_BACKUP' if role_type == GoldenDBNodeType.GTM_NODE else 'DATA_BACKUP'
        data_bkp_mnt_dir = os.path.join(data_path, f'DBCluster_{cluster_id}', rel_dir)
        prod_bkp_mnt_dir = os.path.join(backup_path, f'DBCluster_{cluster_id}', rel_dir)
        # 解挂载生产端的备份根目录
        umount_bind_path(prod_bkp_mnt_dir)
        group_name, _ = get_user_info(role_name)
        if not mkdir_chmod_chown_dir_recursively(data_bkp_mnt_dir, 0o770, role_name, group_name, True):
            log.error(f"Fail to make a data path, job_id: {job_id}.")
            return False, LogDetail(logInfo=ReportDBLabel.SUB_JOB_FALIED, logDetail=ErrorCode.ERROR_INTERNAL)
        if not mkdir_chmod_chown_dir_recursively(prod_bkp_mnt_dir, 0o770, role_name, group_name, True):
            log.error(f"Fail to make a back path, job_id: {job_id}.")
            return False, LogDetail(logInfo=ReportDBLabel.SUB_JOB_FALIED, logDetail=ErrorCode.ERROR_INTERNAL)
        if not mount_bind_path(data_bkp_mnt_dir, prod_bkp_mnt_dir, job_id):
            log.error(f"Mount data_path failed, job_id: {job_id}.")
            return False, LogDetail(logInfo=ReportDBLabel.SUB_JOB_FALIED, logDetail=ErrorCode.ERROR_MOUNT_PATH)
        log.info(f"Step 5-1:finish Mount bind all backup path success, job_id: {job_id}.")
        return True, LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS)

    @staticmethod
    def exec_backup_sub_job(backup_params):
        job_id, sub_id, req_id, file_content, role_name, sla_policy = (
            backup_params.job_id, backup_params.sub_id, backup_params.req_id, backup_params.file_content,
            backup_params.role_name, backup_params.sla_policy)
        meta_path = get_repository_path(file_content, RepositoryDataTypeEnum.META_REPOSITORY)
        bkp_sts_file = os.path.join(meta_path, f'{job_id}_exec_backup_status.json')
        log.info(f"Step 5-2: start to backup, pid: {req_id}, job_id: {job_id}, sub job id: {sub_id}")
        task_infos = TaskInfo(pid=req_id, jobId=job_id, subJobId=sub_id, taskType=JobNameConst.BACKUP,
                              jsonParam=file_content,
                              logComm=f"pid: {req_id}, job_id: {job_id}, sub job id: {sub_id}")
        result = exec_task_on_all_managers(GoldenDBBackupService.get_exec_bkp_result_with_success_report, bkp_sts_file,
                                           f"Step 5-2: exec backup ", task_infos, backup_params)
        return result

    @staticmethod
    def get_exec_bkp_result_with_success_report(agent_id, bkp_sts_file, bkp_params):
        """
        功能描述：从单个管理节点中获取备份结果，根据该结果，更新恢复状态文件。
        1. 执行成功时，通过rpc上报。
        2. dbtool原生异常已在执行后完成更新，只需处理其余异常。
        """
        job_id = bkp_params.job_id
        role_name = bkp_params.role_name
        on_cm = check_exec_on_cm(role_name, job_id)
        bkp_result = GoldenDBBackupService.exec_bkp_on_manager_with_success_report(bkp_params, on_cm)
        update_agent_sts_general_after_exec(agent_id, bkp_sts_file, JobNameConst.BACKUP, bkp_result,
                                            f"job id: {job_id}")
        return bkp_result

    @staticmethod
    def exec_bkp_on_manager_with_success_report(backup_parameters, on_cm):
        """
        功能描述：在单个管理节点中执行备份，判断在cm还是mds中执行，执行成功时，通过rpc上报。
        """
        job_id, sub_id, req_id, file_content, role_name, role_type, sla_policy = (
            backup_parameters.job_id, backup_parameters.sub_id, backup_parameters.req_id,
            backup_parameters.file_content,
            backup_parameters.role_name, backup_parameters.role_type, backup_parameters.sla_policy)
        log.info(f'Step 5-2: start to execute backup, job_id: {job_id}, sub_id: {sub_id}, req_id: {req_id}.')
        # 获取集群版本
        version = GoldenDBResourceInfo.get_cluster_version(role_name)
        cache_path = get_repository_path(file_content, RepositoryDataTypeEnum.CACHE_REPOSITORY)
        version_file = os.path.join(cache_path, f"bkp_{job_id}_version.json")
        output_execution_result_ex(version_file, {"version": version})
        data_path = get_repository_path(file_content, RepositoryDataTypeEnum.DATA_REPOSITORY)
        ret, size = scan_dir_size(job_id, data_path)  # kb
        original_size = 0 if not ret else size
        original_cnts = count_files(data_path)
        sub_job_proc = SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=0, taskStatus=SubJobStatusEnum.RUNNING)
        log_detail = LogDetail(logInfo=ReportDBLabel.BACKUP_SUB_START_COPY, logInfoParam=[sub_job_proc.sub_task_id],
                               logLevel=DBLogLevel.INFO.value)
        sub_job_proc.log_detail = [log_detail]
        report_job_details(req_id, sub_job_proc)
        pool = ThreadPoolExecutor(max_workers=1, thread_name_prefix='goldendb-backup')
        start_time = time.time()
        write_file(os.path.join(cache_path, f'T{sub_id}'), str(int(start_time)))
        # 从管理节点配置文件中取备份根目录
        backup_path = get_backup_path(role_name, role_type, file_content, GoldenDBJsonConst.PROTECTOBJECT)
        # 获取本次备份前，生产端的备份结果文件的文件名
        pre_bkp_files = get_bkp_result_names_in_cluster_id_dir(backup_path, file_content)

        backup_cmd_parameters = BackupCmdParameters(req_id, role_name, file_content, sla_policy, job_id, sub_id,
                                                    data_path)
        # 执行备份指令，完成后，检查命令回显以及文件系统数据仓中的副本文件，其中，数据量单位为kb
        copy_feature = pool.submit(GoldenDBBackupService.exec_backup_cmd, backup_cmd_parameters, on_cm)
        while not copy_feature.done():
            time.sleep(Report.REPORT_INTERVAL)
            proc_params = ReportBkpProcParameters(req_id, job_id, sub_id, sub_job_proc, file_content, original_size,
                                                  original_cnts)
            if not GoldenDBBackupService.report_backup_process(proc_params):
                log.error(f'Backup failed: backup root error, job_id: {job_id}, sub_id: {sub_id}, req_id: {req_id}.')
                pool.shutdown()
                return False
        if not copy_feature.result()[0]:
            return False
        data_size = copy_feature.result()[1] - original_size
        end_time = time.time()
        sub_job_proc.speed = data_size / (end_time - start_time) if (end_time - start_time) != 0 else data_size
        sub_job_proc.data_size = data_size
        sub_job_proc.task_status = SubJobStatusEnum.COMPLETED.value
        sub_job_proc.progress = 100
        log_detail = LogDetail(logInfo=ReportDBLabel.BACKUP_SUB_JOB_SUCCESS,
                               logInfoParam=[sub_job_proc.sub_task_id, str(count_files(data_path) - original_cnts),
                                             format_capacity(data_size)], logLevel=DBLogLevel.INFO.value)
        sub_job_proc.log_detail = [log_detail]
        report_job_details(req_id, sub_job_proc)
        log.info(f'Step 5-2: bkp success: {sub_job_proc}, job_id: {job_id}, sub_id: {sub_id}, req_id: {req_id}.')
        GoldenDBBackupService.rm_incr_redundants(file_content, job_id, on_cm, pre_bkp_files, req_id, sla_policy, sub_id)
        return True

    @staticmethod
    def rm_incr_redundants(file_content, job_id, on_cm, pre_bkp_files, req_id, sla_policy, sub_id):
        # 根据是否在cm中运行dbtool取参数
        bkp_params = get_backup_param_for_cm_backup(req_id, file_content, sla_policy) \
            if on_cm else get_backup_param(req_id, file_content, sla_policy)
        cluster_id = bkp_params.cluster_id
        # 删除文件系统冗余文件
        if not GoldenDBBackupService.rm_all_redundant_incr_result_infos(cluster_id, file_content, pre_bkp_files):
            log.error(f'Delete redundant incr bkp info failed, job_id: {job_id}, sub_id: {sub_id}, req_id: {req_id}.')

    @staticmethod
    def check_bkp_cmd_echo(exec_bkp_outputs, bkp_cmd_params, on_cm):
        """
        功能描述：检查备份指令回显内容，返回可识别的异常，其余异常，默认为ErrorCode.ERROR_INTERNAL。
        """
        req_id = bkp_cmd_params.req_id
        job_id = bkp_cmd_params.job_id
        sub_id = bkp_cmd_params.sub_id
        role_name = bkp_cmd_params.role_name
        log_detail_param = None
        if on_cm:
            result, backup_id, log_detail = check_dbtool_start(req_id, job_id, sub_id,
                                                               exec_bkp_outputs,
                                                               JobNameConst.BACKUP)
            # 检查发起备份命令结果
            result, log_detail = get_dbtool_cm_result(JobNameConst.BACKUP, role_name, (result, backup_id, log_detail),
                                                      job_id, sub_id)
            return result, log_detail
        else:
            if exec_bkp_outputs[0] != CMDResult.SUCCESS.value:
                log_detail = ErrorCode.ERROR_INTERNAL
                log_info = ReportDBLabel.SUB_JOB_FALIED
                log.error(f"Step 5: failed to exec dbtool cmd, job_id: {job_id}, sub_id: {sub_id}.")
                if "response message: " in exec_bkp_outputs[1]:
                    log.error(
                        f"Step 5: failed to exec dbtool cmd, err in response, job_id: {job_id}, sub_id: {sub_id}.")
                    message = exec_bkp_outputs[1].split("response message: ")[1].replace("\\r\\n", " ").rstrip(
                        "'").strip()
                    log.error(f"dbtool message: {message}, job_id: {job_id}, sub_id: {sub_id}")
                    log_detail_model = get_err_log_detail_from_echo(message, JobNameConst.BACKUP)
                else:
                    log_detail_model = LogDetail(logInfo=log_info, logDetail=log_detail,
                                                 logDetailParam=log_detail_param)
                return False, log_detail_model
            log.info(f"Step 5: exec dbtool on mds success, job_id: {job_id}, sub_id: {sub_id}.")
            # 检查正确，无错误码，无错误参数，返回成功标签
            return True, LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS)

    @staticmethod
    def check_dn_xtream_results(file_content):
        """
        功能描述：根据备份结果文件内容，获取各数据节点的备份结果文件，检查文件系统中该文件是否存在。
        返回可识别的异常，其余异常，默认为ErrorCode.ERROR_INTERNAL。
        返回可识别的异常，其余异常，默认错误为ErrorCode.ERROR_INTERNAL。
        检查成功，返回True, LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS)；
        检查失败，返回False，LogDetail(错误码，参数，标签)。
        """
        data_path = get_repository_path(file_content, RepositoryDataTypeEnum.DATA_REPOSITORY)
        goldendb_structure = get_goldendb_structure(file_content)
        cluster_id = goldendb_structure.cluster_id
        # 获取备份结果文件路径，文件名，有后缀
        results_info_file = get_bkp_result_info_file(cluster_id, data_path)
        if not results_info_file:
            # 文件系统中，备份结果文件名为空，异常
            log.error(f'check_dn_xtream_results results_info_path failed, no backup_resultsinfo.')
            task_id = get_bkp_task_id(data_path, cluster_id)
            err_path = os.path.join(data_path, f'DBCluster_{cluster_id}', f"{cluster_id}_backup_resultsinfo.{task_id}")
            log_detail_model = LogDetail(logInfo=ReportDBLabel.COPY_VERIFICATION_FALIED,
                                         logDetail=ErrorCode.ERR_BKP_CHECK,
                                         logDetailParam=[err_path])
            return False, log_detail_model
        results_info_path = os.path.join(data_path, f"DBCluster_{cluster_id}", results_info_file)
        if os.path.exists(results_info_path):
            with open(results_info_path, encoding='utf-8') as file:
                # 读取备份结果文件内容
                contents = file.readlines()
        else:
            # 文件系统中，备份结果文件路径不存在，异常
            log.error(f'check_dn_xtream_results results_info_path failed.')
            log_detail_model = LogDetail(logInfo=ReportDBLabel.COPY_VERIFICATION_FALIED,
                                         logDetail=ErrorCode.ERR_BKP_CHECK,
                                         logDetailParam=[results_info_path])
            return False, log_detail_model
        log.info(f'check_dn_xtream_results contents: {contents}.')
        record_info_list = []
        for record in contents:
            record_info = record.split()
            record_info_list.append(record_info)
        log.info(f'check_dn_xtream_results record_info_list: {record_info_list}.')
        # 判断备份结果文件列出的数据节点文件在文件系统中是否存在
        for record_info in record_info_list:
            ret, err_path = GoldenDBBackupService.check_dn_xtream_result(record_info, data_path, cluster_id)
            if not ret:
                # 文件系统中，数据节点副本缺失，异常
                log.error(f'check_dn_xtream_results check_dn_xtream_result failed, not exist: {err_path}.')
                log_detail_model = LogDetail(logInfo=ReportDBLabel.COPY_VERIFICATION_FALIED,
                                             logDetail=ErrorCode.ERR_BKP_CHECK,
                                             logDetailParam=[err_path])
                return False, log_detail_model
        # 检查正确，无错误码，无错误参数，返回成功标签
        return True, LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS)

    @staticmethod
    def check_dn_xtream_result(record_info, data_path, cluster_id):
        """
        功能描述：检查数据节点生成的备份结果文件，在文件系统中是否存在。
        备份根目录/DBCluster_{cid}/DATA_BACKUP/{tid}/Data/Node_{groupid}_{roomid}_{ip}_{port}/{ip}_{bkp_type}_{tid}.xtream
        """
        # 在备份结果文件中，3表示副本信息
        if int(record_info[0]) == 3:
            task_id = get_bkp_task_id(data_path, cluster_id)
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
    def report_backup_process(report_bkp_proc_parameters):
        req_id = report_bkp_proc_parameters.req_id
        job_id = report_bkp_proc_parameters.job_id
        sub_id = report_bkp_proc_parameters.sub_id
        sub_job_details = report_bkp_proc_parameters.process
        file_content = report_bkp_proc_parameters.file_content
        original_size = report_bkp_proc_parameters.original_size
        original_cnts = report_bkp_proc_parameters.original_cnts
        sub_job_name = file_content.get("subJob", {}).get("jobName")
        data_path = get_repository_path(file_content, RepositoryDataTypeEnum.DATA_REPOSITORY)
        if not verify_path_trustlist(data_path):
            log.error(f"Invalid src path: {data_path}.")
            return False
        if not os.path.exists(data_path):
            log.info(f"No data path: {data_path}, {sub_job_details}.")
            report_job_details(req_id, sub_job_details)
            return True
        if sub_job_name == SubJobName.MOUNT:
            log.info(f"Reporting {sub_job_name}, req_id: {req_id}, job_id: {job_id}, sub_id: {sub_id}.")
            log_detail = LogDetail(logInfo=ReportDBLabel.BACKUP_SUB_START_PREPARE,
                                   logInfoParam=[sub_id], logLevel=DBLogLevel.INFO.value)
        elif sub_job_name == SubJobName.EXEC_BACKUP:
            log.info(f"Reporting {sub_job_name}, req_id: {req_id}, job_id: {job_id}, sub_id: {sub_id}.")
            time_file = os.path.join(
                get_repository_path(file_content, RepositoryDataTypeEnum.CACHE_REPOSITORY),
                f'T{sub_id}')
            size, speed = query_size_and_speed(time_file, data_path, original_size, job_id)
            sub_job_details.progress = 50
            sub_job_details.speed = speed
            sub_job_details.data_size = size - original_size
            file_num = count_files(data_path) - original_cnts
            log_detail = LogDetail(logInfo=ReportDBLabel.BACKUP_SUB_JOB_SUCCESS,
                                   logInfoParam=[sub_id, str(file_num), format_capacity(size - original_size)],
                                   logLevel=DBLogLevel.INFO.value)
        else:
            log.info(f"Reporting {sub_job_name}, req_id: {req_id}, job_id: {job_id}, sub_id: {sub_id}.")
            log_detail = LogDetail(logInfo=ReportDBLabel.BACKUP_SUB_START_COPY,
                                   logInfoParam=[sub_id], logLevel=DBLogLevel.INFO.value)
        sub_job_details.log_detail = [log_detail]
        report_job_details(req_id, sub_job_details)
        log.info(f"Report success {sub_job_details}, req_id: {req_id}, job_id: {job_id}, sub_id: {sub_id}.")
        return True

    @staticmethod
    def exec_backup_cmd(bkp_cmd_params, on_cm):
        file_content = bkp_cmd_params.file_content
        req_id = bkp_cmd_params.req_id
        sla_policy = bkp_cmd_params.sla_policy
        role_name = bkp_cmd_params.role_name
        data_path = bkp_cmd_params.data_path
        job_id = bkp_cmd_params.job_id
        sub_id = bkp_cmd_params.sub_id
        # 根据是否在cm中运行dbtool取参数
        bkp_params = get_backup_param_for_cm_backup(req_id, file_content, sla_policy) \
            if on_cm else get_backup_param(req_id, file_content, sla_policy)
        cluster_id = bkp_params.cluster_id
        backup_type = bkp_params.backup_type_str
        master_or_slave = bkp_params.master_or_slave
        cluster_user = bkp_params.cluster_user
        job_infos = file_content.get("subJob", {}).get("jobInfo", " ").strip().split(" ")
        _, role_type, _, agent_id = job_infos
        meta_path = get_repository_path(file_content, RepositoryDataTypeEnum.META_REPOSITORY)
        bkp_sts_file = os.path.join(meta_path, f'{job_id}_exec_backup_status.json')
        backup_path = get_backup_path(role_name, role_type, file_content, GoldenDBJsonConst.PROTECTOBJECT)
        # 从管理节点配置文件中取备份根目录
        if GoldenDBBackupService.bkp_cmd_injection(backup_type, cluster_id, cluster_user, master_or_slave, role_name):
            log.error(f"Command injection detected in backup command! job_id: {job_id}, sub_id: {sub_id}.")
            return False, 0
        bkp_outputs = GoldenDBBackupService.get_dbtool_bkp_result(bkp_cmd_params, bkp_params, on_cm)
        # 检查是否需要切换主备节点重新执行dbtool
        bkp_outputs = GoldenDBBackupService.swith_and_retry(bkp_cmd_params, bkp_params, on_cm, bkp_outputs)
        status, out_str, err_info = bkp_outputs # status:str
        if status != CMDResult.SUCCESS.value:
            log.error(f"Backup failed, {status}, {out_str}, {err_info}, job_id: {job_id}, sub_id: {sub_id}.")
        ret, log_detail = GoldenDBBackupService.check_res_after_bkp(bkp_outputs, bkp_cmd_params, file_content, on_cm)
        if not ret:
            log.error(f"Backup failed with output: {bkp_outputs}, {log_detail}, job_id: {job_id}, sub_id: {sub_id}.")
            sts_info = StatusInfo(status=SubJobStatusEnum.FAILED.value, logDetail=log_detail.log_detail,
                                  logDetailParam=log_detail.log_detail_param, logInfo=log_detail.log_info)
            update_agent_sts(agent_id, bkp_sts_file, sts_info)
            return False, 0
        cluster_structure = get_goldendb_structure(file_content)
        GoldenDBBackupService.write_backup_result(cluster_structure, data_path, file_content)
        # 备份后备份目录大小。和备份前记录的文件目录大小对比， 可比较出实际备份的内容大小
        _, size = scan_dir_size(job_id, data_path)  # kb
        return True, size

    @staticmethod
    def swith_and_retry(bkp_cmd_params, backup_params, on_cm, bkp_outputs):
        job_id = bkp_cmd_params.job_id
        master_or_slave = backup_params.master_or_slave
        status, out_str, _ = bkp_outputs
        message = ""
        if status != CMDResult.SUCCESS.value:
            log.error(f"Failed to exec dbtool cmd on {master_or_slave}, err in response, job_id: {job_id}.")
            if "response message: " in out_str:
                log.error(f"Step 5: failed to exec dbtool cmd, err in response, job_id: {job_id}.")
                message = out_str.split("response message: ")[1].replace("\\r\\n", " ").rstrip(
                    "'").strip()
                log.error(f"dbtool message: {message}, job_id: {job_id}.")
            if ErrPattern.Select_Bkp_DB_Fail in message:
                log.error(f'Failed to exec dbtool cmd on {master_or_slave}, will switch and retry, job_id: {job_id}.')
                if master_or_slave == f"-{MasterSlavePolicy.MASTER}":
                    master_or_slave = ""
                    log.info(f'Switch to slave nodes, will retry on slave data nodes, job_id: {job_id}.')
                else:
                    master_or_slave = f"-{MasterSlavePolicy.MASTER}"
                    log.info(f'Switch to master nodes, will retry on master data nodes, job_id: {job_id}.')
                backup_params._replace(master_or_slave=master_or_slave)
                bkp_outputs = GoldenDBBackupService.get_dbtool_bkp_result(bkp_cmd_params, backup_params, on_cm)
                log.info(f'Get retry output: {bkp_outputs}, job_id: {job_id}.')
            else:
                log.error(f'Failed to exec dbtool cmd on {master_or_slave}, no need to retry, job_id: {job_id}.')
        else:
            log.info(f"No need to retry, job id: {job_id}.")
        return bkp_outputs

    @staticmethod
    def get_dbtool_bkp_result(backup_cmd_parameters, backup_params, on_cm):
        req_id = backup_cmd_parameters.req_id
        job_id = backup_cmd_parameters.job_id
        role_name = backup_cmd_parameters.role_name
        cluster_id = backup_params.cluster_id
        master_or_slave = backup_params.master_or_slave
        cluster_user = backup_params.cluster_user
        backup_type = backup_params.backup_type_str

        err_info = ""
        if on_cm:
            log.info(f"Will execute backup on cm, job id: {job_id}.")
            backup_cmd = f'su - {role_name} -c "dbtool -cm -backup -strategy={master_or_slave} ' \
                         f'-clusterid={cluster_id}  -groupid=all -type={backup_type} -backup-start-binlog=yes ' \
                         f'-auto-adjust=yes"'
            log.info(f"Get backup cmd: {backup_cmd}, job id: {job_id}.")
            return_code, return_info, err_info = execute_cmd(backup_cmd)
            log.info(f"Get cm result: {(return_code, return_info, err_info)}, job id: {job_id}.")
            status = return_code
        else:
            log.info(f"Will execute backup on mds, job id: {job_id}.")
            backup_cmd = f'su - {role_name} -c "dbtool -mds -backup -binlogBackup -c={cluster_id} {backup_type} ' \
                         f'{master_or_slave} -user={cluster_user} -password"'
            log.info(f"Get backup cmd: {backup_cmd}, job id: {job_id}.")
            # 需要输密码的命令
            return_code, return_info = exec_cmd_spawn(backup_cmd, req_id)
            status = str(return_code) if isinstance(return_code, int) else "1"
            log.info(f'Get mds result: {(status, return_info)}, job id: {job_id}.')
        return status, return_info, err_info

    @staticmethod
    def handle_files_after_bkp(prod_bkp_root, data_path, cluster_id, file_content):
        """
        复制活跃事务文件夹至文件系统, 复制备份结果文件至文件系统。
        :param prod_bkp_root: goldendb备份路径
        :param data_path: 文件系统数据仓路径
        :param cluster_id: 集群id
        :param file_content: pm下发参数
        :return:
        """
        cluster_structure = get_goldendb_structure(file_content)
        manager_os_user = cluster_structure.manager_nodes[0][0]
        if not cp_active_folder(prod_bkp_root, data_path, manager_os_user):
            log.error('Copy active tx info folder failed.')
            return False
        # 文件名，有后缀
        bkp_resultinfo_file = get_bkp_result_info_file(cluster_id, prod_bkp_root)
        # 将生产端备份结果文件，复制到文件系统数据仓
        if not cp_result_info(cluster_id, bkp_resultinfo_file, prod_bkp_root, data_path):
            log.error('Copy backup result info file failed.')
            return False
        return True

    @staticmethod
    def check_res_after_bkp(exec_bkp_outputs, bkp_cmd_params, file_content, on_cm):
        """
        功能描述：备份指令执行后，检查备份结果：1）备份指令回显内容；2）通过备份结果文件内容，检查备份数据是否完整。

        返回值:
        bool, int, list, string:
        检查成功，返回True, LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS)；
        检查失败，返回False，LogDetail(错误码，参数，标签)。
        """
        ret, log_detail_model = GoldenDBBackupService.check_bkp_cmd_echo(exec_bkp_outputs, bkp_cmd_params, on_cm)
        if not ret:
            log.error(f'check_bkp_cmd_echo failed: {exec_bkp_outputs[0]}, {log_detail_model}.')
            return False, log_detail_model

        file_content = bkp_cmd_params.file_content
        req_id = bkp_cmd_params.req_id
        sla_policy = bkp_cmd_params.sla_policy
        role_name = bkp_cmd_params.role_name
        data_path = bkp_cmd_params.data_path
        job_id = bkp_cmd_params.job_id

        # 根据是否在cm中运行dbtool取参数
        bkp_params = get_backup_param_for_cm_backup(req_id, file_content, sla_policy) \
            if on_cm else get_backup_param(req_id, file_content, sla_policy)
        cluster_id = bkp_params.cluster_id
        job_infos = file_content.get("subJob", {}).get("jobInfo", " ").strip().split(" ")
        _, role_type, _, agent_id = job_infos
        backup_path = get_backup_path(role_name, role_type, file_content, GoldenDBJsonConst.PROTECTOBJECT)

        prod_bkp_root = get_etc_ini_path(role_name, "active", backup_path, job_id)
        # 复制活跃事务文件夹至文件系统, 复制备份结果文件至文件系统, 并删除冗余增量文件
        if not GoldenDBBackupService.handle_files_after_bkp(prod_bkp_root, data_path, cluster_id, file_content):
            log.error(f'Backup failed, check_backup_data failed, {log_detail_model}.')
            return True, log_detail_model

        ret, log_detail_model = GoldenDBBackupService.check_dn_xtream_results(file_content)
        if not ret:
            log.error(f'Backup failed, check_backup_data failed, {log_detail_model}.')
            return True, log_detail_model
        return True, LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS)

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
        # 获取本次备份任务的类型，默认为全量备份
        backup_type = file_content.get("job", {}).get("jobParam", {}).get("backupType", BackupTypeEnum.FULL_BACKUP)
        # 全量备份不需要处理
        if backup_type != BackupTypeEnum.FULL_BACKUP:
            cluster_id_path = os.path.join(data_path, f'DBCluster_{cluster_id}')
            cluster_bkp_result_infos = [f for f in os.listdir(cluster_id_path) if f.startswith(f'{cluster_id}_back')]
            for result_info in cluster_bkp_result_infos:
                # 删除冗余的增量备份文件
                GoldenDBBackupService.rm_redundant_incr_result_info(cluster_id_path, result_info, pre_bkp_files)
                log.info(f"Remove redundant incremental files under {cluster_id_path} finished.")
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
                    log.error(f'Backup task folders {task_path} not exists.')
                # 在文件系统中，删除冗余增量备份task id（时间戳）文件夹
                if not su_exec_rm_cmd(task_path):
                    log.error(f'Remove redundant incremental backup task folders {task_path} failed.')
                # 在文件系统中，删除冗余增量备份结果文件
                if not su_exec_rm_cmd(result_info_path):
                    log.error(f'Remove redundant incremental backup info files {result_info_path} failed.')
