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
import stat
import time

from common.common_models import ActionResult, SubJobDetails, LogDetail
from common.const import ExecuteResultEnum, SubJobStatusEnum, DBLogLevel, ReportDBLabel
from common.exception.common_exception import ErrCodeException
from common.util.exec_utils import exec_write_new_file_without_x_permission
from k8s.common.const import Progress
from k8s.common.utils import clean_sys_data
from k8s.logger import log
from common.common import exter_attack, output_result_file, report_job_details
from k8s.services.restore.restore_para_mgr import RestoreParamMgr
from k8s.services.restore.restore_task import RestoreTask


class RestoreApi:
    def __init__(self):
        pass

    @staticmethod
    @exter_attack
    def allow_restore_in_local_node(req_id, job_id, sub_job_id, std_in):
        log.info(
            f'Exec allow_restore_in_local_node, req_id: {req_id}, job_id: {job_id}, sub_job_id: {sub_job_id}')
        param = RestoreParamMgr(req_id, job_id, sub_job_id, True)
        code = ExecuteResultEnum.SUCCESS
        try:
            param.do_check_cluster()
            log.info(f"Check allow restore in local node success, job_id: {job_id}.")
        except ErrCodeException as err:
            log.error(f"Action check_application ErrCodeException failed.{err}")
            code = ExecuteResultEnum.INTERNAL_ERROR
        except Exception as err:
            log.error(f"Action check_application Exception failed.{err}")
            code = ExecuteResultEnum.INTERNAL_ERROR
        output_result_file(req_id, ActionResult(code=code).dict(by_alias=True))
        result = {ExecuteResultEnum.INTERNAL_ERROR: 'failed', ExecuteResultEnum.SUCCESS: 'success'}
        param.clean_restore_auth_info()
        log.info(f"Exec allow_restore_in_local_node interface {result.get(code)}")
        return

    @staticmethod
    @exter_attack
    def restore_prerequisite(req_id, job_id, sub_job_id, std_in):
        log.info(f'Exec restore prepare, jod id: {job_id}.')
        restore_param = RestoreParamMgr(req_id, job_id, sub_job_id)
        try:
            restore_flow = RestoreTask(restore_param)
            restore_flow.prerequisite()
            progress_file = os.path.join(restore_param.job_info.cache_repo.local_path,
                                         "RestorePrerequisiteProgress")
            log.info(f"Prerequisite success, creat file {progress_file}")
            RestoreApi.write_progress_file(progress_file, 'Prerequisite finish!')
            return
        except ErrCodeException as error:
            log.error(f"Prerequisite failed")
            log_detail = LogDetail(logLevel=DBLogLevel.ERROR.value, logDetail=error.error_code,
                                   logDetailParam=error.parameter_list)
            report_job_details(req_id,
                               SubJobDetails(taskId=job_id, subTaskId=sub_job_id, progress=Progress.PROGRESS_ZERO,
                                             logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value))
            return
        except Exception as e:
            log.exception(f'Pre error! {e}, task id:{req_id}')
            job_status = SubJobStatusEnum.FAILED.value
            progress = 100
            log.info(f"Failed！ Job status: {job_status}, pid: {req_id}, job_id: {job_id}.")
            report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_job_id, progress=progress,
                                                     logDetail=[],
                                                     taskStatus=job_status))
            output_result_file(req_id, ActionResult(code=ExecuteResultEnum.SUCCESS).dict(by_alias=True))
            return
        finally:
            restore_param.clean_restore_auth_info()

    @staticmethod
    @exter_attack
    def restore_prerequisite_progress(req_id, job_id, sub_job_id, std_in):
        restore_param = RestoreParamMgr(req_id, job_id, sub_job_id)
        try:
            progress_file = os.path.join(restore_param.job_info.cache_repo.local_path, "RestorePrerequisiteProgress")
            log.info(f"Progress file {progress_file}")
            if os.path.exists(progress_file):
                job_status = SubJobStatusEnum.COMPLETED.value
                progress = 100
                log.info(f"Prerequisite finish, job status: {job_status}, pid: {req_id}, job_id: {job_id}, "
                         f"sub job id {sub_job_id}.")
            else:
                job_status = SubJobStatusEnum.RUNNING.value
                progress = 0
                log.info(f"Prerequisite running, status: {job_status}, pid: {req_id}, job_id: {job_id}.")
            output = SubJobDetails(taskId=job_id, subTaskId=sub_job_id,
                                   taskStatus=job_status, progress=progress, logDetail=None)
            output_result_file(req_id, output.dict(by_alias=True))
        finally:
            restore_param.clean_restore_auth_info()

    @staticmethod
    @exter_attack
    def restore_gen_sub_job(req_id, job_id, sub_job_id, std_in):
        log.info(
            f'Step 5: execute backup_gen_sub_job, req_id: {req_id}, job_id: {job_id}, sub_job_id: {sub_job_id}')
        sub_job_array = []
        restore_param = RestoreParamMgr(req_id, job_id, sub_job_id)
        try:
            sub_job = restore_param.get_sub_job()
            sub_job_array.append(sub_job)
            log.debug(f"Sub_job array!:{sub_job_array}")
            output_result_file(req_id, sub_job_array)
            log.info(f"step 5: execute backup_gen_sub_job interface success")
            return
        except Exception:
            log.error(f'Execute backup_gen_sub_job failed! req_id: {req_id}, job_id: {job_id}, '
                      f'sub_job_id: {sub_job_id}')
            output_result_file(req_id, ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR).dict(by_alias=True))
            return
        finally:
            restore_param.clean_restore_auth_info()

    @staticmethod
    @exter_attack
    def restore(req_id, job_id, sub_job_id, std_in):
        log.info(f'Begin exec restore, jod id: {job_id}.')
        restore_param = RestoreParamMgr(req_id, job_id, sub_job_id)
        try:
            restore_flow = RestoreTask(restore_param)
            if not restore_flow.restore():
                log.error(f"Restore failed. {job_id} Sub_id={sub_job_id}")
                RestoreApi.restore_report_error(req_id=req_id, job_id=job_id, sub_job_id=sub_job_id)
                output_result_file(req_id, ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR).dict(by_alias=True))
                return
            log.info(f"Restore succeed! Sub_id={sub_job_id}")
            log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS,
                                   logInfoParam=[sub_job_id], logLevel=DBLogLevel.INFO)
            report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_job_id,
                                                     progress=Progress.PROGRESS_FORTY,
                                                     logDetail=[log_detail],
                                                     taskStatus=SubJobStatusEnum.COMPLETED.value))
            output_result_file(req_id, ActionResult(code=ExecuteResultEnum.SUCCESS).dict(by_alias=True))
            return
        except ErrCodeException as error:
            log.exception(f"Restore failed. {job_id} Sub_id={sub_job_id}, ERR {error}")
            log_detail = LogDetail(logInfo=ReportDBLabel.RESTORE_SUB_FAILED, logInfoParam=[sub_job_id],
                                   logLevel=DBLogLevel.ERROR, logDetail=error.error_code,
                                   logDetailParam=error.parameter_list)
            report_job_details(req_id,
                               SubJobDetails(taskId=job_id, subTaskId=sub_job_id, progress=Progress.PROGRESS_ZERO,
                                             logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value))
            return
        except Exception as e:
            log.exception(f"Restore failed. {job_id} Sub_id={sub_job_id}, ERR {e}")
            RestoreApi.restore_report_error(req_id=req_id, job_id=job_id, sub_job_id=sub_job_id)
            output_result_file(req_id, ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR).dict(by_alias=True))
            return
        finally:
            restore_param.clean_restore_auth_info()

    @staticmethod
    @exter_attack
    def restore_report_error(req_id, job_id, sub_job_id):
        log_detail = LogDetail(logInfo=ReportDBLabel.RESTORE_SUB_FAILED,
                               logInfoParam=[sub_job_id], logLevel=DBLogLevel.ERROR)
        report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_job_id,
                                                 progress=Progress.PROGRESS_ZERO,
                                                 logDetail=[log_detail],
                                                 taskStatus=SubJobStatusEnum.FAILED.value))

    @staticmethod
    @exter_attack
    def restore_post_job(req_id, job_id, sub_job_id, std_in):
        log.info(
            f'Step 7: execute backup_post_job, req_id: {req_id}, job_id: {job_id}, sub_job_id: {sub_job_id}')
        restore_param = RestoreParamMgr(req_id, job_id, sub_job_id)
        try:
            restore_flow = RestoreTask(restore_param)
            if not restore_flow.post():
                log.error(f"Clean task failed! job_id={job_id}")
                output_result_file(req_id, ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR).dict(by_alias=True))
                return
            progress_info = SubJobDetails(taskId=job_id, subTaskId=sub_job_id,
                                          taskStatus=SubJobStatusEnum.COMPLETED.value,
                                          logDetail=[], progress=100, dataSize=0, speed=0, extendInfo=None)
            report_job_details(req_id, progress_info)
            output_result_file(req_id, ActionResult(code=ExecuteResultEnum.SUCCESS).dict(by_alias=True))
            log.info(f"Step 7: execute backup_post_job interface success")
            return
        finally:
            restore_param.clean_restore_auth_info()
            clean_sys_data()

    @staticmethod
    @exter_attack
    def restore_post_job_progress(req_id, job_id, sub_job_id, std_in):
        progress_info = SubJobDetails(taskId=job_id, subTaskId=sub_job_id, taskStatus=SubJobStatusEnum.COMPLETED.value,
                                      logDetail=[], progress=Progress.PROGRESS_THIRTY,
                                      dataSize=0, speed=0, extendInfo=None)
        report_job_details(req_id, progress_info)
        output_result_file(req_id, ActionResult(code=ExecuteResultEnum.SUCCESS).dict(by_alias=True))
        return

    @staticmethod
    def write_progress_file(file_name: str, message: str):
        # 注意根据具体业务的需要设置文件读写方式
        # 注意根据具体业务的需要设置文件权限
        exec_write_new_file_without_x_permission(file_name, message, json_flag=False)
