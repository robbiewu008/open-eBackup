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
from abc import abstractmethod, ABCMeta

from common.const import ExecuteResultEnum, RepositoryDataTypeEnum
from common.logger import Logger
from postgresql.common.const import RestoreAction
from postgresql.common.models import RestoreProgress
from postgresql.common.util.pg_common_utils import PostgreCommonUtils
from postgresql.restore.pg_restore_service import PostgreRestoreService

LOGGER = Logger().get_logger("postgresql.log")


class PostgresRestoreBase(metaclass=ABCMeta):
    """PostgreSQL恢复任务执行基类
    """
    def __init__(self, pid, job_id, sub_job_id, param_dict):
        self.pid = pid
        self.job_id = job_id
        self.sub_job_id = sub_job_id
        self.param_dict = param_dict
        self.progress = 0

    @abstractmethod
    def exec_pre_task(self):
        """执行前置任务
        """
        LOGGER.info("Start executing restore prerequisite ...")
        cache_path = PostgreRestoreService.get_cache_mount_path(self.param_dict)

        # 1.恢复前检查目标实例是否已停止
        if not PostgreRestoreService.pre_check_db_not_running(self.pid, self.param_dict, cache_path):
            return
        # 2.检查数据库端口是否被占用
        if not PostgreRestoreService.check_db_port_not_listen(self.pid, self.param_dict, cache_path):
            return

        job_dict = self.param_dict.get("job", {})
        copies = PostgreRestoreService.parse_copies(job_dict)
        copy_mount_path = PostgreRestoreService.get_copy_mount_paths(
            copies[0], RepositoryDataTypeEnum.DATA_REPOSITORY.value)[0]
        tgt_location = job_dict.get("extendInfo", {}).get("targetLocation")
        # 恢复到原位置
        if tgt_location == "original":
            # 检查表空间的目录是否存在，存在则检查表空间所属用户是否与data目录一致
            if not PostgreRestoreService.pre_check_table_space_path(self.pid, self.job_id, copy_mount_path):
                return
            PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE_PRE,
                                                   RestoreProgress(progress=10, message="completed"))
            LOGGER.info(f"Executing restore prerequisite task success, target location is {tgt_location}.")
            PostgreRestoreService.record_task_result(self.pid, "Executing restore prerequisite task success",
                                                     code=ExecuteResultEnum.SUCCESS.value)
            return

        # 3.检查副本的数据库版本和目标实例版本是否匹配
        if not PostgreRestoreService.pre_check_version_is_matched(self.pid, self.param_dict, cache_path):
            return

        # 4.检查目标实例安装目录所属用户是否是副本数据所属的系统用户
        if not PostgreRestoreService.pre_check_os_user_is_same(
                self.pid, self.param_dict, cache_path):
            return

        # 5.检查副本数据所属的系统用户是否可读写目标实例数据目录的父目录
        if not PostgreRestoreService.pre_check_target_data_path_is_rw(
                self.pid, self.param_dict, cache_path):
            return

        # 6.检查表空间的目录是否存在，存在则检查表空间所属用户是否与data目录一致
        if not PostgreRestoreService.pre_check_table_space_path(self.pid, self.job_id, copy_mount_path):
            return

        LOGGER.info(f"Execute restore prerequisite task success, target location is {tgt_location}")
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE_PRE,
                                               RestoreProgress(progress=100, message="completed"))
        PostgreRestoreService.record_task_result(self.pid, "Executing restore prerequisite task success",
                                                 code=ExecuteResultEnum.SUCCESS.value)

    @abstractmethod
    def exec_restore_subtask(self):
        """执行恢复子任务
        """
        pass

    @abstractmethod
    def exec_post_task(self):
        """执行后置任务
        """
        pass

    @abstractmethod
    def abort_task(self):
        """中止任务
        """
        LOGGER.info("Start aborting restore task ...")

        tgt_data_path = ""
        tgt_data_upper_path = os.path.realpath(os.path.join(tgt_data_path, ".."))
        restore_cmd_reg = r"^cp\s+((/[^/]{1,255})+|/)\s+" + tgt_data_upper_path + "$"
        is_process_exist, p_id = PostgreCommonUtils.check_exist_process_by_reg(restore_cmd_reg)
        if is_process_exist:
            LOGGER.info(f"There is a recovery process: {p_id}, try killing it")
            PostgreCommonUtils.kill_process(p_id)
        else:
            LOGGER.warning("No recovery process exists when aborting restore task")

        LOGGER.info("Abort restore task success.")
        PostgreRestoreService.record_task_result(self.pid, "Executing abort restore task success",
                                                 code=ExecuteResultEnum.SUCCESS.value)
