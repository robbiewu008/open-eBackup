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

from common.const import RoleType, ExecuteResultEnum, RepositoryDataTypeEnum
from common.logger import Logger
from common.number_const import NumberConst
from kingbase.common.const import KbConst
from kingbase.common.kb_exception import ErrCodeException
from kingbase.common.models import RestoreProgress
from kingbase.common.util.kb_decorators import restore_exception_decorator
from kingbase.common.util.resource_util import get_sys_rman_configuration_item
from kingbase.restore.kb_restore_base import RestoreBase
from kingbase.restore.kb_restore_process import KingbaseRestoreProcess
from kingbase.restore.kb_restore_service import KingbaseRestoreService

LOGGER = Logger().get_logger("kingbase.log")


class FullRestore(RestoreBase):
    """Kingbase全量副本恢复任务执行类
    """

    def __init__(self, pid, job_id, sub_job_id, job_dict):
        super().__init__(pid, job_id, sub_job_id, job_dict)

    def exec_pre_task(self):
        """执行前置任务
        """
        super(FullRestore, self).exec_pre_task()

    def exec_pre_subtask(self):
        """执行前置子任务
        """
        super(FullRestore, self).exec_pre_subtask()

    @restore_exception_decorator
    def exec_task(self):
        """执行任务"""
        LOGGER.info("Start executing restore task of full or incr copy ...")
        cache_path = KingbaseRestoreService.get_cache_mount_path(self.job_dict)
        # 写进度文件，供插件框架读取
        KingbaseRestoreService.write_progress_info(cache_path, "QueryRestoreProcess",
                                                   RestoreProgress(progress=NumberConst.ZERO,
                                                                   message="full restore begin"))
        db_install_path, db_data_path = KingbaseRestoreService.get_db_install_and_data_path(self.job_dict)
        copies = self.job_dict.get("copies", "")
        if not copies:
            LOGGER.error(f"The copies value in the param file is empty or does not exist.")
            raise Exception("The copies value in the param file is empty or does not exist")
        # 全量副本挂载路径
        copy_mount_path = KingbaseRestoreService.get_copy_mount_paths(
            copies[-1], RepositoryDataTypeEnum.DATA_REPOSITORY.value)[0]
        # 主节点或单机备份配置文件
        tgt_role = KingbaseRestoreService.get_current_node_role(self.job_dict)

        # 区分新老副本
        new_copy = copies[-1].get("extendInfo", {}).get("new_copy")
        if new_copy == KbConst.IS_NEW_COPY:
            repo_path = get_sys_rman_configuration_item(db_install_path, self.pid)
            self.job_dict["repo_path"] = repo_path
            KingbaseRestoreProcess.recovery_by_sys_rman(self.job_dict, self.pid, cache_path,
                                                        tgt_role)
            KingbaseRestoreService.record_task_result(self.pid, "Execute restore task success",
                                                      code=ExecuteResultEnum.SUCCESS.value)
            KingbaseRestoreService.write_progress_info(cache_path, "QueryRestoreProcess",
                                                       RestoreProgress(progress=NumberConst.HUNDRED,
                                                                       message="exec restore completed"))
        else:
            self.recovery_by_cp_command(db_install_path, db_data_path, copy_mount_path, cache_path, tgt_role)

        LOGGER.info("Execute restore task of full or incr copy success.")

    def recovery_by_cp_command(self, db_install_path, db_data_path, copy_mount_path, cache_path, tgt_role):
        # 1.清空目标实例data目录
        # 配置文件复原，防止影响本次备份
        KingbaseRestoreProcess.reset_recovery_config(db_data_path, False, tgt_role)

        if tgt_role != str(RoleType.STANDBY.value):
            KingbaseRestoreService.backup_conf_file(db_data_path)
        KingbaseRestoreService.clear_data_dir(KingbaseRestoreService.parse_os_user(self.job_dict), db_data_path)
        # 读取文件系统/ache仓的表空间信息，判断是否有表空间需要恢复，如果有，判断表空间目录存在则清空，不存在则创建
        code = KingbaseRestoreService.clear_table_space_dir(KingbaseRestoreService.parse_os_user(self.job_dict),
                                                            cache_path, copy_mount_path)

        if code != ExecuteResultEnum.SUCCESS:
            LOGGER.error(f"Clear table space failed.")
            raise ErrCodeException(code, message="Clear table space failed.")
        # 2.重做备机/恢复全量副本数据到目标实例
        if tgt_role == str(RoleType.STANDBY.value):
            time.sleep(KbConst.WAIT_EIGHTY_SECONDS)
            KingbaseRestoreProcess.standby_exec_restore_task(self.job_dict, cache_path, db_install_path,
                                                             db_data_path)
        else:
            KingbaseRestoreProcess.singleinst_or_primary_copy_data(self.job_dict, cache_path,
                                                                   db_install_path, db_data_path,
                                                                   job_id=self.job_id)
            # 删除data目录残留的表空间临时文件
            KingbaseRestoreService.del_table_space_info(db_data_path)
            # 删除备份的conf文件
            KingbaseRestoreService.delete_useless_bak_files(db_data_path)
            KingbaseRestoreProcess.singleinst_or_primary_set_full_res_command(self.job_dict, cache_path)

        # 3.清空目标实例archive_status目录
        KingbaseRestoreProcess.clear_archive_status_dir(db_data_path, cache_path)

        # 4.删除目标实例无用的文件
        KingbaseRestoreProcess.delete_useless_files_of_data_dir(db_data_path, cache_path)

        # 5.启动数据库实例，集群备节点启动数据库监控
        KingbaseRestoreProcess.start_kingbase_monitor_or_database(self.pid, self.job_dict, tgt_role)

        # 6.配置文件复原，防止影响下次备份
        KingbaseRestoreProcess.reset_recovery_config(db_data_path, False, tgt_role)

        # 7.清理目标实例归档目录
        deploy_type = self.job_dict.get("targetEnv", {}).get("extendInfo", {}).get("deployType")
        KingbaseRestoreProcess.cleanup_archive_dir(db_install_path, db_data_path, deploy_type, self.job_dict)

    def exec_post_task(self):
        """执行后置任务
        """
        super(FullRestore, self).exec_post_task()

    def exec_init_sys_rman_task(self):
        """
        执行sys_rman初始化任务
        """
        super(FullRestore, self).exec_init_sys_rman_task()

    def report_progress(self, task_name):
        """上报进度
        """
        super(FullRestore, self).report_progress(task_name)

    def abort_task(self):
        """中止任务
        """
        super(FullRestore, self).abort_task()
