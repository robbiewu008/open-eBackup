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

from common import cleaner
from common.common import check_command_injection
from common.const import RepositoryDataTypeEnum, ExecuteResultEnum, RoleType
from common.logger import Logger
from common.number_const import NumberConst
from common.util.cmd_utils import get_livemount_path
from common.util.exec_utils import read_lines_cmd, exec_overwrite_file
from postgresql.common.const import RestoreAction, InstallDeployType, PgConst
from postgresql.common.models import RestoreConfigParam, RestoreProgress
from postgresql.common.util.get_sensitive_utils import get_env_variable
from postgresql.common.util.pg_common_utils import PostgreCommonUtils
from postgresql.common.util.pg_decorators import restore_exception_decorator
from postgresql.restore.pg_cluster_restore_abstract import PostgresClusterRestoreAbstract
from postgresql.restore.pg_restore_service import PostgreRestoreService

LOGGER = Logger().get_logger("postgresql.log")


class ClusterInstFullRestore(PostgresClusterRestoreAbstract):
    """PostgreSQL集群实例全量副本恢复任务执行类
    """

    def __init__(self, pid, job_id, sub_job_id, param_dict):
        super().__init__(pid, job_id, sub_job_id, param_dict)

    def exec_pre_task(self):
        """执行prepare子任务
        """
        pass

    @restore_exception_decorator
    def exec_pre_subtask(self):
        super(ClusterInstFullRestore, self).exec_pre_subtask()

    @restore_exception_decorator
    def exec_restore_subtask(self):
        """执行任务
        """
        node_role = PostgreRestoreService.get_sub_instance_role(self.param_dict)
        LOGGER.info(f"Start executing restore subtask of full copy, node role: {node_role}.")
        # 从节点恢复
        if str(node_role) == str(RoleType.STANDBY.value):
            LOGGER.info("The current node is the standby node.")
            PostgreRestoreService.standby_node_restore(self.pid, self.param_dict)
            return
        cache_path = PostgreRestoreService.get_cache_mount_path(self.param_dict)

        # 1.清空目标实例data目录
        tgt_install_path, tgt_data_path, _ = PostgreRestoreService.get_db_install_and_data_path(self.param_dict)
        tgt_wal_path, tgt_real_wal_path = PostgreRestoreService.get_db_pg_wal_dir_real_path(self.param_dict,
                                                                                            tgt_data_path)
        PostgreRestoreService.backup_conf_file(tgt_data_path, self.job_id)
        PostgreRestoreService.cleanup_archive_after_restore(self.param_dict)
        PostgreRestoreService.clear_data_dir(PostgreRestoreService.parse_os_user(self.param_dict), tgt_data_path)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=15, message="delete data dir success"))

        # 1.1 清空目标实例的table space目录
        job_dict = self.param_dict.get("job", {})
        copies = PostgreRestoreService.parse_copies(job_dict)
        copy_mount_path = PostgreRestoreService.get_copy_mount_paths(
            copies[0], RepositoryDataTypeEnum.DATA_REPOSITORY.value)[0]
        copy_mount_path = get_livemount_path(self.job_id, copy_mount_path)
        PostgreRestoreService.clear_table_space_dir(copy_mount_path=copy_mount_path)

        # 2.恢复全量副本数据到目标实例
        self.restore_full_data(cache_path, job_dict, tgt_data_path, tgt_wal_path, tgt_real_wal_path)

        # 3.清空目标实例archive_status目录
        tgt_obj_extend_info_dict = job_dict.get("targetObject", {}).get("extendInfo", {})
        tgt_version = tgt_obj_extend_info_dict.get("version", "")
        PostgreRestoreService.clear_archive_status_dir(tgt_version, tgt_data_path)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=NumberConst.SEVENTY_ONE,
                                                               message="clear archive_status success"))

        # 4.删除目标实例无用的文件
        PostgreRestoreService.delete_useless_files_of_data_dir(tgt_data_path)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=NumberConst.SEVENTY_TWO,
                                                               message="delete useless files success"))

        # 5.配置恢复命令
        self.set_recovery_command_and_start_instance(cache_path, job_dict, node_role)

    def restore_full_data(self, cache_path, job_dict, tgt_data_path, tgt_wal_path, tgt_real_wal_path):
        copies = PostgreRestoreService.parse_copies(job_dict)
        copy_mount_path = PostgreRestoreService.get_copy_mount_paths(
            copies[0], RepositoryDataTypeEnum.DATA_REPOSITORY.value)[0]
        tgt_obj_extend_info_dict = job_dict.get("targetObject", {}).get("extendInfo", {})
        tgt_db_os_user = tgt_obj_extend_info_dict.get("osUsername", "")
        PostgreRestoreService.change_owner_of_download_data(self.param_dict, copy_mount_path)
        PostgreRestoreService.restore_data(cache_path, copy_mount_path,
                                           tgt_data_path, job_id=self.job_id)
        PostgreRestoreService.restore_pg_wal_dir(tgt_wal_path, tgt_real_wal_path, job_id=self.job_id)
        # 获取副本id
        copy_id = copies[0].get("id")
        # 恢复完数据后删除没用的文件
        repl_info_name = os.path.join(tgt_data_path, f"Repl_{copy_id}.info")
        if not check_command_injection(repl_info_name) and os.path.isfile(repl_info_name) and not os.path.islink(
                repl_info_name):
            os.remove(repl_info_name)
        PostgreRestoreService.restore_conf_file(tgt_data_path, self.job_id)
        # CLup集群类型恢复后需要清空主节点的postgresql.auto.conf中primary_conninfo字段
        install_deploy_type = job_dict.get("targetEnv", {}).get("extendInfo", {}).get(
            "installDeployType", InstallDeployType.PGPOOL)
        if install_deploy_type == InstallDeployType.CLUP:
            postgresql_auto_conf_path = os.path.join(tgt_data_path, PgConst.POSTGRESQL_AUTO_CONF_FILE_NAME)
            ret, mount_list = read_lines_cmd(postgresql_auto_conf_path)
            if mount_list:
                lines = [line for line in mount_list if not line.startswith("primary_conninfo =")]
                lines_conn = '\n'.join(lines) + '\n'
                exec_overwrite_file(postgresql_auto_conf_path, lines_conn, json_flag=False)
        PostgreCommonUtils.write_progress_info(
            cache_path, RestoreAction.QUERY_RESTORE,
            RestoreProgress(progress=NumberConst.SEVENTY, message="restore data success"))

    def set_recovery_command_and_start_instance(self, cache_path, job_dict, node_role):
        primary_ip, primary_inst_port = PostgreRestoreService.get_primary_ip_and_instance_port(self.param_dict)
        repl_user = job_dict.get("targetObject", {}).get("auth", {}).get("extendInfo", {}).get("dbStreamRepUser")
        repl_pwd = get_env_variable(f"job_targetObject_auth_extendInfo_dbStreamRepPwd_{self.pid}")
        copy_repl_user, copy_repl_pwd = PostgreCommonUtils.get_repl_info(job_dict)
        if copy_repl_user:
            repl_user = copy_repl_user
            repl_pwd = copy_repl_pwd
        try:
            cfg_extend_info = {
                "primary_ip": primary_ip,
                "primary_inst_port": primary_inst_port,
                "db_stream_rep_user": repl_user,
                "db_stream_rep_pwd": repl_pwd
            }
            tgt_install_path, tgt_data_path, tgt_archive_dir = PostgreRestoreService.get_db_install_and_data_path(
                self.param_dict)
            tgt_archive_path = PostgreCommonUtils.get_archive_path_offline(tgt_data_path, archive_dir=tgt_archive_dir)
            tgt_obj_extend_info_dict = job_dict.get("targetObject", {}).get("extendInfo", {})
            tgt_db_os_user = tgt_obj_extend_info_dict.get("osUsername", "")
            tgt_version = tgt_obj_extend_info_dict.get("version", "")
            cfg_param = RestoreConfigParam(
                system_user=tgt_db_os_user, target_version=tgt_version, target_install_path=tgt_install_path,
                target_data_path=tgt_data_path, target_archive_path=tgt_archive_path, extend_info=cfg_extend_info
            )
            PostgreRestoreService.set_recovery_conf_file(self.param_dict, cfg_param, role=node_role)
        finally:
            cleaner.clear(repl_pwd)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=NumberConst.SEVENTY_FIVE,
                                                               message="set recovery command success"))
        PostgreRestoreService.start_db_after_restore(self.pid, self.param_dict, cache_path, node_role=node_role)

    @restore_exception_decorator
    def exec_post_subtask(self):
        super(ClusterInstFullRestore, self).exec_post_subtask()

    @restore_exception_decorator
    def exec_post_task(self):
        """执行后置任务
        """
        LOGGER.info("Start executing restore post ...")
        cache_path = PostgreRestoreService.get_cache_mount_path(self.param_dict)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE_POST,
                                               RestoreProgress(progress=0, message="begin"))

        tgt_install_path, tgt_data_path, _ = PostgreRestoreService.get_db_install_and_data_path(self.param_dict)
        PostgreRestoreService.delete_useless_bak_files(tgt_data_path, self.job_id)

        LOGGER.info("Execute restore post task success.")
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE_POST,
                                               RestoreProgress(progress=100, message="completed"))
        PostgreRestoreService.record_task_result(self.pid, "Execute restore post task success",
                                                 code=ExecuteResultEnum.SUCCESS.value)

    def abort_task(self):
        """中止任务
        """
        super(ClusterInstFullRestore, self).abort_task()
