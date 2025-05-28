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
import stat

import psutil
from urllib3.exceptions import HTTPError

from common.exception.common_exception import ErrCodeException
from common.util.exec_utils import exec_write_new_file_without_x_permission
from k8s.common.cluster_watcher.cluster_checker import ClusterChecker
from k8s.common.const import Progress
from k8s.common import const
from k8s.common.kubernetes_client.k8s_api_class_core.k8s_backup_pod_api import BackupPod
from k8s.common.utils import check_white_list, clean_sys_data
from k8s.logger import log
from k8s.common.error_code import ErrorCode
from k8s.services.backup.param_mgr import ParamMgr
from k8s.services.backup.backup_task import BackupTask
from common.common import exter_attack, output_result_file, report_job_details
from common.common_models import ActionResult, SubJobDetails, LogDetail
from common.const import ExecuteResultEnum, SubJobStatusEnum, DBLogLevel, ReportDBLabel
from k8s.common.k8s_manager.backup_pod_manager import ImageNameIllegalError


class BackupApi:
    def __init__(self):
        pass

    @staticmethod
    @exter_attack
    def allow_backup_in_local_node(req_id, job_id, sub_job_id, std_in):
        log.info(
            f'Step 1: execute allow_backup_in_local_node, req_id: {req_id}, job_id: {job_id}, sub_job_id: {sub_job_id}')
        param = ParamMgr(req_id, job_id, sub_job_id, True)
        code = ExecuteResultEnum.SUCCESS
        try:
            param.do_check_cluster()
            log.info(f"Check allow backup in local node success, job_id: {job_id}.")
        except ErrCodeException as err:
            log.error(f"Action check_application ErrCodeException failed.{err}")
            code = ExecuteResultEnum.INTERNAL_ERROR
        except Exception as err:
            log.error(f"Action check_application Exception failed.{err}")
            code = ExecuteResultEnum.INTERNAL_ERROR
        output_result_file(req_id, ActionResult(code=code).dict(by_alias=True))
        result = {ExecuteResultEnum.INTERNAL_ERROR: 'failed', ExecuteResultEnum.SUCCESS: 'success'}
        param.clean_backup_auth_info()
        log.info(f"Step 1: execute AllowBackupInLocalNode interface {result.get(code)}")

    @staticmethod
    @exter_attack
    def check_backup_job_type(req_id, job_id, sub_job_id, std_in):
        log.info(
            f'Step 2: start execute check_backup_job_type, req_id: {req_id}, job_id: {job_id},  sub_id: {sub_job_id}')
        param = ParamMgr(req_id, job_id, sub_job_id)
        try:
            if not param.check_backup_job_type():
                log.error(f"Step 2: execute check_backup_job_type interface failed")
                output_result_file(req_id, ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                                        message=ErrorCode.PARAM_FAILED.value).dict(by_alias=True))
            else:
                output_result_file(req_id, ActionResult(code=ExecuteResultEnum.SUCCESS).dict(by_alias=True))
                log.info(f"Step 2: execute check_backup_job_type interface success")
        finally:
            param.clean_backup_auth_info()

    @staticmethod
    @exter_attack
    def query_job_permission(req_id, job_id, sub_job_id, std_in):
        log.info(
            f'Step 3: execute query_job_permission, req_id: {req_id}, job_id: {job_id}, sub_job_id: {sub_job_id}')
        output_result_file(req_id, ActionResult(code=ExecuteResultEnum.SUCCESS).dict(by_alias=True))
        log.info(f"Step 3: execute query_job_permission interface success")

    @staticmethod
    @exter_attack
    def backup_prerequisite(req_id, job_id, sub_job_id, std_in):
        log.info(f'Exec backup prepare, jod id: {job_id}.')
        param_mgr = ParamMgr(req_id, job_id, sub_job_id)
        try:
            BackupPod(param_mgr.job_info.resource.cluster_authentication)
        except Exception as error:
            log.error("Check K8S Cluster Connection Failed!", error)
            param_mgr.clean_backup_auth_info()
            BackupApi._write_progress_file(param_mgr, "Check K8S Cluster Connection Failed!")
            return
        backup_job = BackupTask(param_mgr.job_info)
        cluster_checker = ClusterChecker(kubernetes_back_info=param_mgr.job_info)
        error_info = cluster_checker.before_backup_check()
        if ErrorCode.SUCCESS != error_info.error_code:
            log.error(f"Backup pre check failed, code: {error_info.error_code}, taskId: {job_id}")
            BackupApi._report_job_detail(error_info, job_id, req_id, sub_job_id)
            return
        error_message = 'Prerequisite failed! Prerequisite finish!'
        try:
            if not backup_job.prerequisite():
                log.error(f"Prerequisite failed")
                BackupApi._write_progress_file(param_mgr, error_message)
                return
        except HTTPError as error:
            error_message = 'Network Connect Failed!'
            log.error(f"Prerequisite failed", error)
            BackupApi._write_progress_file(param_mgr, error_message)
            return
        except ImageNameIllegalError as error:
            error_message = "Image Name Illegal!"
            log.error(f"{param_mgr.job_info.image_name} The image name is illegal")
            BackupApi._write_progress_file(param_mgr, error_message)
            return
        except ErrCodeException as error:
            log.error(f"Prerequisite failed")
            BackupApi._report_job_detail(error, job_id, req_id, sub_job_id)
            return
        except Exception as error:
            error_message = 'Exception ERROR!'
            log.exception(f"Prerequisite failed, {error}")
            BackupApi._write_progress_file(param_mgr, error_message)
            return
        finally:
            param_mgr.clean_backup_auth_info()
        progress_file = os.path.join(param_mgr.job_info.cache_repo.local_path, "BackupPrerequisiteProgress")
        log.info(f"Prerequisite success, creat file {progress_file}")
        BackupApi.write_progress_file(progress_file, 'Prerequisite finish!')
        return

    @staticmethod
    @exter_attack
    def backup_prerequisite_progress(req_id, job_id, sub_id, std_in):
        # 当前根据是否存在BackupPrerequisiteProgress文件来上报前置任务进度
        param_mgr = ParamMgr(req_id, job_id, sub_id)
        try:
            progress_file = os.path.join(param_mgr.job_info.cache_repo.local_path, "BackupPrerequisiteProgress")
            progress_failed_file = os.path.join(param_mgr.job_info.cache_repo.local_path,
                                                "BackupPrerequisiteProgressFailed")
            log.debug(f"Progress file {progress_file} Progress failed file {progress_failed_file}")
            if not os.path.exists(progress_file) and not os.path.exists(progress_failed_file):
                job_status = SubJobStatusEnum.RUNNING.value
                log.info(f"Prerequisite running, status: {job_status}, pid: {req_id}, job_id: {job_id}.")
                output = SubJobDetails(taskId=job_id, subTaskId=sub_id,
                                       taskStatus=job_status, progress=0, logDetail=None)
                output_result_file(req_id, output.dict(by_alias=True))
                report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_id, logDetail=[],
                                                         progress=Progress.PROGRESS_ZERO, taskStatus=job_status))
            elif os.path.exists(progress_file):
                job_status = SubJobStatusEnum.COMPLETED.value
                log.info(f"Finished！ Job status: {job_status}, pid: {req_id}, job_id: {job_id}.")
                output_result_file(req_id, ActionResult(code=ExecuteResultEnum.SUCCESS).dict(by_alias=True))
                report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_id, logDetail=[],
                                                         taskStatus=job_status, progress=Progress.PROGRESS_THIRTY))
            else:
                job_status = SubJobStatusEnum.FAILED.value
                log.info(f"Failed！ Job status: {job_status}, pid: {req_id}, job_id: {job_id}.")
                with open(progress_failed_file, "r", encoding='UTF-8') as file_stream:
                    data = file_stream.read()
                message = "Execute Backup Prerequisite failed!"
                error_code = ErrorCode.OPERATION_FAILED
                log_info = ReportDBLabel.PRE_REQUISIT_FAILED
                log_info_param = [param_mgr.job_info.image_name]
                if "Image Name Illegal!" in data:
                    log.error(f"data: {data}")
                    log_info = const.K8sReportLabel.PVC_PRE_REGISTERED_IMAGE_NAME_ILLEGAL
                    message = "Image Name Illegal!"
                elif 'Network Connect Failed!' in data:
                    log.error(f"data: {data}")
                    message = "Network Connect Failed!"
                    error_code = ErrorCode.ERROR_NETWORK_CONNECT_TIMEOUT
                elif 'The Pod Status Is Not Running!' in data:
                    log.error(f"data: {data}")
                    message = "The Pod Status Is Not Running!"
                elif 'Exception ERROR!' in data:
                    log.error(f"data: {data}")
                    message = "Exception ERROR!"
                output_result_file(req_id, ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR).dict(by_alias=True))
                log_detail = LogDetail(logInfo=log_info, logLevel=DBLogLevel.ERROR.value, logInfoParam=log_info_param,
                                       logDetail=error_code, logDetailParam=["Backup", message])
                report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_id, logDetail=[log_detail],
                                                         progress=Progress.PROGRESS_ZERO, taskStatus=job_status))
        finally:
            param_mgr.clean_backup_auth_info()

    @staticmethod
    @exter_attack
    def backup_gen_sub_job(req_id, job_id, sub_job_id, std_in):
        log.info(
            f'Step 5: execute backup_gen_sub_job, req_id: {req_id}, job_id: {job_id}, sub_job_id: {sub_job_id}')
        sub_job_array = []
        param_mgr = ParamMgr(req_id, job_id, sub_job_id)
        try:
            sub_job = param_mgr.get_sub_job()
            sub_job_array.append(sub_job)
            log.debug(f"Sub_job arrary!:{sub_job_array}")
            output_result_file(req_id, sub_job_array)
            log.info(f"step 5: execute backup_gen_sub_job interface success")
        finally:
            param_mgr.clean_backup_auth_info()

    @staticmethod
    @exter_attack
    def backup(req_id, job_id, sub_job_id, std_in):
        log.info(
            f'Step 6: execute backup, req_id: {req_id}, job_id: {job_id}, sub_job_id: {sub_job_id}')
        param_mgr = ParamMgr(req_id, job_id, sub_job_id)
        path_dict, ip_dict = param_mgr.get_path_ip_dict()
        backup_task = BackupTask(param_mgr.job_info, path_dict, ip_dict)
        try:
            if not backup_task.backup_metadata(param_mgr.job_info.data_repo.local_path):
                log.error(f"Backup metadata failed! Sub_id={sub_job_id}")
                BackupApi.backup_report_error(req_id=req_id, job_id=job_id, sub_job_id=sub_job_id,
                                              report_info=backup_task.report_info)
                return ExecuteResultEnum.INTERNAL_ERROR
            log.info(f"Backup metadata succeed! Sub_id={sub_job_id}")
            if not backup_task.backup_data():
                log.error(f"Backup failed! Sub_id={sub_job_id}")
                BackupApi.backup_report_error(req_id=req_id, job_id=job_id, sub_job_id=sub_job_id,
                                              report_info=backup_task.report_info)
                return ExecuteResultEnum.INTERNAL_ERROR
            else:
                log.info(f"Backup succeed! Sub_id={sub_job_id}")
                log_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS,
                                       logInfoParam=[sub_job_id], logLevel=DBLogLevel.INFO)
                report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_job_id,
                                                         progress=Progress.PROGRESS_FORTY,
                                                         logDetail=[log_detail],
                                                         dataSize=backup_task.data_size,
                                                         taskStatus=SubJobStatusEnum.COMPLETED.value))
                return ExecuteResultEnum.SUCCESS
        except ErrCodeException as err:
            log.exception(f"Backup failed! Err:{err}, Sub_id={sub_job_id}")
            backup_task.report_info.log_detail = err.error_code
            backup_task.report_info.log_detail_param = err.parameter_list
            BackupApi.backup_report_error(req_id=req_id, job_id=job_id, sub_job_id=sub_job_id,
                                          report_info=backup_task.report_info)
            return ExecuteResultEnum.INTERNAL_ERROR
        except Exception as err:
            log.exception(f"Backup failed! Err:{err}, Sub_id={sub_job_id}")
            BackupApi.backup_report_error(req_id=req_id, job_id=job_id, sub_job_id=sub_job_id,
                                          report_info=backup_task.report_info)
            return ExecuteResultEnum.INTERNAL_ERROR
        finally:
            param_mgr.clean_backup_auth_info()

    @staticmethod
    def backup_report_error(req_id, job_id, sub_job_id, report_info):
        log_detail = LogDetail(logInfo=ReportDBLabel.BACKUP_SUB_FAILED,
                               logInfoParam=[sub_job_id], logLevel=DBLogLevel.ERROR,
                               logDetail=report_info.log_detail, logDetailParam=report_info.log_detail_param)
        report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_job_id,
                                                 progress=Progress.PROGRESS_ZERO,
                                                 logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value))

    @staticmethod
    @exter_attack
    def backup_post_job(req_id, job_id, sub_job_id, std_in):
        log.info(
            f'Step 7: execute backup_post_job, req_id: {req_id}, job_id: {job_id}, sub_job_id: {sub_job_id}')
        param_mgr = ParamMgr(req_id, job_id, sub_job_id)
        try:
            path_dict, ip_dict = param_mgr.get_path_ip_dict()
            backup_task = BackupTask(param_mgr.job_info, path_dict, ip_dict)
            if not backup_task.clean_task(param_mgr.job_info.cache_repo.local_path):
                log.error(f"Clean task failed! job_id={job_id}")
                output_result_file(req_id, ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR).dict(by_alias=True))
                return ExecuteResultEnum.INTERNAL_ERROR
            output_result_file(req_id, ActionResult(code=ExecuteResultEnum.SUCCESS).dict(by_alias=True))
            log.info(f"Step 7: execute backup_post_job interface success")
            return ExecuteResultEnum.SUCCESS
        finally:
            param_mgr.clean_backup_auth_info()
            clean_sys_data()

    @staticmethod
    @exter_attack
    def backup_post_job_progress(req_id, job_id, sub_id, std_in):
        progress_info = SubJobDetails(taskId=job_id, subTaskId=sub_id, taskStatus=SubJobStatusEnum.COMPLETED.value,
                                      logDetail=[], progress=Progress.PROGRESS_THIRTY,
                                      dataSize=0, speed=0, extendInfo=None)
        output_result_file(req_id, progress_info.dict(by_alias=True))
        return ExecuteResultEnum.SUCCESS

    @staticmethod
    def write_progress_file(file_name: str, message: str):
        # 注意根据具体业务的需要设置文件读写方式
        # # 注意根据具体业务的需要设置文件权限
        exec_write_new_file_without_x_permission(file_name, message, json_flag=False)

    @staticmethod
    @exter_attack
    def abort_job(req_id, job_id, sub_job_id, std_in):
        """
        终止备份任务
        :return: boolean，True代表终止成功，False代表终止失败
        """
        param_mgr = ParamMgr(req_id, job_id, sub_job_id)
        param_mgr.clean_backup_auth_info()
        check_white_list([param_mgr.job_info.data_repo.local_path])
        aborting_file = os.path.join(param_mgr.job_info.data_repo.local_path, "abort.ing")
        try:
            pathlib.Path(aborting_file).touch()
        except Exception as ex:
            log.error(f"Failed to create aborting file, job id: {job_id}. "
                      f"Exception info: {ex}.")
            report_job_details(req_id,
                               SubJobDetails(taskId=job_id,
                                             subTaskId=sub_job_id,
                                             progress=Progress.PROGRESS_ZERO,
                                             logDetail=[],
                                             taskStatus=SubJobStatusEnum.ABORTED_FAILED.value))
            return False
        pid_list = psutil.pids()
        for pid in pid_list:
            try:
                process = psutil.Process(pid)
            except Exception as err:
                log.error(f"Get process err: {err}.")
                continue
            cmd = process.cmdline()
            if 'python3' in cmd and (job_id in cmd and
                                     req_id not in cmd):
                process.kill()
                log.info(f"The backup task has been terminated, job id: {job_id}.")
                break
        aborted_file = os.path.join(param_mgr.job_info.data_repo.local_path, "abort.done")
        try:
            os.rename(aborting_file, aborted_file)
        except Exception as ex:
            log.error(f"Failed to create aborted file, job id: {job_id}. "
                      f"Exception info: {ex}.")
            report_job_details(req_id, SubJobDetails(taskId=job_id,
                                                     subTaskId=sub_job_id,
                                                     progress=Progress.PROGRESS_ZERO,
                                                     logDetail=[],
                                                     taskStatus=SubJobStatusEnum.ABORTED_FAILED.value))
            return False
        report_job_details(req_id, SubJobDetails(taskId=job_id,
                                                 subTaskId=sub_job_id,
                                                 progress=Progress.PROGRESS_ZERO,
                                                 logDetail=[],
                                                 taskStatus=SubJobStatusEnum.ABORTED.value))
        log.info(f"Succeed to abort backup job, job id: {job_id}.")
        return True

    @staticmethod
    def _write_progress_file(param_mgr, error_message):
        progress_failed_file = os.path.join(param_mgr.job_info.cache_repo.local_path,
                                            "BackupPrerequisiteProgressFailed")
        BackupApi.write_progress_file(progress_failed_file, error_message)

    @staticmethod
    def _report_job_detail(error, job_id, req_id, sub_job_id):
        log_detail = LogDetail(logLevel=DBLogLevel.ERROR.value, logDetail=error.error_code,
                               logDetailParam=error.parameter_list)
        report_job_details(req_id,
                           SubJobDetails(taskId=job_id, subTaskId=sub_job_id, progress=Progress.PROGRESS_ZERO,
                                         logDetail=[log_detail], taskStatus=SubJobStatusEnum.FAILED.value))
