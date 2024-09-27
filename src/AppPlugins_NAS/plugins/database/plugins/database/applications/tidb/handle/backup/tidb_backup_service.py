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

from common.common_models import SubJobDetails, LogDetail
from common.const import DBLogLevel, ExecuteResultEnum, SubJobStatusEnum
from common.common import output_result_file
from goldendb.schemas.glodendb_schemas import ActionResponse
from tidb.common.const import EnvName, ErrorCode
from tidb.common.safe_get_information import ResourceParam
from tidb.common.tidb_common import get_agent_uuids, get_tidb_structure, report_job_details
from tidb.handle.backup.exec_backup import BackUp
from tidb.logger import log


class TiDBBackupService(object):
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
        EnvName.IAM_USERNAME = "job_protectEnv_auth_authKey"
        EnvName.IAM_PASSWORD = "job_protectEnv_auth_authPwd"
        param_inst = ResourceParam(req_id)
        param = param_inst.get_param()
        agent_uuids = get_agent_uuids(param)
        protect_obj_extend_info = param.get("job", {}).get("protectObject", {}).get("extendInfo", {})
        cluster_info_str = protect_obj_extend_info.get("clusterInfoList")
        cluster_info = get_tidb_structure(cluster_info_str)
        tiup_uuid = [protect_obj_extend_info.get("tiupUuid")]
        tikvs = cluster_info.tikv_nodes
        tikv_uuids = [tikv.get("hostManagerResourceUuid", "") for tikv in tikvs]
        tiflashes = cluster_info.tiflash_nodes
        tiflash_uuids = [tiflash.get("hostManagerResourceUuid", "") for tiflash in tiflashes]
        exec_uuids = tiup_uuid + tikv_uuids + tiflash_uuids
        for exec_uuid in exec_uuids:
            if exec_uuid not in agent_uuids:
                response = ActionResponse(code=ExecuteResultEnum.INTERNAL_ERROR,
                                          bodyErr=ErrorCode.ERR_ENVIRONMENT,
                                          message=f"{exec_uuid} is offline")
                log_detail = LogDetail(logInfo="plugin_generate_subjob_fail_label", logLevel=DBLogLevel.ERROR)
                report_job_details(req_id, SubJobDetails(taskId=job_id, progress=100, logDetail=[log_detail],
                                                         taskStatus=SubJobStatusEnum.FAILED.value))
                output_result_file(req_id, response.dict(by_alias=True))
                return False
        response = ActionResponse(code=ExecuteResultEnum.SUCCESS)
        output_result_file(req_id, response.dict(by_alias=True))
        log.info(f"step 1: execute AllowBackupInLocalNode interface success")
        return True

    @staticmethod
    def check_backup_job_type(req_id, job_id, sub_id, json):
        """
        功能描述：当此次任务是日志备份，且之前没有任何备份副本，日志备份失败
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        返回值：
        """
        log.info(
            f'step 2: start execute backup_gen_sub_job, req_id: {req_id}, job_id: {job_id},  sub_id: {sub_id}')
        response = ActionResponse(code=ExecuteResultEnum.SUCCESS)
        output_result_file(req_id, response.dict(by_alias=True))
        log.info(f"step 2: execute check_backup_job_type interface success")
        return True

    @staticmethod
    def backup_prerequisite(req_id, job_id, sub_id, json):
        """
        功能描述：备份前置任务进度上报
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        @sub_id：子任务ID
        返回值：
        """
        log.info(f'step 3: execute backup_prerequisite,req_id:{req_id} job_id:{job_id}')
        response = ActionResponse()
        log_detail = LogDetail(logInfo="plugin_execute_prerequisit_task_success_label",
                               logLevel=DBLogLevel.INFO.value)
        sub_dict = SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, logDetail=[log_detail],
                                 taskStatus=SubJobStatusEnum.COMPLETED.value)

        output_result_file(req_id, response.dict(by_alias=True))
        report_job_details(req_id, sub_dict.dict(by_alias=True))
        log.info(f'step 3: finish to execute backup_prerequisite,req_id:{req_id} job_id:{job_id}')
        return True

    @staticmethod
    def backup_gen_sub_job(req_id, job_id, sub_id, json):
        """
        功能描述：备份扫描任务
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        返回值：
        """
        log.info(
            f'step 4: start execute backup_gen_sub_job, req_id: {req_id}, job_id: {job_id},  sub_id: {sub_id}')
        param_inst = ResourceParam(req_id)
        param = param_inst.get_param()
        backup_inst = BackUp(req_id, job_id, sub_id, json, param)
        backup_inst.gen_sub_job()
        return True

    @staticmethod
    def backup(req_id, job_id, sub_id, std_in):
        """
        功能描述：备份任务
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        @sub_id：子任务ID
        返回值：
        """
        log.info(f"step 5: start execute backup")
        param_inst = ResourceParam(req_id)
        param = param_inst.get_param()
        backup_inst = BackUp(req_id, job_id, sub_id, std_in, param)
        backup_inst.exec_sub_jobs()
        return True

    @staticmethod
    def query_backup_copy(req_id, job_id, sub_id, std_in):
        """
        功能描述：查询上报副本
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        @sub_id：子任务ID
        返回值：
        """
        return True

    @staticmethod
    def backup_post_job(req_id, job_id, sub_id, std_in):
        """
        功能描述：执行备份后置任务任务
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        返回值：
        """
        log.info(f"step 6: start execute backup_post_job")
        param_inst = ResourceParam(req_id)
        param = param_inst.get_param()
        backup_inst = BackUp(req_id, job_id, sub_id, std_in, param)
        backup_inst.do_post_job()
        response = ActionResponse(code=ExecuteResultEnum.SUCCESS)
        output_result_file(req_id, response.dict(by_alias=True))
        return True

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
