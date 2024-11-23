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
import shutil

from common.const import RepositoryDataTypeEnum, ExecuteResultEnum
from common.logger import Logger
from postgresql.common.const import RestoreAction
from postgresql.common.models import RestoreConfigParam, RestoreProgress
from postgresql.common.util.pg_common_utils import PostgreCommonUtils
from postgresql.common.util.pg_decorators import restore_exception_decorator
from postgresql.restore.pg_restore_base import PostgresRestoreBase
from postgresql.restore.pg_restore_service import PostgreRestoreService

LOGGER = Logger().get_logger("postgresql.log")


class SingleInstPitRestore(PostgresRestoreBase):
    """PostgreSQL按时间点恢复任务执行类
    """
    def __init__(self, pid, job_id, sub_job_id, param_dict):
        super().__init__(pid, job_id, sub_job_id, param_dict)

    @restore_exception_decorator
    def exec_pre_task(self):
        """执行前置任务
        """
        super(SingleInstPitRestore, self).exec_pre_task()

    @restore_exception_decorator
    def exec_restore_subtask(self):
        """执行任务
        """
        LOGGER.info("Start executing point-in-time recovery task ...")
        cache_path = PostgreRestoreService.get_cache_mount_path(self.param_dict)

        # 1.清空目标实例data目录
        tgt_install_path, tgt_data_path = PostgreRestoreService.get_db_install_and_data_path(self.param_dict)
        PostgreRestoreService.backup_conf_file(tgt_data_path, self.job_id)
        PostgreRestoreService.clear_data_dir(PostgreRestoreService.parse_os_user(self.param_dict), tgt_data_path)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=5, message="delete data dir success"))
        job_dict = self.param_dict.get("job", {})
        copies = PostgreRestoreService.parse_copies(job_dict)
        full_copy_mount_path = PostgreRestoreService.get_copy_mount_paths(
            copies[0], RepositoryDataTypeEnum.DATA_REPOSITORY.value)[0]

        # 1.1 清空目标实例的table space目录
        PostgreRestoreService.clear_table_space_dir(copy_mount_path=full_copy_mount_path)

        # 2.恢复全量副本数据到目标实例
        log_copy_merge_path = PostgreRestoreService.handle_log_copy(self.param_dict, job_id=self.job_id)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=20, message="merge log copy success"))

        PostgreRestoreService.change_owner_of_download_data(self.param_dict, full_copy_mount_path)
        tgt_obj_extend_info_dict = job_dict.get("targetObject", {}).get("extendInfo", {})
        tgt_db_os_user = tgt_obj_extend_info_dict.get("osUsername", "")
        PostgreRestoreService.restore_data(cache_path, full_copy_mount_path,
                                           tgt_data_path, job_id=self.job_id)
        PostgreRestoreService.restore_conf_file(tgt_data_path, self.job_id)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=70, message="restore data success"))

        # 3.清空目标实例archive_status目录
        tgt_version = tgt_obj_extend_info_dict.get("version", "")
        PostgreRestoreService.clear_archive_status_dir(tgt_version, tgt_data_path)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=75, message="clear archive_status success"))

        # 4.删除目标实例无用的文件
        PostgreRestoreService.delete_useless_files_of_data_dir(tgt_data_path)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=80, message="delete useless files success"))

        # 5.配置恢复命令
        recovery_timestamp = job_dict.get("extendInfo", {}).get("restoreTimestamp")
        recovery_tgt_time = PostgreCommonUtils.convert_timestamp_to_datetime(recovery_timestamp)
        tgt_archive_path = PostgreCommonUtils.get_archive_path_offline(tgt_data_path)
        cfg_param = RestoreConfigParam(
            system_user=tgt_db_os_user, target_version=tgt_version, target_install_path=tgt_install_path,
            target_data_path=tgt_data_path, log_copy_path=log_copy_merge_path,
            recovery_target_time=recovery_tgt_time, target_archive_path=tgt_archive_path
        )
        PostgreRestoreService.set_recovery_conf_file(self.param_dict, cfg_param)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=85, message="set recovery command success"))

        PostgreRestoreService.start_db_after_restore(self.pid, self.param_dict, cache_path)

    @restore_exception_decorator
    def exec_post_task(self):
        """执行后置任务
        """
        LOGGER.info("Start executing restore post ...")
        cache_path = PostgreRestoreService.get_cache_mount_path(self.param_dict)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE_POST,
                                               RestoreProgress(progress=0, message="begin"))

        # 恢复后清理data目录无用文件
        _, tgt_data_path = PostgreRestoreService.get_db_install_and_data_path(self.param_dict)
        PostgreRestoreService.delete_useless_files_of_data_dir(tgt_data_path, before_restore=False)
        PostgreRestoreService.delete_useless_bak_files(tgt_data_path, self.job_id)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE_POST, RestoreProgress(
            progress=50, message="delete useless file of data dir success"))

        # 删除合并日志目录
        merged_path = os.path.realpath(os.path.join(cache_path, "merged_log_copies"))
        if os.path.exists(merged_path):
            ret, realpath = PostgreCommonUtils.check_path_in_white_list(merged_path)
            if ret:
                shutil.rmtree(realpath)
                LOGGER.info("Delete the temporary log copy merge directory success.")
        
        LOGGER.info("Execute restore post task success.")
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE_POST,
                                               RestoreProgress(progress=100, message="completed"))
        PostgreRestoreService.record_task_result(self.pid, "Execute restore post task success",
                                                 code=ExecuteResultEnum.SUCCESS.value)

    @restore_exception_decorator
    def abort_task(self):
        """中止任务
        """
        super(SingleInstPitRestore, self).abort_task()
