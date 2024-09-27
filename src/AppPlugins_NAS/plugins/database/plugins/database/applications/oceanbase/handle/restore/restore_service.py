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

import json
import os
import time

from common.common import output_result_file, report_job_details, execute_cmd, read_tmp_json_file, \
    output_execution_result_ex, is_clone_file_system
from common.common_models import SubJobDetails, SubJobModel, LogDetail, ActionResult
from common.const import RepositoryDataTypeEnum, SubJobPriorityEnum, SubJobPolicyEnum, ExecuteResultEnum, CMDResult
from common.const import SubJobStatusEnum
from common.parse_parafile import ParamFileUtil, CopyParamParseUtil
from oceanbase.common.const import SubJobType, LogLevel, ArchiveType, OceanBaseReportLabel, RestoreConstant, \
    OceanBaseSubJobName, SYSTEM_TENANT_ID, INCARNATION_1, CLUSTER
from oceanbase.common.oceanbase_client import OceanBaseClient
from oceanbase.common.oceanbase_common import get_agent_id, remove_dir
from oceanbase.logger import log


class OceanBaseRestoreService(object):
    """
    恢复任务相关接口
    """

    @staticmethod
    def allow_restore_in_local_node(req_id, job_id):
        """
        功能描述：是否允许本地运行, 业务目前不需要实现, 主任务执行
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        @sub_id： 子任务ID
        返回值：ExecuteResultEnum
        """
        log.info(f'step 1: execute allow_restore_in_local_node, pid: {req_id}, job_id: {job_id}')
        output = ActionResult(code=ExecuteResultEnum.SUCCESS)
        output_result_file(req_id, output.dict(by_alias=True))
        return ExecuteResultEnum.SUCCESS

    @staticmethod
    def restore_prerequisite(req_id, job_id):
        """
        功能描述：恢复前置任务
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        @sub_id： 子任务ID
        返回值：ExecuteResultEnum
        """
        log.info(f'step 2: execute restore_prerequisite, pid: {req_id}, job_id: {job_id}')
        try:
            body_param = ParamFileUtil.parse_param_file(req_id)
        except Exception as err_code:
            raise Exception(f"Failed to parse job param file for {err_code}") from err_code
        if not body_param:
            raise Exception(f"Failed to parse job param file is none")
        return ExecuteResultEnum.SUCCESS

    @staticmethod
    def restore_prerequisite_progress(req_id, job_id, sub_id):
        """
        功能描述：恢复前置任务进度上报
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        @sub_id：子任务ID
        返回值：
        """
        log.info(
            f'step 3: execute restore_prerequisite_progress, pid: {req_id}, job_id: {job_id}, sub_job_id: {sub_id}')
        progress = SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100,
                                 taskStatus=SubJobStatusEnum.COMPLETED.value)
        output_result_file(req_id, progress.dict(by_alias=True))
        return True

    @staticmethod
    def restore_gen_sub_job(req_id, job_id, sub_id):
        """
        功能描述：生成子任务
        参数：
        @req_id： 请求ID
        @job_id： 主任务任务ID
        @sub_id：子任务ID
        返回值：
        """
        log.info(
            f'step 4: execute restore_gen_sub_job, pid: {req_id}, job_id: {job_id}, sub_job_id: {sub_id}')
        sub_jobs = []
        mount_sub_job = SubJobModel(jobId=job_id, jobType=SubJobType.BUSINESS_SUB_JOB.value,
                                    jobName=OceanBaseSubJobName.SUB_EXEC_MOUNT_JOB,
                                    jobPriority=SubJobPriorityEnum.JOB_PRIORITY_1, ignoreFailed=False,
                                    policy=SubJobPolicyEnum.EVERY_NODE_ONE_TIME.value).dict(by_alias=True)
        sub_jobs.append(mount_sub_job)
        file_content = ParamFileUtil.parse_param_file(req_id)
        exec_node_id = file_content['job']['targetEnv']['extendInfo']['execNodeId']
        sub_job = SubJobModel(jobId=job_id, jobType=SubJobType.BUSINESS_SUB_JOB.value,
                              jobPriority=SubJobPriorityEnum.JOB_PRIORITY_2, jobName=exec_node_id,
                              policy=SubJobPolicyEnum.EVERY_NODE_ONE_TIME.value, ignoreFailed=False).dict(by_alias=True)
        sub_jobs.append(sub_job)
        output_result_file(req_id, sub_jobs)
        return True

    @staticmethod
    def exec_restore(req_id, job_id, sub_id):
        """
        功能描述：恢复任务, 子任务执行, 每个节点都执行
        参数：
        @pid： 请求ID
        @job_id： 主任务任务ID
        @sub_id： 子任务ID
        返回值：
        """
        log.info(f'step 5: execute restore, pid: {req_id}, job_id: {job_id}, sub_job_id: {sub_id}')
        try:
            file_content = ParamFileUtil.parse_param_file(req_id)
            sub_job_name = file_content.get("subJob", {}).get("jobName", "")
            current_agent_id = get_agent_id()
            if sub_job_name == OceanBaseSubJobName.SUB_EXEC_MOUNT_JOB or \
                    file_content['job']['targetEnv']['extendInfo']['execNodeId'] != current_agent_id:
                report_job_details(req_id, SubJobDetails(taskId=job_id, progress=100, subTaskId=sub_id,
                                                         taskStatus=SubJobStatusEnum.COMPLETED.value))
                return
            # 先从meta仓读取子任务进度信息
            # 如果没有进度信息，写入meta仓，子任务任务执行中
            # 更新子任务进度信息为完成，写入结果
            copies = file_content['job']['copies']
            cache_path = CopyParamParseUtil.get_cache_path(copies[0])
            restore_result_path = os.path.join(cache_path, 'restore_result')
            if os.path.isfile(restore_result_path):
                sub_job_exe_result = read_tmp_json_file(restore_result_path)
                task_status = sub_job_exe_result.get('task_status')
                if task_status in [SubJobStatusEnum.COMPLETED.value, SubJobStatusEnum.FAILED.value]:
                    job_detail = SubJobDetails(taskId=job_id, progress=100, subTaskId=sub_id, taskStatus=task_status)
                    report_job_details(req_id, job_detail)
                return
            output_execution_result_ex(restore_result_path, {'task_status': SubJobStatusEnum.RUNNING.value})
            local_ip = OceanBaseRestoreService.get_local_ip(current_agent_id, file_content)
            log_detail = LogDetail(logInfo=OceanBaseReportLabel.EXECUTE_SUB_TASK_SUCCESS_LABEL,
                                   logInfoParam=[local_ip, sub_id], logLevel=LogLevel.INFO.value)
            report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100,
                                                     logDetail=[log_detail], taskStatus=SubJobStatusEnum.RUNNING.value))
            client = OceanBaseClient(req_id, {'application': file_content['job']['targetEnv']})
            check_result = OceanBaseRestoreService.check(client, file_content, sub_id, req_id, job_id)
            if not check_result:
                report_job_details(req_id, SubJobDetails(taskId=job_id, progress=100,
                                                         subTaskId=sub_id, taskStatus=SubJobStatusEnum.FAILED.value))
                return
            tenant_info_list_len = len(json.loads(file_content['job']['extendInfo']['tenantInfos']))
            log_detail = LogDetail(logInfo=OceanBaseReportLabel.RESTORE_TENANT_TOTAL_LABEL,
                                   logInfoParam=[local_ip, str(tenant_info_list_len)], logLevel=LogLevel.INFO.value)
            report_job_details(req_id,
                               SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, logDetail=[log_detail],
                                             taskStatus=SubJobStatusEnum.RUNNING.value))
            task_status = OceanBaseRestoreService.restore_ob(file_content, client, req_id=req_id, job_id=job_id,
                                                             sub_id=sub_id, local_ip=local_ip)
            job_detail = SubJobDetails(taskId=job_id, progress=100, subTaskId=sub_id, taskStatus=task_status)
            report_job_details(req_id, job_detail)
            output_execution_result_ex(restore_result_path, {'task_status': task_status})
        except Exception as err:
            job_detail = SubJobDetails(taskId=job_id, progress=100, subTaskId=sub_id,
                                       taskStatus=SubJobStatusEnum.FAILED.value)
            report_job_details(req_id, job_detail)
            output_execution_result_ex(restore_result_path, {'task_status': SubJobStatusEnum.FAILED.value})
            log.info(err, exec_info=True)

    @staticmethod
    def restore_ob(file_content, client, **kwargs):
        job_id, req_id, sub_id = OceanBaseRestoreService.get_param(kwargs)
        local_ip = kwargs.get('local_ip')
        # 如果有时间戳，检查时间戳合法性:
        # 大于等于最早备份的数据备份的 CDB_OB_BACKUP_SET_DETAILS 的 START_TIME
        # 小于等于日志备份 CDB_OB_BACKUP_ARCHIVELOG_SUMMARY 的MAX_NEXT_TIME
        # 恢复的时间戳是在备份的时候把MAX_NEXT_TIME记下来？还是只要校验当前表的START_TIME和MAX_NEXT_TIME
        tenant_info_list_len = len(json.loads(file_content['job']['extendInfo']['tenantInfos']))
        success_num = 0
        # 查询最大的JOB_ID
        max_restore_job_id = client.query_max_restore_job_id()
        success_list, tem = OceanBaseRestoreService.handle_tenant_info_list(client, file_content, **kwargs)
        task_progress = int(tem / tenant_info_list_len)
        for tenant_info in success_list:
            restore_job = None
            while True:
                restore_jobs = client.query_restore_job(tenant_info['targetName'], tenant_info['originalName'],
                                                        max_restore_job_id)
                if restore_jobs is not None:
                    restore_job = restore_jobs[0]
                if restore_job is None:
                    job_detail = SubJobDetails(taskId=job_id, subTaskId=sub_id, taskStatus=SubJobStatusEnum.RUNNING,
                                               progress=task_progress)
                    report_job_details(req_id, job_detail)
                    time.sleep(10)
                    continue
                tem = tem + 1
                task_progress = int(tem / tenant_info_list_len)
                if restore_job.get('STATUS') == RestoreConstant.RESTORE_SUCCESS_STATUS:
                    success_num = success_num + 1
                    log_detail = LogDetail(logInfo=OceanBaseReportLabel.RESTORE_TENANT_SUCCESS_LABEL,
                                           logInfoParam=[local_ip, str(tem), tenant_info['targetName']],
                                           logLevel=LogLevel.INFO.value)
                    report_job_details(req_id,
                                       SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=task_progress,
                                                     logDetail=[log_detail],
                                                     taskStatus=SubJobStatusEnum.RUNNING.value))
                else:
                    error_info = client.query_error_info(restore_job.get('JOB_ID'))
                    log_detail = LogDetail(logInfo=OceanBaseReportLabel.RESTORE_TENANT_FAIL_LABEL,
                                           logInfoParam=[local_ip, str(tem), tenant_info['targetName'],
                                                         error_info[0].get('INFO')], logLevel=LogLevel.ERROR.value)
                    report_job_details(req_id,
                                       SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=task_progress,
                                                     logDetail=[log_detail],
                                                     taskStatus=SubJobStatusEnum.RUNNING.value))
                break
        if success_num == 0 or success_num < tenant_info_list_len:
            return SubJobStatusEnum.FAILED.value
        else:
            return SubJobStatusEnum.COMPLETED.value

    @staticmethod
    def get_local_ip(current_agent_id, file_content):
        local_ip = None
        nodes = file_content['job']['targetEnv']['nodes']
        for node in nodes:
            if current_agent_id == node['id']:
                local_ip = node['endpoint']
                break
        return local_ip

    @staticmethod
    def handle_tenant_info_list(client, file_content, **kwargs):
        job_id, req_id, sub_id = OceanBaseRestoreService.get_param(kwargs)
        copies = file_content['job']['copies']
        copy_id = OceanBaseRestoreService.get_copy_id(copies[len(copies) - 1])
        cluster = json.loads(copies[len(copies) - 1]['protectEnv']['extendInfo']['clusterInfo'])
        tenant_info_list = json.loads(file_content['job']['extendInfo']['tenantInfos'])
        tenant_info_list_len = len(tenant_info_list)
        tem = 0
        success_tenant_info_list = []
        for tenant_info in tenant_info_list:
            # 检查资源池，1、是否存在；2、是否被占用
            if not OceanBaseRestoreService.check_resource_pool(client, tenant_info):
                tem = tem + 1
                task_progress = int(tem / tenant_info_list_len)
                log_detail = LogDetail(logInfo=OceanBaseReportLabel.CHECK_RESOURCE_POOL_FAIL_LABEL,
                                       logInfoParam=[kwargs.get('local_ip'), str(tem), tenant_info['originalName'],
                                                     tenant_info['resourcePoolName']], logLevel=LogLevel.ERROR.value)
                report_job_details(req_id, SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=task_progress,
                                                         logDetail=[log_detail],
                                                         taskStatus=SubJobStatusEnum.RUNNING.value))
                continue
            # 判断dest_tenant_name是否存在
            if len(client.query_tenant_back(tenant_info['targetName'])):
                tem = tem + 1
                task_progress = int(tem / tenant_info_list_len)
                log_detail = LogDetail(logInfo=OceanBaseReportLabel.CHECK_TENANT_EXIST_FAIL_LABEL,
                                       logInfoParam=[kwargs.get('local_ip'), str(tem), tenant_info['originalName'],
                                                     tenant_info['targetName']], logLevel=LogLevel.ERROR.value)
                report_job_details(req_id,
                                   SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=task_progress,
                                                 logDetail=[log_detail], taskStatus=SubJobStatusEnum.RUNNING.value))
                continue
            tenant_id = OceanBaseRestoreService.get_tenant_id(copies, tenant_info)
            uri = OceanBaseRestoreService.generate_uri(cluster, copies, copy_id, tenant_id, file_content)
            pool_list = tenant_info['resourcePoolName']
            restore_tables_str, timestamp = OceanBaseRestoreService.get_tables_and_timestamp(copies, file_content,
                                                                                             tenant_info, copy_id)
            _, out, _ = client.execute_restore(tenant_info['targetName'], tenant_info['originalName'], uri,
                                               restore_tables=restore_tables_str, pool_list=pool_list,
                                               backup_cluster_name=cluster.get("cluster_name"), timestamp=timestamp,
                                               backup_cluster_id=cluster.get("cluster_id"))
            if "ERROR" in out:
                tem = tem + 1
                task_progress = int(tem / tenant_info_list_len)
                log_detail = LogDetail(logInfoParam=[kwargs.get('local_ip'), str(tem), tenant_info['originalName'],
                                                     out], logInfo=OceanBaseReportLabel.RESTORE_TENANT_FAIL_LABEL,
                                       logLevel=LogLevel.ERROR.value)
                report_job_details(req_id,
                                   SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=task_progress,
                                                 logDetail=[log_detail], taskStatus=SubJobStatusEnum.RUNNING.value))
            else:
                success_tenant_info_list.append(tenant_info)
        return success_tenant_info_list, tem

    @staticmethod
    def get_tenant_id(copies, tenant_info):
        copy = copies[len(copies) - 1]
        copy_type = copy.get("type", "")
        if copy_type in ArchiveType.archive_array:
            tenant_list = copy['extendInfo']['extendInfo']['tenant_list']
        else:
            tenant_list = copy['extendInfo']['tenant_list']
        tenant_id = None
        for tenant in tenant_list:
            if tenant['name'] == tenant_info['originalName']:
                tenant_id = tenant['id']
                break
        return tenant_id

    @staticmethod
    def get_param(kwargs):
        job_id = kwargs.get('job_id')
        req_id = kwargs.get('req_id')
        sub_id = kwargs.get('sub_id')
        return job_id, req_id, sub_id

    @staticmethod
    def check_resource_pool(client, tenant_info):
        resource_pools = client.query_resource_pool_back(tenant_info['resourcePoolId'])
        if len(resource_pools) == 0:
            return False
        for resource_pool in resource_pools:
            if resource_pool['tenant_id'] != 'NULL' and resource_pool['tenant_id'] != '-1':
                return False
        return True

    @staticmethod
    def get_tables_and_timestamp(copies, file_content, tenant_info, copy_id):
        restore_tables_str = None
        if file_content['job']['jobParam']['restoreType'] == 3:
            restore_sub_objects = file_content['job']['restoreSubObjects']
            restore_tables = []
            for table in restore_sub_objects:
                restore_tables.append(table['name'])
            restore_tables_str = ','.join(restore_tables)
        sub_type = copies[len(copies) - 1]['protectObject']['subType']
        timestamp = tenant_info['timestamp'] if tenant_info.__contains__('timestamp') else None
        if sub_type == RestoreConstant.CLUSTER_SUB_TYPE and timestamp is None:
            meta_uri = OceanBaseRestoreService.get_meta_repository_path_restore(copies[len(copies) - 1])
            copy_info_path = os.path.join(meta_uri, copy_id, 'copy_info')
            copy_info = read_tmp_json_file(copy_info_path)
            timestamp = copy_info.get('backup_time')
        return restore_tables_str, timestamp

    @staticmethod
    def generate_uri(cluster, copies, copy_id, tenant_id, file_content):
        restore_type = copies[len(copies) - 1]['type']
        uri = OceanBaseRestoreService.get_repository_path_restore(copies[len(copies) - 1],
                                                                  RepositoryDataTypeEnum.DATA_REPOSITORY)
        if restore_type == 'log':
            # 从后往前找到第一个不是日志的副本，然后记下id
            dest_num = 0
            restore_copy_id = None
            for copy in copies[::-1]:
                restore_type = copy['type']
                if restore_type != 'log':
                    restore_copy_id = copy['id']
                    uri = OceanBaseRestoreService.get_repository_path_restore(copy,
                                                                              RepositoryDataTypeEnum.DATA_REPOSITORY)
                    break
                dest_num = dest_num + 1
            cluster_name = cluster.get("cluster_name")
            cluster_id = cluster.get("cluster_id")
            log_uri = OceanBaseRestoreService.get_repository_path_restore(copies[len(copies) - 1],
                                                                          RepositoryDataTypeEnum.LOG_REPOSITORY)
            log_uri_strs = log_uri.split('/')
            log_uri = log_uri.replace(f'/{log_uri_strs[len(log_uri_strs) - 1]}', '')
            id_list = OceanBaseRestoreService.get_id_list(log_uri, restore_copy_id)
            if not is_clone_file_system(file_content):
                # 非克隆文件系统，在cache仓组装恢复副本
                uri = OceanBaseRestoreService.symlink_data_copy(cluster_id, cluster_name, copies, restore_copy_id, uri)
            for current_copy_id in id_list:
                cp_cmd = f'/bin/cp -rf {log_uri}/{current_copy_id}/{current_copy_id}/{tenant_id} {uri}/{CLUSTER}' \
                         f'/{cluster_name}/{cluster_id}/{INCARNATION_1}'
                execute_cmd(cp_cmd)
                chmod_cmd = f'chmod -R 755 {uri}/{CLUSTER}/{cluster_name}/{cluster_id}/{INCARNATION_1}/{tenant_id}/clog'
                execute_cmd(chmod_cmd)
        sub_type = copies[len(copies) - 1]['protectObject']['subType']
        if sub_type == RestoreConstant.CLUSTER_SUB_TYPE:
            uri = uri + os.sep + CLUSTER
        else:
            uri = uri + os.sep + copy_id
        return uri

    @staticmethod
    def symlink_data_copy(cluster_id, cluster_name, copies, restore_copy_id, uri):
        # 获取cache仓，创建目录
        cache_path = CopyParamParseUtil.get_cache_path(copies[0])
        temp_uri = os.path.join(cache_path, restore_copy_id, CLUSTER, cluster_name, cluster_id,
                                INCARNATION_1)
        if not os.path.exists(temp_uri):
            os.makedirs(temp_uri)
            data_copy_path = os.path.join(cache_path, restore_copy_id)
            return_code, _, std_err = execute_cmd(f'chmod -R 755 {data_copy_path}')
            if return_code != CMDResult.SUCCESS:
                log.error(f"Fail to chmod, err: {std_err}")
            parts_child_names = os.listdir(f'{uri}/{CLUSTER}/{cluster_name}/{cluster_id}/{INCARNATION_1}/')
            for store_child_name in parts_child_names:
                OceanBaseRestoreService.symlink_data(cluster_id, cluster_name, store_child_name, temp_uri, uri)
            uri = os.path.join(cache_path, restore_copy_id)
        return uri

    @staticmethod
    def symlink_data(cluster_id, cluster_name, store_child_name, temp_uri, uri):
        if store_child_name.isdigit() and store_child_name != SYSTEM_TENANT_ID:
            # 非系统租户需要执行以下逻辑
            src = f'{uri}/{CLUSTER}/{cluster_name}/{cluster_id}/{INCARNATION_1}/{store_child_name}/data'
            dst = f'{temp_uri}/{store_child_name}/data'
            if not os.path.exists(f'{temp_uri}/{store_child_name}/'):
                os.makedirs(f'{temp_uri}/{store_child_name}/')
                execute_cmd(f'chmod 755 {temp_uri}/{store_child_name}/')
            # 创建软链接
            os.symlink(src, dst)
            cp_cmd = (f'/bin/cp -rf {uri}/{CLUSTER}/{cluster_name}/{cluster_id}/{INCARNATION_1}/'
                      f'{store_child_name}/clog {temp_uri}/{store_child_name}/')
            return_code, _, std_err = execute_cmd(cp_cmd)
            if return_code != CMDResult.SUCCESS:
                log.error(f"Fail to cp, err: {std_err}")
        else:
            dst = f'{temp_uri}/{store_child_name}'
            os.symlink(f'{uri}/{CLUSTER}/{cluster_name}/{cluster_id}/{INCARNATION_1}/{store_child_name}', dst)

    @staticmethod
    def get_id_list(log_path_parent_dir, restore_copy_id):
        """
        从副本id.meta文件，组装timestamp和id的字典，如下
        {'788dee39-7123-453e-a697-af6a94896bfb': '1664939072~1665040898'}
        """
        restore_copy_id_file = restore_copy_id + ".meta"
        dot_meta_path = os.path.join(log_path_parent_dir, restore_copy_id_file)
        id_list = []
        if not os.path.exists(dot_meta_path) or not dot_meta_path:
            return id_list
        with open(dot_meta_path, 'r', encoding='utf-8') as file_read:
            for line in file_read.readlines():
                key_value = line.strip('\n').split(";")
                key = key_value[0].strip()
                id_list.append(key)
        return id_list

    @staticmethod
    def get_repository_path_restore(copy, repository_type):
        repositories = copy.get("repositories", [])
        repositories_path = ""
        for repository in repositories:
            if repository['repositoryType'] == repository_type and repository.__contains__('path'):
                repositories_path = repository["path"][0]
                break
        return repositories_path

    @staticmethod
    def get_meta_repository_path_restore(copy):
        repositories = copy.get("repositories", [])
        repositories_path = ""
        for repository in repositories:
            if repository['repositoryType'] == 0 and 'Database_MetaDataRepository' not in repository['remotePath']:
                repositories_path = repository["path"][0]
                break
        return repositories_path

    @staticmethod
    def restore_post_job(req_id, job_id):
        """
        功能描述：恢复后置子任务
        参数：
        @pid： 请求ID
        @job_id： 主任务任务ID
        @sub_job_id： 子任务ID
        返回值：
        """
        log.info(f'step 6: execute restore_post_job, pid: {req_id}, job_id: {job_id}')
        output = ActionResult(code=ExecuteResultEnum.SUCCESS)
        try:
            body_param = ParamFileUtil.parse_param_file(req_id)
            if not is_clone_file_system(body_param):
                OceanBaseRestoreService.remove_cache_data(body_param)
            copies = body_param['job']['copies']
            cache_path = CopyParamParseUtil.get_cache_path(copies[0])
            restore_result_path = os.path.join(cache_path, 'restore_result')
            if os.path.isfile(restore_result_path):
                os.remove(restore_result_path)
        except Exception as err_code:
            raise Exception(f"Failed to parse job param file for {err_code}") from err_code
        if not body_param:
            raise Exception(f"Failed to parse job param file is none")
        output_result_file(req_id, output.dict(by_alias=True))
        return ExecuteResultEnum.SUCCESS

    @staticmethod
    def remove_cache_data(body_param):
        copies = body_param['job']['copies']
        restore_type = copies[len(copies) - 1]['type']
        if restore_type == 'log':
            restore_copy_id = None
            for copy in copies[::-1]:
                restore_type = copy['type']
                if restore_type != 'log':
                    restore_copy_id = copy['id']
                    break
            cache_path = CopyParamParseUtil.get_cache_path(copies[0])
            if os.path.exists(os.path.join(cache_path, restore_copy_id)):
                remove_dir(os.path.join(cache_path, restore_copy_id))

    @staticmethod
    def restore_post_job_progress(req_id, job_id, sub_id):
        """
        功能描述：恢复后置子任务进度
        参数：
        @pid： 请求ID
        @job_id： 主任务任务ID
        @sub_job_id： 子任务ID
        返回值：
        """
        log.info(f'step 7: execute restore_post_job_progress, pid: {req_id}, job_id: {job_id}')
        progress_info = SubJobDetails(taskId=job_id, subTaskId=sub_id, taskStatus=SubJobStatusEnum.COMPLETED.value,
                                      logDetail=[], progress=100, dataSize=0, speed=0, extendInfo=None)
        output_result_file(req_id, progress_info.dict(by_alias=True))
        return ExecuteResultEnum.SUCCESS

    @staticmethod
    def check(client, file_content, sub_id, req_id, job_id):
        cluster_infos = client.query_cluster_info(client.observer_ip, client.observer_port)
        # 检查恢复目标集群状态是否为有效状态，以下集群不能作为恢复目标：
        # 1、备模式的集群
        # 2、状态为不可用的集群
        cluster = None
        cluster_info_str = file_content['job']['targetEnv']['extendInfo']['clusterInfo']
        cluster_name = file_content['job']['targetEnv']['name']
        cluster_info_json = json.loads(cluster_info_str)
        for cluster_info in cluster_infos:
            cluster_id = cluster_info.get("cluster_id")
            if cluster_id == cluster_info_json.get("cluster_id"):
                cluster = cluster_info
                break
        if cluster is None:
            log_detail = LogDetail(logInfo=OceanBaseReportLabel.CHECK_CLUSTER_EXIST_FAIL_LABEL,
                                   logInfoParam=[cluster_name], logLevel=LogLevel.ERROR.value)
            report_job_details(req_id,
                               SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, logDetail=[log_detail],
                                             taskStatus=SubJobStatusEnum.RUNNING.value))
            return False
        if cluster.get("cluster_role") != RestoreConstant.PRIMARY_CLUSTER_ROLE:
            log_detail = LogDetail(logInfo=OceanBaseReportLabel.CHECK_CLUSTER_ROLE_FAIL_LABEL,
                                   logInfoParam=[cluster_name], logLevel=LogLevel.ERROR.value)
            report_job_details(req_id,
                               SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, logDetail=[log_detail],
                                             taskStatus=SubJobStatusEnum.RUNNING.value))
            return False
        if cluster.get("cluster_status") != RestoreConstant.VALID_CLUSTER_STATUS:
            log_detail = LogDetail(logInfo=OceanBaseReportLabel.CHECK_CLUSTER_STATUS_FAIL_LABEL,
                                   logInfoParam=[cluster_name], logLevel=LogLevel.ERROR.value)
            report_job_details(req_id,
                               SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, logDetail=[log_detail],
                                             taskStatus=SubJobStatusEnum.RUNNING.value))
            return False

        # 检查目标集群恢复配置是否打开,如果为 0
        restore_concurrency = client.query_restore_concurrency()
        if restore_concurrency == '0':
            log_detail = LogDetail(logInfo=OceanBaseReportLabel.CHECK_RESTORE_CONCURRENCY_FAIL_LABEL,
                                   logInfoParam=[cluster_name], logLevel=LogLevel.ERROR.value)
            report_job_details(req_id,
                               SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, logDetail=[log_detail],
                                             taskStatus=SubJobStatusEnum.RUNNING.value))
            return False
        return True

    @staticmethod
    def get_copy_id(copy):
        """
        获取恢复副本本身的ID
        """
        copy_type = copy.get("type", "")
        if copy_type in ArchiveType.archive_array:
            # 如果是归档到磁带副本，需要从extendInfo里拿原始备份副本
            return copy.get("extendInfo", {}).get("extendInfo", {}).get("copy_id", "")
        return copy.get("extendInfo", {}).get("copy_id", "")
