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
import json

from common.const import (BackupTypeEnum, SubJobStatusEnum, ExecuteResultEnum,
                          RepositoryDataTypeEnum)
from common.common import (exter_attack, check_del_dir,
                           output_execution_result_ex)
from common.logger import Logger
from common.common_models import ActionResult, SubJobDetails
from common.parse_parafile import ParamFileUtil

from dameng.commons.const import ArrayIndex, DMJsonConstant
from dameng.commons.common import del_file, check_path_in_white_list

LOGGER = Logger().get_logger("dameng.log")


class DeleteCopies():
    def __init__(self, pid, job_id, sub_job_id=None):
        self.pid = pid
        self.job_id = job_id
        self.sub_job_id = sub_job_id
        self.param_dict = ParamFileUtil.parse_param_file(pid)

    def get_copies_data_path(self):
        """
        解析参数文件，获取当前任务的副本路径
        :return: 副本路径列表
        """
        copies_path = []
        sub_type = self.param_dict.get(DMJsonConstant.JOB, {}).get(DMJsonConstant.APPENV, {})\
            .get(DMJsonConstant.SUBTYPE, '')
        if not sub_type:
            LOGGER.error(f"Job id: {self.job_id} copy param not exist protectEnv-subType field.")
            return copies_path
        copy_info = self.param_dict.get(DMJsonConstant.JOB, {}).get(DMJsonConstant.COPIES, [{}])
        for copy_item in copy_info:
            backup_type = copy_item.get(DMJsonConstant.EXTENDINFO, {}).get(DMJsonConstant.BACKUPTYPE, 0)
            repositories_info = ParamFileUtil.parse_backup_path(copy_item.get(DMJsonConstant.REPORITTORIES, []))
            data_path = repositories_info.get("data_repository", [''])[ArrayIndex.INDEX_FIRST_0]
            meta_path = repositories_info.get("meta_repository", [''])[ArrayIndex.INDEX_FIRST_0]
            if backup_type == BackupTypeEnum.LOG_BACKUP:
                data_path = repositories_info.get("log_repository", [''])[ArrayIndex.INDEX_FIRST_0]
            if not data_path:
                LOGGER.error(f"Job id: {self.job_id} Failed to get the copy path.")
                return copies_path
            if meta_path:
                sqlite_path = os.path.join(meta_path, 'sqlite')
                copies_path.append(sqlite_path)
            data_path_info = copy_item.get(DMJsonConstant.EXTENDINFO, {}).get(DMJsonConstant.DATAPATH, {})
            if not data_path_info:
                LOGGER.error(f"Job id: {self.job_id} copy param not exist {DMJsonConstant.BACKUPSETNAME} field.")
                return copies_path
            if isinstance(data_path_info, str):
                if backup_type != BackupTypeEnum.FULL_BACKUP:
                    del_path = os.path.join(data_path, data_path_info)
                    copies_path.append(del_path)
                    return copies_path
                data_path_file_list = os.listdir(data_path)
                if not data_path_file_list:
                    LOGGER.error(f"Job id: {self.job_id} No copy file exists in this path.")
                    return copies_path
                for data_path_file in data_path_file_list:
                    del_path = os.path.join(data_path, data_path_file)
                    copies_path.append(del_path)
            else:
                for path in data_path_info:
                    path = path[ArrayIndex.INDEX_FIRST_1:]
                    del_path = os.path.join(data_path, path)
                    copies_path.append(del_path)
        LOGGER.info(f'Job id: {self.job_id} get copy dir succ.')
        return copies_path

    def get_cache_path(self):
        cache_area = ''
        job_dict = self.param_dict.get("job", {}).get("copies", {})
        for copy in job_dict:
            repositories_info = ParamFileUtil.parse_backup_path(copy.get("repositories", []))
            cache_area = repositories_info.get("cache_repository", [])[ArrayIndex.INDEX_FIRST_0]
            if cache_area:
                break
        if not cache_area:
            LOGGER.error(f"Job id: {self.job_id} Failed to get the cache path.")
            return ''
        cache_area = os.path.join(cache_area, f"{self.job_id}_progress.json")
        ret, cache_path = check_path_in_white_list(cache_area)
        if not ret:
            LOGGER.error(f"Job id: {self.job_id} The path is not in the white list.")
            return ''
        return cache_path

    @exter_attack
    def delete_copies_info(self):
        """
        删除副本信息
        :return: 删除结果, 接口返回格式结果
        """
        LOGGER.info(f'Job id: {self.job_id} start.')
        action_result = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="The delete copy failed")
        cache_path = self.get_cache_path()
        if not cache_path:
            output_execution_result_ex(cache_path, {'status': SubJobStatusEnum.FAILED.value})
            return action_result
        copies_path = self.get_copies_data_path()
        if not copies_path:
            LOGGER.error(f'Job id: {self.job_id} can not get copy path.')
            output_execution_result_ex(cache_path, {'status': SubJobStatusEnum.FAILED.value})
            return action_result
        for data_path in copies_path:
            ret, realpath = check_path_in_white_list(data_path)
            if not ret:
                LOGGER.error(f"Job id: {self.job_id}.path non-compliant.")
                output_execution_result_ex(cache_path, {'status': SubJobStatusEnum.FAILED.value})
                return action_result
            if os.path.isfile(realpath):
                try:
                    os.remove(realpath)
                except Exception as exception_str:
                    LOGGER.exception(f"Job id: {self.job_id} exec delete copy dir failed.")
                    output_execution_result_ex(cache_path, {'status': SubJobStatusEnum.FAILED.value})
                    return action_result
            else:
                try:
                    check_del_dir(realpath)
                except Exception as exception_str:
                    LOGGER.exception(f"Job id: {self.job_id} exec delete copy dir failed.")
                    output_execution_result_ex(cache_path, {'status': SubJobStatusEnum.FAILED.value})
                    return action_result
            LOGGER.info(f"Job id: {self.job_id} exec delete copy dir success.")
        LOGGER.info(f'Job id: {self.job_id} success end.')
        action_result = ActionResult(code=ExecuteResultEnum.SUCCESS)
        output_execution_result_ex(cache_path, {'status': SubJobStatusEnum.COMPLETED.value})
        return action_result

    @exter_attack
    def delete_copies_progress(self):
        """
        删除副本进度查询
        :return: 查询结果, 接口返回格式结果
        """
        LOGGER.info(f'Job id: {self.job_id} start.')
        action_result = SubJobDetails(taskId=self.job_id, subTaskId=self.sub_job_id,
                                      taskStatus=SubJobStatusEnum.RUNNING.value, progress=int(0))
        cache_path = self.get_cache_path()
        if not cache_path:
            action_result = SubJobDetails(taskId=self.job_id, subTaskId=self.sub_job_id,
                                          taskStatus=SubJobStatusEnum.FAILED.value, progress=int(0))
            return action_result
        if not os.path.exists(cache_path):
            return action_result
        try:
            with open(cache_path, 'r') as jsonfile:
                json_dict = json.loads(jsonfile.read())
        except Exception:
            LOGGER.exception(f"Job id: {self.job_id} Failed to read the status file.")
            action_result = SubJobDetails(taskId=self.job_id, subTaskId=self.sub_job_id,
                                          taskStatus=SubJobStatusEnum.FAILED.value, progress=int(0))
            return action_result
        task_status = json_dict.get('status', -1)
        if task_status == -1:
            action_result = SubJobDetails(taskId=self.job_id, subTaskId=self.sub_job_id,
                                          taskStatus=SubJobStatusEnum.FAILED.value, progress=int(0))
            return action_result
        if task_status != SubJobStatusEnum.RUNNING.value:
            del_file(cache_path)
        progress = 50
        if task_status == SubJobStatusEnum.COMPLETED.value:
            progress = 100
            LOGGER.info(f'Job id: {self.job_id} success end.')
        action_result = SubJobDetails(taskId=self.job_id, subTaskId=self.sub_job_id,
                                      taskStatus=task_status, progress=progress)
        return action_result
