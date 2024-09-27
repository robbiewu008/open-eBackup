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

from common.const import ExecuteResultEnum, RepositoryDataTypeEnum, SubJobStatusEnum, ExecuteResultEnum, DBLogLevel
from common.parse_parafile import ParamFileUtil
from common.common import output_result_file
from common.common_models import Copy, CopyInfoRepModel, SubJobDetails, LogDetail, ActionResult
from tdsql.common.const import JobInfo, ErrorCode
from tdsql.common.tdsql_common import report_job_details, get_agent_uuids, get_nodes
from tdsql.common.safe_get_information import ResourceParam
from tdsql.handle.backup.exec_backup import BackUp
from tdsql.logger import log
from dws.commons.dws_param_parse import DwsParamParse


class TdsqlBackupService(object):
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
        response = ActionResult(code=ExecuteResultEnum.SUCCESS)
        try:
            file_content = ParamFileUtil.parse_param_file(req_id)
        except Exception as exception:
            log.error(exception, exc_info=True)
            return False
        agent_uuids = get_agent_uuids(file_content)
        log.info(f"agent_uuids {agent_uuids}")
        nodes = get_nodes(file_content)
        log.info(f"nodes {nodes}")
        for node in nodes:
            node_id = node.get("parentUuid", "")
            log.info(f"node_id {node_id}")
            if node_id not in agent_uuids:
                response = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR,
                                        bodyErr=ErrorCode.ERR_ENVIRONMENT,
                                        message=f"{node_id} is offline")
                log_detail = LogDetail(logInfo="plugin_generate_subjob_fail_label", logLevel=DBLogLevel.ERROR)
                report_job_details(req_id, SubJobDetails(taskId=job_id, progress=100, logDetail=[log_detail],
                                                         taskStatus=SubJobStatusEnum.FAILED.value).dict(by_alias=True))
                output_result_file(req_id, response.dict(by_alias=True))
                return False
        output_result_file(req_id, response.dict(by_alias=True))
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
            f'step 2-1: start execute backup_gen_sub_job, req_id: {req_id}, job_id: {job_id},  sub_id: {sub_id}')
        param_inst = ResourceParam(req_id)
        param = param_inst.get_param()
        backup_inst = BackUp(req_id, job_id, sub_id, json, param)
        backup_inst.check_backup_job_type()

    @staticmethod
    def backup_pre_job(req_id, job_id, sub_id, json):
        """
        功能描述：备份前置任务, 主任务执行, 检查参数是否符合要求, 配置roach映射关系
        参数：
         @req_id： 请求ID
         @job_id： 主任务任务ID
        返回值：
        """
        log.info(f'step 2-2: execute backup_pre_job,req_id:{req_id} job_id:{job_id}')

        response = ActionResult(code=ExecuteResultEnum.SUCCESS)

        # 1、验证参数
        try:
            file_content = ParamFileUtil.parse_param_file(req_id)
        except Exception as exception:
            TdsqlBackupService.set_error_response(response)
            log.error(exception, exc_info=True)
            output_result_file(req_id, response.dict(by_alias=True))
            return False

        backup_inst = BackUp(req_id, job_id, sub_id, json, file_content)

        backup_inst.backup_pre_job()

        log_detail = LogDetail(logInfo="plugin_execute_prerequisit_task_success_label", logLevel=DBLogLevel.INFO)
        sub_dict = SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, logDetail=[log_detail],
                                 taskStatus=SubJobStatusEnum.COMPLETED.value)

        output_result_file(req_id, response.dict(by_alias=True))
        report_job_details(req_id, sub_dict.dict(by_alias=True))
        log.info(f'step 2-3: finish to execute backup_pre_job,req_id:{req_id} job_id:{job_id}')
        return True

    @staticmethod
    def backup_gen_sub_job(req_id, job_id, sub_id, json):
        """
        功能描述：划分子任务
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        @sub_id：子任务ID
        @json: 任务参数
        返回值：
        """
        log.info(
            f'start execute backup_gen_sub_job, req_id: {req_id}, job_id: {job_id},  sub_id: {sub_id}')
        param_inst = ResourceParam(req_id)
        param = param_inst.get_param()
        log.info(f"backup_gen_sub_job param: {param}")
        backup_inst = BackUp(req_id, job_id, sub_id, json, param)
        backup_inst.gen_sub_job()
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

    @staticmethod
    def query_backup_copy(req_id, job_id, sub_job_id):
        log.info(f'execute to query_backup_copy, pid: {req_id}, job_id: {job_id}, sub_job_id {sub_job_id}')
        """
        功能描述： 获取副本信息
        参数：
        @job_info：JobInfo 任务相关信息
        """
        param = ParamFileUtil.parse_param_file(req_id)
        job_info = TdsqlBackupService.query_back_copy_jobinfo(req_id, param)
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
        log_detail = LogDetail(logInfo="plugin_task_subjob_success_label", logInfoParam=[sub_job_id], logLevel=1)
        report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_job_id, progress=100,
                                                 logDetail=[log_detail],
                                                 taskStatus=SubJobStatusEnum.COMPLETED.value).dict(by_alias=True))
        return True

    @staticmethod
    def do_post_job(req_id, job_id, sub_id, json):
        """
        功能描述：后置任务
        返回值：
        """
        log.info(
            f'start execute do_post_job, req_id: {req_id}, job_id: {job_id},  sub_id: {sub_id}')
        param_inst = ResourceParam(req_id)
        param = param_inst.get_param()
        backup_inst = BackUp(req_id, job_id, sub_id, json, param)
        backup_inst.do_post_job()
        return True

    @staticmethod
    def get_repository_path(file_content, repository_type):
        repositories = file_content.get("job", {}).get("repositories", [])
        repositories_path = ""
        for repository in repositories:
            if repository['repositoryType'] == repository_type:
                repositories_path = repository["path"][0]
                log.info(f"repositories_path {repositories_path}")
                break
        return repositories_path

    @staticmethod
    def set_error_response(response):
        response.code = ExecuteResultEnum.INTERNAL_ERROR.value
        response.body_err = ExecuteResultEnum.INTERNAL_ERROR.value

    @staticmethod
    def backup_prerequisite_progress(req_id, job_id, sub_id, json):
        log.info(
            f'start execute backup_prerequisite_job_progress, req_id: {req_id}, job_id: {job_id},  sub_id: {sub_id}')
