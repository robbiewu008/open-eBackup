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

import time
from common.const import RepositoryDataTypeEnum, ExecuteResultEnum
from common.logger import Logger
from postgresql.common.const import RestoreAction
from postgresql.common.error_code import ErrorCode
from postgresql.common.models import RestoreConfigParam, RestoreProgress
from postgresql.common.util.pg_common_utils import PostgreCommonUtils
from postgresql.common.util.pg_decorators import restore_exception_decorator
from postgresql.restore.pg_restore_base import PostgresRestoreBase
from postgresql.restore.pg_restore_service import PostgreRestoreService

LOGGER = Logger().get_logger("postgresql.log")


class SingleInstFullRestore(PostgresRestoreBase):
    """PostgreSQL全量副本恢复任务执行类
    """
    def __init__(self, pid, job_id, sub_job_id, param_dict):
        super().__init__(pid, job_id, sub_job_id, param_dict)

    @restore_exception_decorator
    def exec_pre_task(self):
        """执行前置任务
        """
        super(SingleInstFullRestore, self).exec_pre_task()

    def exec_pre_subtask(self):
        pass

    @restore_exception_decorator
    def exec_restore_subtask(self):
        """执行任务
        """
        LOGGER.info("Start executing restore task of full copy ...")
        cache_path = PostgreRestoreService.get_cache_mount_path(self.param_dict)

        # 1.清空目标实例data目录
        tgt_install_path, tgt_data_path = PostgreRestoreService.get_db_install_and_data_path(self.param_dict)
        PostgreRestoreService.backup_conf_file(tgt_data_path)
        PostgreRestoreService.clear_data_dir(PostgreRestoreService.parse_os_user(self.param_dict), tgt_data_path)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=5, message="delete data dir ok"))
        # 1.1 清空目标实例的table space目录
        job_dict = self.param_dict.get("job", {})
        copy_mount_path = PostgreRestoreService.get_copy_mount_paths(
            PostgreRestoreService.parse_copies(job_dict)[0], RepositoryDataTypeEnum.DATA_REPOSITORY.value)[0]
        PostgreRestoreService.clear_table_space_dir(copy_mount_path=copy_mount_path)

        # 2.恢复全量副本数据到目标实例
        tgt_obj_extend_info_dict = job_dict.get("targetObject", {}).get("extendInfo", {})
        tgt_db_os_user = tgt_obj_extend_info_dict.get("osUsername", "")
        PostgreRestoreService.change_owner_of_download_data(self.param_dict, copy_mount_path)
        PostgreRestoreService.restore_data(cache_path, copy_mount_path,
                                           tgt_data_path, job_id=self.job_id)
        PostgreRestoreService.restore_conf_file(tgt_data_path)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=72, message="restore data ok"))

        # 3.清空目标实例archive_status目录
        PostgreRestoreService.clear_archive_status_dir(tgt_obj_extend_info_dict.get("version", ""), tgt_data_path)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=74, message="clear archive_status dir ok"))

        # 4.删除目标实例无用的文件
        PostgreRestoreService.delete_useless_files_of_data_dir(tgt_data_path)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=76, message="delete useless files of data ok"))

        # 5.配置恢复命令
        tgt_archive_path = PostgreCommonUtils.get_archive_path_offline(tgt_data_path)
        cfg_param = RestoreConfigParam(
            system_user=tgt_db_os_user, target_version=tgt_obj_extend_info_dict.get("version", ""),
            target_install_path=tgt_install_path, target_data_path=tgt_data_path, target_archive_path=tgt_archive_path)
        PostgreRestoreService.set_recovery_conf_file(self.param_dict, cfg_param)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=80, message="set recovery command ok"))

        # 6.启动数据库实例
        PostgreCommonUtils.start_postgresql_database(tgt_db_os_user, tgt_install_path, tgt_data_path)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=90, message="start database ok"))

        # 7.检查数据库实例是否启动成功
        is_running = PostgreCommonUtils.is_db_running(tgt_db_os_user, tgt_install_path, tgt_data_path)
        if not is_running:
            PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                                   RestoreProgress(progress=95, message="database run failed"))
            PostgreRestoreService.record_task_result(self.pid, "The target instance is not running after restore.",
                                                     err_code=ErrorCode.DB_STATUS_ERR_AFTER_RESTORE)
            return
        PostgreRestoreService.cleanup_archive_after_restore(self.param_dict)
        PostgreRestoreService.reset_recovery_config(job_dict, tgt_obj_extend_info_dict.get("version", ""),
                                                    tgt_data_path, "")
        LOGGER.info("Execute restore task of full copy success.")
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=100, message="completed"))
        PostgreRestoreService.record_task_result(self.pid, "Executing restore task success",
                                                 code=ExecuteResultEnum.SUCCESS.value)

    def exec_post_subtask(self):
        pass

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

        LOGGER.info("Execute restore post task success.")
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE_POST,
                                               RestoreProgress(progress=100, message="completed"))
        PostgreRestoreService.record_task_result(self.pid, "Execute restore post task success",
                                                 code=ExecuteResultEnum.SUCCESS.value)

    @restore_exception_decorator
    def abort_task(self):
        """中止任务
        """
        super(SingleInstFullRestore, self).abort_task()
