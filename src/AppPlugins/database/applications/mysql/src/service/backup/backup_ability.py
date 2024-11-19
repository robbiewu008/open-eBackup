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

from common.common import exter_attack, output_result_file, report_job_details
from common.common_models import SubJobDetails, LogDetail
from common.const import ExecuteResultEnum, SubJobStatusEnum, DBLogLevel, ReportDBLabel
from mysql import log
from mysql.src.common.constant import MySQLClusterType, MySQLType, MySQLJsonConstant, MySQLExecPower
from mysql.src.service.backup.backup_param import BackupParam
from mysql.src.service.backup.backup_service import BackupService, SingleDatabaseService
from mysql.src.service.backup.eapp_backup_service import EAPPBackupService
from mysql.src.utils.common_func import ActionResponse, ErrCodeException, PermissionInfo


def build_backup_service(job_id, sub_id, param: BackupParam):
    if param.cluster_type == MySQLClusterType.EAPP:
        return EAPPBackupService(job_id, sub_id, param)
    elif param.app_type == MySQLType.SUBTYPE:
        return SingleDatabaseService(job_id, sub_id, param)
    return BackupService(job_id, sub_id, param)


def handle_exception(pid, job_id, error: Exception, error_label: str, log_detail_param=None):
    response = ActionResponse(code=ExecuteResultEnum.INTERNAL_ERROR)
    log_detail = LogDetail(logInfo=error_label, logLevel=DBLogLevel.ERROR)
    if not log_detail_param:
        log_detail.log_detail_param = log_detail_param
    if isinstance(error, ErrCodeException):
        response.body_err = error.error_code
        response.message = error.error_message
        log_detail.log_detail = error.error_code
        log_detail.log_detail_param = error.parameter_list
    output_result_file(pid, response.dict(by_alias=True))
    report_job_details(pid, SubJobDetails(taskId=job_id, progress=100, logDetail=[log_detail],
                                          taskStatus=SubJobStatusEnum.FAILED.value))


class BackupAbility:

    @staticmethod
    @exter_attack
    def query_job_permission(pid, job_id, sub_id, data):
        with BackupParam(pid) as backup_param:
            permission_info = PermissionInfo(user="root", file_mode="0700")
            if backup_param.job_type == MySQLJsonConstant.LIVEMOUNT:
                extend_dict = {MySQLJsonConstant.PATH: MySQLExecPower.MYSQL_LIVEMOUNT_PATH}
                permission_info.extend_info = extend_dict
            output_result_file(pid, permission_info.dict(by_alias=True))
            log.info(f"execute query_job_permission interface success,pid:{pid},job_id:{job_id}")

    @staticmethod
    @exter_attack
    def check_backup_job_type(pid, job_id, sub_id, data):
        response = ActionResponse(code=ExecuteResultEnum.SUCCESS)
        output_result_file(pid, response.dict(by_alias=True))

    @staticmethod
    @exter_attack
    def allow_backup_in_local_node(pid, job_id, sub_id, data):
        try:
            with BackupParam(pid) as backup_param:
                backup_service = build_backup_service(job_id, sub_id, backup_param)
                backup_service.allow_backup_in_local_node()
                response = ActionResponse(code=ExecuteResultEnum.SUCCESS)
                output_result_file(pid, response.dict(by_alias=True))
                log.info(f"execute AllowBackupInLocalNode interface success,job_id:{job_id}")
        except Exception as error:
            log.error(f"execute allow_backup_in_local_node interface failed,job_id:{job_id},error:{error}")
            handle_exception(pid, job_id, error, "plugin_generate_subjob_fail_label")

    @staticmethod
    @exter_attack
    def backup_prerequisite(pid, job_id, sub_id, data):
        try:
            with BackupParam(pid) as backup_param:
                backup_service = build_backup_service(job_id, sub_id, backup_param)
                backup_service.backup_prerequisite()
                response = ActionResponse()
                output_result_file(pid, response.dict(by_alias=True))
                log_detail = LogDetail(logInfo="plugin_execute_prerequisit_task_success_label",
                                       logLevel=DBLogLevel.INFO.value)
                sub_dict = SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, logDetail=[log_detail],
                                         taskStatus=SubJobStatusEnum.COMPLETED.value)
                report_job_details(pid, sub_dict)
                log.info(f"execute backup_prerequisite interface success,job_id:{job_id}")
        except Exception as error:
            log.error(f"execute backup prerequisite interface failed,job_id:{job_id},{error}")
            handle_exception(pid, job_id, error, "plugin_execute_prerequisit_task_fail_label")

    @staticmethod
    @exter_attack
    def backup(pid, job_id, sub_id, data):
        with BackupParam(pid) as backup_param:
            backup_service = build_backup_service(job_id, sub_id, backup_param)
            backup_service.exec_backup()

    @staticmethod
    @exter_attack
    def backup_post_job(pid, job_id, sub_id, data):
        try:
            with BackupParam(pid) as backup_param:
                backup_service = build_backup_service(job_id, sub_id, backup_param)
                backup_service.backup_post()
                response = ActionResponse()
                output_result_file(pid, response.dict(by_alias=True))
                sub_dict = SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, logDetail=[],
                                         taskStatus=SubJobStatusEnum.COMPLETED.value)
                report_job_details(pid, sub_dict)
                log.info(f"execute backup post interface success,job_id:{job_id}")
        except Exception as error:
            handle_exception(pid, job_id, error, ReportDBLabel.POST_TASK_FAIL,
                             log_detail_param=[backup_param.get_current_ip(), sub_id])
            log.error(f"execute backup prerequisite interface failed,job_id:{job_id}")

    @staticmethod
    @exter_attack
    def query_backup_copy(pid, job_id, sub_id, data):
        with BackupParam(pid) as backup_param:
            backup_service = build_backup_service(job_id, sub_id, backup_param)
            copy_json = backup_service.query_backup_copy()
            output_result_file(pid, copy_json)
