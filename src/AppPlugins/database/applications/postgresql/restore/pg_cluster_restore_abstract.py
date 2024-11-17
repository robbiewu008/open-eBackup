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
import time
from abc import ABC, abstractmethod

from common.const import ExecuteResultEnum, RepositoryDataTypeEnum
from common.logger import Logger
from common.number_const import NumberConst
from postgresql.common.const import RestoreAction, InstallDeployType
from postgresql.common.error_code import ErrorCode
from postgresql.common.models import RestoreProgress
from postgresql.common.util.pg_common_utils import PostgreCommonUtils
from postgresql.restore.pg_restore_base import PostgresRestoreBase
from postgresql.restore.pg_restore_service import PostgreRestoreService

LOGGER = Logger().get_logger("postgresql.log")


class PostgresClusterRestoreAbstract(PostgresRestoreBase, ABC):
    """PostgreSQL集群实例恢复任务执行抽象类
    """
    def __init__(self, pid, job_id, sub_job_id, param_dict):
        super().__init__(pid, job_id, sub_job_id, param_dict)

    @abstractmethod
    def exec_pre_subtask(self):
        """执行恢复前子任务
        """
        LOGGER.info("Start executing restore prepare subtask ...")
        cache_path = PostgreRestoreService.get_cache_mount_path(self.param_dict)

        # 1.恢复前检查目标实例是否已停止
        if not PostgreRestoreService.pre_check_db_not_running(self.pid, self.param_dict, cache_path, is_cluster=True):
            return
        # 检查数据库端口是否被占用
        if not PostgreRestoreService.check_db_port_not_listen(self.pid, self.param_dict, cache_path, is_cluster=True):
            return
        install_deploy_type = self.param_dict.get("job", {}).get("targetEnv", {}).get("extendInfo", {}).get(
            "installDeployType", InstallDeployType.PGPOOL)
        restore_time = self.param_dict.get("job", {}).get("extendInfo", {}).get("restoreTimestamp")
        if install_deploy_type == InstallDeployType.PGPOOL:
            # 检查Pgpool端口是否被占用
            if not PostgreRestoreService.check_pgpool_port_not_listen(self.pid, self.param_dict, cache_path):
                return
        elif install_deploy_type == InstallDeployType.PATRONI:
            # 如果以Patroni部署，进行日志恢复，检查patroni.yml是否配置相关参数
            if restore_time and not PostgreRestoreService.check_patroni_pit_conf(self.pid, self.param_dict, cache_path):
                return

        job_dict = self.param_dict.get("job", {})
        copies = PostgreRestoreService.parse_copies(job_dict)
        copy_mount_path = PostgreRestoreService.get_copy_mount_paths(
            copies[0], RepositoryDataTypeEnum.DATA_REPOSITORY.value)[0]
        tgt_location = job_dict.get("extendInfo", {}).get("targetLocation")
        # 恢复到原位置
        if tgt_location == "original":
            # 检查表空间的目录是否存在，存在则检查表空间所属用户是否与data目录一致，不存在则创建
            if not PostgreRestoreService.pre_check_table_space_path(self.pid, self.job_id, copy_mount_path):
                return
            PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE, RestoreProgress(
                progress=10, message="completed"))
            LOGGER.info(f"Executing restore prepare subtask success, target location is {tgt_location}.")
            PostgreRestoreService.record_task_result(self.pid, "Executing restore prepare subtask success",
                                                     code=ExecuteResultEnum.SUCCESS.value)
            return

        # 3.检查副本的数据库版本和目标实例版本是否匹配
        if not PostgreRestoreService.pre_check_version_is_matched(
                self.pid, self.param_dict, cache_path, is_cluster=True):
            return

        # 4.检查目标实例安装目录所属用户是否是副本数据所属的系统用户
        if not PostgreRestoreService.pre_check_os_user_is_same(
                self.pid, self.param_dict, cache_path, is_cluster=True):
            return

        # 5.检查副本数据所属的系统用户是否可读写目标实例数据目录的父目录
        if not PostgreRestoreService.pre_check_target_data_path_is_rw(
                self.pid, self.param_dict, cache_path, is_cluster=True):
            return

        # 6.检查表空间的目录是否存在，存在则检查表空间所属用户是否与data目录一致，不存在则创建
        if not PostgreRestoreService.pre_check_table_space_path(self.pid, self.job_id, copy_mount_path):
            return

        # 7.清理残余patroni日志恢复拷贝数据目录
        PostgreRestoreService.delete_copy_dir_for_patroni(self.param_dict)
        LOGGER.info(f"Execute restore prepare subtask success, target location is {tgt_location}")
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE, RestoreProgress(
            progress=10, message="completed"))
        PostgreRestoreService.record_task_result(self.pid, "Executing restore prepare subtask success",
                                                 code=ExecuteResultEnum.SUCCESS.value)

    @abstractmethod
    def exec_post_subtask(self):
        """执行恢复后子任务
        """
        LOGGER.info("Start executing restore post subtask ...")
        cache_path = PostgreRestoreService.get_cache_mount_path(self.param_dict)
        install_deploy_type = self.param_dict.get("job", {}).get("targetEnv", {}).get("extendInfo", {}).get(
            "installDeployType", InstallDeployType.PGPOOL)
        if install_deploy_type == InstallDeployType.PGPOOL:
            pgpool_install_path = PostgreRestoreService.get_pgpool_install_path(self.param_dict)
            # 清理pgpool_status文件
            pgpool_log_dir = PostgreRestoreService.get_pgpool_log_dir(pgpool_install_path)
            if pgpool_log_dir and os.path.isdir(pgpool_log_dir):
                pgpool_status_path = os.path.realpath(os.path.join(pgpool_log_dir, "pgpool_status"))
                PostgreCommonUtils.delete_path(pgpool_status_path)
            PostgreCommonUtils.write_progress_info(
                cache_path, RestoreAction.QUERY_RESTORE,
                RestoreProgress(progress=NumberConst.NINETY_TWO, message="clear pgpool_status file success"))

            # 启动Pgpool
            PostgreCommonUtils.start_pgpool(pgpool_install_path)
            PostgreCommonUtils.write_progress_info(
                cache_path, RestoreAction.QUERY_RESTORE,
                RestoreProgress(progress=NumberConst.NINETY_FOUR, message="execute start pgpool command success"))

            # 休眠120s，等待Pgpool调度完成
            time.sleep(120)
            tgt_pgpool_port = PostgreRestoreService.get_pgpool_port(self.param_dict)
            is_listened = PostgreCommonUtils.check_port_is_listen(tgt_pgpool_port)
            if not is_listened:
                PostgreCommonUtils.write_progress_info(
                    cache_path, RestoreAction.QUERY_RESTORE,
                    RestoreProgress(progress=NumberConst.NINETY_NINE,
                                    message="pgpool port not listening failed after startup",
                                    err_code=ErrorCode.PGPOOL_PORT_NOT_LISTEN_ERR_AFTER_RESTORE))
                PostgreRestoreService.record_task_result(self.pid,
                                                         "The target pgpool port not listening after startup.",
                                                         err_code=ErrorCode.PGPOOL_PORT_NOT_LISTEN_ERR_AFTER_RESTORE)
                return
            LOGGER.info(f"Start pgpool success.")
        # 删除patroni日志恢复拷贝数据目录
        if install_deploy_type == InstallDeployType.PATRONI:
            PostgreRestoreService.delete_copy_dir_for_patroni(self.param_dict)
        LOGGER.info(f"Execute restore post subtask success.")

        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE, RestoreProgress(
            progress=NumberConst.HUNDRED, message="completed"))
        PostgreRestoreService.record_task_result(self.pid, "Executing restore prepare subtask success",
                                                 code=ExecuteResultEnum.SUCCESS.value)
