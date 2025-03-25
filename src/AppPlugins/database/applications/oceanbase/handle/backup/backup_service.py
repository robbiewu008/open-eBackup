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
import pathlib

import psutil

from common.common import output_result_file, report_job_details
from common.common_models import SubJobDetails, LogDetail, CopyInfoRepModel, Copy
from common.const import SubJobStatusEnum, ExecuteResultEnum, RepositoryDataTypeEnum
from common.parse_parafile import ParamFileUtil

from oceanbase.common.const import ActionResponse, OceanBaseCode, OceanBaseSubJobName, LogLevel, JobInfo, ErrorCode
from oceanbase.common.oceanbase_backup_exception import LogDetailException
from oceanbase.handle.backup.exec_backup import BackUp
from oceanbase.logger import log
from dws.commons.dws_param_parse import DwsParamParse


class OceanBaseBackupService(object):
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
        param = OceanBaseBackupService.safe_get_info(req_id)
        BackUp.allow_backup_in_local_node(req_id, job_id, param)
        log.info(f"step 1: execute AllowBackupInLocalNode interface success")
        return True

    @staticmethod
    def check_backup_job_type(req_id, job_id, sub_id, json):
        """
        功能描述：检查增量备份是否转全量
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        返回值：
        """
        log.info(
            f'step 2: start execute check_backup_job_type, req_id: {req_id}, job_id: {job_id},  sub_id: {sub_id}')
        param = OceanBaseBackupService.safe_get_info(req_id)
        log.info(f"check_backup_job_type param: {param}")
        backup_inst = BackUp(req_id, job_id, sub_id, json, param)
        backup_inst.check_backup_job_type()
        response = ActionResponse(code=ExecuteResultEnum.SUCCESS)
        output_result_file(req_id, response.dict(by_alias=True))
        return True

    @staticmethod
    def backup_prerequisite_job(req_id, job_id, sub_id, json):
        """
        功能描述：备份前置任务, 业务目前不需要实现, 主任务执行
        参数：
         @req_id： 请求ID
         @job_id： 主任务任务ID
        返回值：
        """
        log.info(f'step 3: execute backup_pre_job,req_id:{req_id} , job_id:{job_id}, sub_id:{sub_id}, json:{json}')
        response = ActionResponse(code=ExecuteResultEnum.SUCCESS)
        # 尝试解析参数，失败return False，将失败写入结果文件
        try:
            file_content = ParamFileUtil.parse_param_file(req_id)
        except Exception as exception:
            log.error(exception, exc_info=True)
            BackUp.set_error_response(response)
            output_result_file(req_id, response.dict(by_alias=True))
            return False
        output_result_file(req_id, response.dict(by_alias=True))
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
            f'step 4: start execute backup_prerequisite_job_progress, req_id: {req_id}, job_id: {job_id},\
            sub_id: {sub_id}')
        progress = SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100,
                                 taskStatus=SubJobStatusEnum.COMPLETED.value)
        output_result_file(req_id, progress.dict(by_alias=True))
        return True

    @staticmethod
    def backup_gen_sub_job(req_id, job_id, sub_id, json):
        response = ActionResponse()
        log.info(
            f'step 5: start execute backup_gen_sub_job, req_id: {req_id}, job_id: {job_id},  sub_id: {sub_id}')
        param = OceanBaseBackupService.safe_get_info(req_id)
        backup_inst = BackUp(req_id, job_id, sub_id, json, param)
        try:
            ret = backup_inst.gen_sub_job()
        except Exception:
            OceanBaseBackupService.set_error_response(response)
            OceanBaseBackupService.report_error_job_details(job_id, req_id, response, sub_id)
            return False
        if not ret:
            OceanBaseBackupService.set_error_response(response)
            OceanBaseBackupService.report_error_job_details(job_id, req_id, response, sub_id)
            return False
        return True

    @staticmethod
    def exec_backup(req_id, job_id, sub_id, json):
        response = ActionResponse()
        log.info(f'step 6: start to execute backup, req_id: {req_id}, job_id: {job_id},sub_id:{sub_id}')
        file_content = OceanBaseBackupService.safe_get_info(req_id)
        sub_job_name = file_content.get("subJob", {}).get("jobName", "")
        backup_inst = BackUp(req_id, job_id, sub_id, json, file_content)
        exec_backup_func_dict = {
            OceanBaseSubJobName.SUB_EXEC_MOUNT_JOB: backup_inst.exec_mount_job,
            OceanBaseSubJobName.SUB_CHECK_LOG_STATUS: backup_inst.exec_check_log_status_sub_job,
            OceanBaseSubJobName.SUB_EXEC_DATA_BACKUP: backup_inst.exec_data_backup_sub_job,
            OceanBaseSubJobName.SUB_EXEC_LOG_COPY: backup_inst.exec_log_copy_sub_job
        }
        try:
            ret = exec_backup_func_dict.get(sub_job_name)()
        except LogDetailException as err_expected:
            log_detail = err_expected.log_detail
            OceanBaseBackupService.set_error_response(response)
            OceanBaseBackupService.report_error_job_details(job_id, req_id, response, sub_id, log_detail)
            return False
        except Exception as err_unexpected:
            log.error(f"exec_backup failed: {sub_job_name} : {err_unexpected}")
            OceanBaseBackupService.set_error_response(response)
            OceanBaseBackupService.report_error_job_details(job_id, req_id, response, sub_id)
            return False

        if sub_job_name == OceanBaseSubJobName.SUB_EXEC_DATA_BACKUP \
                or sub_job_name == OceanBaseSubJobName.SUB_EXEC_LOG_COPY:
            if not ret[0]:
                OceanBaseBackupService.set_error_response(response)
                OceanBaseBackupService.report_error_job_details(job_id, req_id, response, sub_id)
                return False
            total_dir_size, speed = ret[1], ret[2]
        else:
            if not ret:
                OceanBaseBackupService.set_error_response(response)
                OceanBaseBackupService.report_error_job_details(job_id, req_id, response, sub_id)
                return False
            total_dir_size, speed = 0, 0
        response = ActionResponse(code=ExecuteResultEnum.SUCCESS)
        output_result_file(req_id, response.dict(by_alias=True))
        log.info(f"exec_backup_success, total_dir_size {total_dir_size}, speed {speed}")
        log_detail = LogDetail(logInfo="plugin_task_subjob_success_label",
                               logInfoParam=[sub_id], logLevel=LogLevel.INFO)
        report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100,
                                                 logDetail=[log_detail], dataSize=total_dir_size, speed=speed,
                                                 taskStatus=SubJobStatusEnum.COMPLETED.value))
        log.info(f"exec_backup end")
        return True
    
    @staticmethod
    def query_scan_repositories(req_id, job_id, sub_id, json):
        param = OceanBaseBackupService.safe_get_info(req_id)
        backup_inst = BackUp(req_id, job_id, sub_id, json, param)
        backup_inst.query_scan_repositories()
        log.info("execute queryScanRepositories interface success")
        return True

    @staticmethod
    def abort_job(req_id, job_id):
        param = OceanBaseBackupService.safe_get_info(req_id)
        cache_area = BackUp.get_repository_path(param, RepositoryDataTypeEnum.CACHE_REPOSITORY)
        abort_file = os.path.join(cache_area, "abort.ing")
        pathlib.Path(abort_file).touch()
        pid_list = psutil.pids()
        for pid in pid_list:
            process = psutil.Process(pid)
            try:
                cmd = process.cmdline()
            except Exception as ex:
                log.warn(f"The pid {pid} of process not exist! err:{ex}")
                continue
            if 'python3' in cmd and (job_id in cmd and req_id not in cmd):
                try:
                    process.kill()
                except Exception as ex:
                    log.warn(f"Kill process kill error!err:{ex}")
                    break
                log.info(f"The backup task has been terminated, job id: {job_id}.")
                break
        os.rename(abort_file, os.path.join(cache_area, "abort.done"))
        log.info(f"Succeed to abort backup job, job id: {job_id}.")
        return True

    @staticmethod
    def query_abort_job_progress(req_id, job_id, sub_id):
        param = OceanBaseBackupService.safe_get_info(req_id)
        cache_area = BackUp.get_repository_path(param, RepositoryDataTypeEnum.CACHE_REPOSITORY)
        if os.path.exists(os.path.join(cache_area, "abort.ing")):
            status = SubJobStatusEnum.ABORTING.value
        elif os.path.exists(os.path.join(cache_area, "abort.done")):
            status = SubJobStatusEnum.ABORTED.value
        else:
            status = SubJobStatusEnum.ABORTED_FAILED.value
        output = SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, taskStatus=status)
        log.info(f"SubJobDetails: {output.dict(by_alias=True)}, job id: {job_id}.")
        output_result_file(req_id, output.dict(by_alias=True))
        return True

    @staticmethod
    def set_error_response(response):
        response.code = OceanBaseCode.FAILED.value
        response.body_err = OceanBaseCode.FAILED.value

    @staticmethod
    def report_error_job_details(job_id, req_id, response, sub_id, log_detail=None):
        if log_detail is None:
            log_detail = LogDetail(logInfo="plugin_task_subjob_fail_label", logInfoParam=[sub_id],
                                   logLevel=LogLevel.ERROR)
        if response != OceanBaseCode.SUCCESS.value:
            log.info(f'report_sub_job_details:{response}')
            report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100,
                                                     logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value))

    @staticmethod
    def backup_post_job(req_id, job_id, sub_id, json):
        """
        功能描述：清理备份生成的tmp_info文件
        """
        response = ActionResponse()
        log.info(
            f'step 7: start to execute backup_post_job, req_id: {req_id}, job_id: {job_id},  sub_id: {sub_id}')
        param = OceanBaseBackupService.safe_get_info(req_id)
        backup_inst = BackUp(req_id, job_id, sub_id, json, param)
        ret = backup_inst.exec_backup_post_job()
        if not ret:
            log.error(f'fail to execute backup_post_job')
            OceanBaseBackupService.set_error_response(response)
            OceanBaseBackupService.report_error_job_details(job_id, req_id, response, sub_id)
            return False
        report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100,
                                                 logDetail=[],
                                                 taskStatus=SubJobStatusEnum.COMPLETED.value))
        return True

    @staticmethod
    def safe_get_info(pid):
        # 通过pid读取到相应的参数文件
        try:
            param = ParamFileUtil.parse_param_file(pid)
        except Exception as err:
            raise Exception(f"Failed to parse job param file for {err}") from err
        if not param:
            raise Exception(f"Failed to parse job param file is none")
        return param

    @staticmethod
    def backup_post_job_progress(req_id, job_id, sub_id):
        progress_info = SubJobDetails(taskId=job_id, subTaskId=sub_id, taskStatus=SubJobStatusEnum.COMPLETED.value,
                                      logDetail=[], progress=100, dataSize=0, speed=0, extendInfo=None)
        output_result_file(req_id, progress_info.dict(by_alias=True))
        return True

    @staticmethod
    def query_backup_copy(req_id, job_id, sub_id, std_in):
        log.info(f'execute to query_backup_copy, pid: {req_id}, job_id: {job_id}, sub_job_id {sub_id}')
        """
        功能描述： 获取副本信息
        参数：
        @job_info：JobInfo 任务相关信息
        """
        param = ParamFileUtil.parse_param_file(req_id)
        job_info = OceanBaseBackupService.query_back_copy_jobinfo(req_id, param)
        copy_id = ""
        nodes = []
        try:
            nodes = param['job']['protectEnv']['nodes']
        except Exception as ex:
            log.error(ex, exc_info=True)
        try:
            copy_id = param['job']['copy'][0]['id']
        except Exception as ex:
            log.error(ex, exc_info=True)

        data_rep_rsp = []
        # 数据仓
        for item in job_info.data_reps:
            data_rep_rsp.append(
                CopyInfoRepModel(id=item.get('id'),
                                 repositoryType=item.get("repositoryType"),
                                 isLocal=item.get("isLocal"),
                                 protocol="NFS",
                                 remotePath=f"{item.get('remotePath')}/data/{job_info.copy_id}",
                                 remoteHost=item.get("remoteHost"),
                                 extendInfo={
                                     "fsId": item.get("extendInfo", {}).get("fsId")
                                 }).dict(by_alias=True))

        log.debug(f"Construct meta: {job_info.log_format()}")
        # 元数据
        data_rep_rsp.append(
            CopyInfoRepModel(id=job_info.meta_rep.get('id'),
                             repositoryType=job_info.meta_rep.get("repositoryType"),
                             isLocal=job_info.meta_rep.get("isLocal"),
                             protocol="NFS",
                             remotePath=f"{job_info.meta_rep.get('remotePath')}/meta/{job_info.copy_id}",
                             remoteHost=job_info.meta_rep.get("remoteHost"),
                             extendInfo={
                                 "fsId": job_info.meta_rep.get("extendInfo", {}).get("fsId")
                             }).dict(by_alias=True))
        copy_info = Copy(repositories=data_rep_rsp, extendInfo={'nodes': nodes, 'copyId': copy_id}).dict(by_alias=True)
        output_result_file(req_id, copy_info)
        log_detail = LogDetail(logInfo="plugin_task_subjob_success_label", logInfoParam=[sub_id], logLevel=1)
        report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100,
                                                 logDetail=[log_detail],
                                                 taskStatus=SubJobStatusEnum.COMPLETED.value).dict(by_alias=True))
        return True

    @staticmethod
    def query_back_copy_jobinfo(req_id, param):
        job_info = JobInfo(pid=req_id,
                           job_id=DwsParamParse.get_job_id(param),
                           sub_job_id=DwsParamParse.get_sub_job_id(param),
                           copy_id=DwsParamParse.get_copy_id(param),
                           backup_type=DwsParamParse.get_type(param),
                           meta_path=DwsParamParse.get_meta_path(param),
                           meta_rep=DwsParamParse.get_meta_rep(param),
                           cache_path=DwsParamParse.get_cache_path(param),
                           cache_rep=DwsParamParse.get_cache_rep(param),
                           data_reps=DwsParamParse.get_data_reps(param))
        return job_info