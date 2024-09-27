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

from common.common import is_clone_file_system_for_kingbase
from common.const import ExecuteResultEnum, RepositoryDataTypeEnum
from common.const import RoleType
from common.enums.common_enums import DeployTypeEnum
from common.logger import Logger
from common.number_const import NumberConst
from common.util import check_user_utils
from kingbase.common.const import KbConst
from kingbase.common.error_code import ErrorCode
from kingbase.common.kb_exception import ErrCodeException
from kingbase.common.models import RestoreConfigParam, RestoreProgress
from kingbase.common.util import resource_util
from kingbase.common.util.resource_util import check_special_character, check_white_list
from kingbase.restore.kb_restore_service import KingbaseRestoreService

LOGGER = Logger().get_logger(filename="kingbase.log")


class KingbaseRestoreProcess:

    @staticmethod
    def check_db_status_before_restore(pid, job_dict, cache_path, process_stage):
        LOGGER.info("Check db status before restore start.")
        db_system_user = KingbaseRestoreService.get_db_system_user(job_dict)
        db_install_path, db_data_path = KingbaseRestoreService.get_db_install_and_data_path(job_dict)
        is_running = KingbaseRestoreService.is_db_running(db_system_user, db_install_path, db_data_path)
        # 数据库正在运行，提示用户先停止数据库
        if is_running:
            LOGGER.error("The target instance is still running when restoring.")
            err_msg = f"The target instance is still running when restoring."
            KingbaseRestoreService.record_task_result(pid, err_msg, err_code=ErrorCode.DB_RUNNING_ERR_BEFORE_RESTORE)
            KingbaseRestoreService.write_progress_info(
                cache_path, process_stage,
                RestoreProgress(progress=NumberConst.TWENTY,
                                message="database is running, restore failed",
                                err_code=ErrorCode.DB_RUNNING_ERR_BEFORE_RESTORE)
            )
            return False
        LOGGER.info("Check db status before restore success.")
        KingbaseRestoreService.write_progress_info(cache_path, process_stage,
                                                   RestoreProgress(progress=NumberConst.TWENTY,
                                                                   message="database not running"))
        return True

    @staticmethod
    def check_sys_user_before_restore(pid, job_dict, cache_path, process_stage):
        LOGGER.info("Check restore to same user start.")
        db_install_path, db_data_path = KingbaseRestoreService.get_db_install_and_data_path(job_dict)
        # 恢复目标实例数据目录所属用户
        tgt_db_system_user = KingbaseRestoreService.get_path_owner(db_data_path)
        LOGGER.info(f"Check if restore to same user, "
                    f"target data path: {db_data_path}, owner: {tgt_db_system_user}")
        copies = job_dict.get("copies", [])
        if KingbaseRestoreService.is_log_restore(job_dict):
            copy_mount_path = KingbaseRestoreService.get_copy_mount_paths(
                copies[-2], RepositoryDataTypeEnum.DATA_REPOSITORY.value)[0]
        else:
            copy_mount_path = KingbaseRestoreService.get_copy_mount_paths(
                copies[-1], RepositoryDataTypeEnum.DATA_REPOSITORY.value)[0]
        check_white_list([copy_mount_path])
        # 副本数据所属系统用户
        copy_system_user = KingbaseRestoreService.get_copy_os_username(copies)
        LOGGER.info(f"Check if restore to same user, copy data path: {copy_mount_path}, owner: {copy_system_user}")
        if tgt_db_system_user != copy_system_user:
            err_msg = f"The user of the database instance installation directory is different from " \
                      f"the system user running the database instance."
            KingbaseRestoreService.record_task_result(pid, err_msg, err_code=ErrorCode.OS_USER_NOT_EQUAL_BEFORE_RESTORE)
            KingbaseRestoreService.write_progress_info(
                cache_path, process_stage,
                RestoreProgress(progress=NumberConst.SEVENTY_FIVE,
                                message="user is not the same, restore failed",
                                err_code=ErrorCode.OS_USER_NOT_EQUAL_BEFORE_RESTORE)
            )
            return False
        LOGGER.info("Check restore to same user success.")
        KingbaseRestoreService.write_progress_info(cache_path, process_stage,
                                                   RestoreProgress(progress=NumberConst.SEVENTY_FIVE,
                                                                   message="restore to same user"))
        return True

    @staticmethod
    def check_rw_permission_before_restore(pid, job_dict, cache_path, process_stage):
        db_install_path, db_data_path = KingbaseRestoreService.get_db_install_and_data_path(job_dict)
        # 目标实例数据目录的父目录
        db_data_upper_path = os.path.realpath(os.path.join(db_data_path, ".."))
        copies = job_dict.get("copies", [])
        if KingbaseRestoreService.is_log_restore(job_dict):
            copy_mount_path = KingbaseRestoreService.get_copy_mount_paths(
                copies[-2], RepositoryDataTypeEnum.DATA_REPOSITORY.value)[0]
        else:
            copy_mount_path = KingbaseRestoreService.get_copy_mount_paths(
                copies[-1], RepositoryDataTypeEnum.DATA_REPOSITORY.value)[0]
        check_white_list([copy_mount_path])
        # 副本数据所属系统用户
        copy_system_user = KingbaseRestoreService.get_copy_os_username(copies)
        is_dir_w_and_r = KingbaseRestoreService.is_dir_readable_and_writable_for_input_user(
            db_data_upper_path, copy_system_user)
        if not is_dir_w_and_r:
            err_msg = f"The parent directory of the target instance data directory is not readable " \
                      f"and writable by the system user running the database instance."
            KingbaseRestoreService.record_task_result(pid, err_msg, err_code=ErrorCode.TARGET_NOT_RW_BEFORE_RESTORE)
            KingbaseRestoreService.write_progress_info(
                cache_path, process_stage,
                RestoreProgress(progress=NumberConst.HUNDRED,
                                message="check rw permission failed",
                                err_code=ErrorCode.TARGET_NOT_RW_BEFORE_RESTORE)
            )
            return False
        return True

    @staticmethod
    def check_target_max_connections_is_greater_than_or_equal_to_original(pid, job_dict, cache_path, process_stage):
        db_install_path, db_data_path = KingbaseRestoreService.get_db_install_and_data_path(job_dict)
        copies = job_dict.get("copies", [])
        if KingbaseRestoreService.is_log_restore(job_dict):
            copy_mount_path = KingbaseRestoreService.get_copy_mount_paths(
                copies[-2], RepositoryDataTypeEnum.DATA_REPOSITORY.value)[0]
        else:
            copy_mount_path = KingbaseRestoreService.get_copy_mount_paths(
                copies[-1], RepositoryDataTypeEnum.DATA_REPOSITORY.value)[0]
        check_white_list([copy_mount_path])

        # 老副本需要检查最大连接数，不满足会导致集群启动失败
        tgt_config_file = os.path.realpath(os.path.join(db_data_path, KbConst.CLUSTER_CONF_FILE_NAME))
        if not os.path.exists(tgt_config_file):
            LOGGER.warning("Target cluster conf file name is not exist!")
            return True
        # 恢复目标主机的最大连接数
        tgt_max_connections = KingbaseRestoreService.get_max_connections(tgt_config_file)

        # 判断是否是新副本恢复，新副本会发下
        new_copy = job_dict.get("copies", "")[-1].get("extendInfo", {}).get("new_copy")
        if new_copy == KbConst.IS_NEW_COPY:
            org_max_connections = job_dict.get("copies", "")[-1].get("extendInfo", {}).get("max_connections")
        else:
            org_config_file = os.path.realpath(os.path.join(copy_mount_path, KbConst.CLUSTER_CONF_FILE_NAME))
            if not os.path.exists(org_config_file):
                err_msg = "Get original cluster conf file failed!"
                KingbaseRestoreService.record_task_result(pid, err_msg, err_code=ErrorCode.AGENT_INTERNAL_ERROR)
                KingbaseRestoreService.write_progress_info(cache_path, process_stage,
                                                           RestoreProgress(progress=NumberConst.HUNDRED,
                                                                           message=err_msg,
                                                                           err_code=ErrorCode.AGENT_INTERNAL_ERROR))
                return False
            # 恢复所用副本的最大连接数
            org_max_connections = KingbaseRestoreService.get_max_connections(org_config_file)

        if int(tgt_max_connections) < int(org_max_connections):
            err_msg = f"Target max_connections:{tgt_max_connections} is smaller than" \
                      f"original:{org_max_connections}, check failed!"
            KingbaseRestoreService.record_task_result(
                pid, err_msg, err_code=ErrorCode.TARGET_MAX_CONNECTIONS_MUST_GREATER_THAN_OR_EQUAL_TO_ORIGINAL
            )
            KingbaseRestoreService.write_progress_info(
                cache_path, process_stage,
                RestoreProgress(progress=NumberConst.HUNDRED,
                                message=err_msg,
                                err_param=[tgt_max_connections,
                                           org_max_connections],
                                err_code=ErrorCode.TARGET_MAX_CONNECTIONS_MUST_GREATER_THAN_OR_EQUAL_TO_ORIGINAL)
            )
            return False
        LOGGER.info(f"Execute restore prerequisite task success")
        KingbaseRestoreService.record_task_result(pid, "Execute restore prerequisite task success",
                                                  code=ExecuteResultEnum.SUCCESS.value)
        KingbaseRestoreService.write_progress_info(cache_path, process_stage,
                                                   RestoreProgress(progress=NumberConst.HUNDRED,
                                                                   message="restore pre check completed"))
        return True

    @staticmethod
    def standby_exec_restore_task(job_dict, cache_path, db_install_path, db_data_path):
        primary_ip = KingbaseRestoreService.get_primary_node_ip(job_dict)
        db_system_user = KingbaseRestoreService.get_db_system_user(job_dict)
        KingbaseRestoreService.standby_clone(db_system_user, db_install_path, primary_ip)
        LOGGER.info("Execute standby restore task success.")
        KingbaseRestoreService.write_progress_info(cache_path, "QueryRestoreProcess",
                                                   RestoreProgress(progress=NumberConst.NINETY,
                                                                   message="standby restore completed"))
        return

    @staticmethod
    def recovery_by_sys_rman(job_dict, pid, cache_path, tgt_role):
        db_install_path, db_data_path = KingbaseRestoreService.get_db_install_and_data_path(job_dict)
        if tgt_role == str(RoleType.STANDBY.value):
            # 集群备节点
            KingbaseRestoreService.restore_standby_node_by_sys_rman(job_dict, cache_path, db_install_path, db_data_path,
                                                                    pid)
        else:
            # 单机或集群主节点
            KingbaseRestoreService.restore_single_or_primary_node_by_sys_rman(job_dict, pid, cache_path,
                                                                              db_install_path, db_data_path)

    @staticmethod
    def singleinst_or_primary_copy_data(job_dict, cache_path, db_install_path, db_data_path, job_id=""):
        copies = job_dict.get("copies")
        if not copies:
            LOGGER.error(f"The copies value in the param file is empty or does not exist.")
            raise Exception("The copies value in the param file is empty or does not exist")

        # 全量副本挂载路径
        if KingbaseRestoreService.is_log_restore(job_dict):
            copy_mount_path = KingbaseRestoreService.get_copy_mount_paths(
                copies[-2], RepositoryDataTypeEnum.DATA_REPOSITORY.value)[0]
        else:
            copy_mount_path = KingbaseRestoreService.get_copy_mount_paths(
                copies[-1], RepositoryDataTypeEnum.DATA_REPOSITORY.value)[0]

        # 恢复表空间
        restore_table_space_res = KingbaseRestoreService.restore_table_space(copy_mount_path, cache_path, job_id)
        if restore_table_space_res != ExecuteResultEnum.SUCCESS:
            LOGGER.error("Restore table space failed.")
            raise ErrCodeException(restore_table_space_res, message="Restore table space failed!")

        # 目标实例data的上层目录
        if is_clone_file_system_for_kingbase(job_dict):
            KingbaseRestoreService.change_auth_of_download_data(db_install_path, copy_mount_path)
        KingbaseRestoreService.copy_directory(copy_mount_path, db_data_path, wildcard=".", job_id=job_id)
        # 恢复配置文件
        KingbaseRestoreService.restore_conf_file(db_data_path)
        LOGGER.info("singleInst or primary copy data success.")
        KingbaseRestoreService.write_progress_info(cache_path, "QueryRestoreProcess",
                                                   RestoreProgress(progress=NumberConst.TWENTY,
                                                                   message="copy data success"))
        return

    @staticmethod
    def singleinst_or_primary_set_full_res_command(job_dict, cache_path):
        # 配置全量副本恢复命令
        deploy_type = job_dict.get("targetEnv", {}).get("extendInfo", {}).get("deployType")
        db_system_user = KingbaseRestoreService.get_db_system_user(job_dict)
        db_install_path, db_data_path = KingbaseRestoreService.get_db_install_and_data_path(job_dict)
        tgt_archive_path = KingbaseRestoreService.get_archive_path_offline(deploy_type, job_dict)
        check_special_character([db_system_user, tgt_archive_path])
        cfg_param = RestoreConfigParam(
            system_user=db_system_user, target_install_path=db_install_path,
            target_data_path=db_data_path, target_archive_path=tgt_archive_path
        )
        resource_util.check_is_path_exists(tgt_archive_path)
        if deploy_type == DeployTypeEnum.SINGLE.value:
            KingbaseRestoreService.set_single_inst_recovery_conf_file(cfg_param)
        elif deploy_type == DeployTypeEnum.AP.value:
            KingbaseRestoreService.set_cluster_recovery_conf_file(cfg_param)
        LOGGER.info("Set restore command of singleInst or primary success.")
        KingbaseRestoreService.write_progress_info(cache_path, "QueryRestoreProcess",
                                                   RestoreProgress(progress=NumberConst.FORTY,
                                                                   message="set res command success"))
        return

    @staticmethod
    def singleinst_or_primary_set_pitr_res_command(job_dict, cache_path, log_path):
        # 配置日志副本恢复命令
        db_install_path, db_data_path = KingbaseRestoreService.get_db_install_and_data_path(job_dict)
        db_system_user = KingbaseRestoreService.get_db_system_user(job_dict)
        deploy_type = job_dict.get("targetEnv", {}).get("extendInfo", {}).get("deployType")
        recovery_timestamp = job_dict.get("extendInfo", {}).get("restoreTimestamp")
        recovery_tgt_time = KingbaseRestoreService.convert_timestamp_to_datetime(recovery_timestamp)
        tgt_archive_path = KingbaseRestoreService.get_archive_path_offline(deploy_type, job_dict)
        check_special_character([db_system_user, tgt_archive_path, log_path, recovery_tgt_time])
        cfg_param = RestoreConfigParam(
            system_user=db_system_user, target_install_path=db_install_path,
            target_data_path=db_data_path, target_archive_path=tgt_archive_path,
            log_copy_path=log_path, recovery_target_time=recovery_tgt_time)
        if deploy_type == DeployTypeEnum.SINGLE.value:
            KingbaseRestoreService.set_single_inst_recovery_conf_file(cfg_param)
        elif deploy_type == DeployTypeEnum.AP.value:
            KingbaseRestoreService.set_cluster_recovery_conf_file(cfg_param)
        LOGGER.info("Set restore command of singleInst or primary success.")
        KingbaseRestoreService.write_progress_info(cache_path, "QueryRestoreProcess",
                                                   RestoreProgress(progress=NumberConst.FORTY,
                                                                   message="set res command success"))
        return

    @staticmethod
    def clear_archive_status_dir(tgt_data_path: str, cache_path):
        if not os.path.isdir(tgt_data_path):
            LOGGER.error(f"Clear archive_status dir, target data path: {tgt_data_path} is invalid.")
            raise Exception("Target data path is invalid when clearing archive_status directory")
        archive_status_dir = os.path.join(tgt_data_path, "sys_wal", "archive_status")
        KingbaseRestoreService.clear_dir_when_exist(archive_status_dir)
        LOGGER.info("Clear archive status dir success.")
        KingbaseRestoreService.write_progress_info(cache_path, "QueryRestoreProcess",
                                                   RestoreProgress(progress=NumberConst.SIXTY,
                                                                   message="clear archive status success"))

    @staticmethod
    def delete_useless_files_of_data_dir(tgt_data_path, cache_path):
        for file_name in KbConst.DELETE_FILE_NAMES_OF_DATA_DIR:
            tmp_path = os.path.join(tgt_data_path, file_name)
            KingbaseRestoreService.delete_path(tmp_path)
        LOGGER.info("Delete useless files of data dir success.")
        KingbaseRestoreService.write_progress_info(cache_path, "QueryRestoreProcess",
                                                   RestoreProgress(progress=NumberConst.EIGHTY,
                                                                   message="clear useless files success"))

    @staticmethod
    def cleanup_archive_dir(tgt_install_path, tgt_data_path, deploy_type, job_dict):
        tgt_archive_path = KingbaseRestoreService.get_archive_path_offline(deploy_type, job_dict)
        if not tgt_archive_path or not os.path.isdir(tgt_archive_path):
            LOGGER.warning("The obtained archive dir is empty or does not exist.")
            return
        control_data_tool = os.path.realpath(os.path.join(tgt_install_path, "bin", "sys_controldata"))
        if not os.path.isfile(control_data_tool):
            LOGGER.warning(f"The file: {control_data_tool} does not exist.")
            return
        kb_ctl_path = os.path.realpath(os.path.join(tgt_install_path, "bin", "sys_ctl"))
        kb_ctl_os_user = KingbaseRestoreService.get_path_owner(kb_ctl_path)
        if not resource_util.check_os_name(kb_ctl_os_user, tgt_install_path):
            LOGGER.warning("The Os username is not exist.")
            return
        if not check_user_utils.check_path_owner(control_data_tool, [kb_ctl_os_user]):
            LOGGER.warning("The control data tool and os username is not matching.")
            return
        if not check_user_utils.check_path_owner(tgt_data_path, [kb_ctl_os_user]):
            LOGGER.warning("The data dir and os username is not matching.")
            return
        wal_file_name = KingbaseRestoreService.get_latest_redo_wal_file(kb_ctl_os_user, control_data_tool,
                                                                        tgt_data_path)
        if not wal_file_name:
            LOGGER.warning("The obtained latest redo wal file name is empty.")
            return
        KingbaseRestoreService.manual_cleanup_archive_dir(tgt_archive_path, wal_file_name)
        sys_wal_dir = os.path.realpath(os.path.join(tgt_data_path, "sys_wal"))
        KingbaseRestoreService.manual_cleanup_archive_dir(sys_wal_dir, wal_file_name)

    @staticmethod
    def start_kingbase_monitor_or_database(pid, job_dict, tgt_role):
        LOGGER.info("-------------------begin start db------------------------------")
        db_install_path, db_data_path = KingbaseRestoreService.get_db_install_and_data_path(job_dict)
        cache_path = KingbaseRestoreService.get_cache_mount_path(job_dict)
        db_system_user = KingbaseRestoreService.get_db_system_user(job_dict)
        if not resource_util.check_os_name(db_system_user, db_install_path):
            raise ErrCodeException(ErrorCode.EXEC_START_DB_CMD_FAILED,
                                   message="Os username is not exist.")
        KingbaseRestoreService.start_kingbase_database(db_system_user, db_install_path, db_data_path)
        if tgt_role == str(RoleType.PRIMARY.value):
            time.sleep(KbConst.WAIT_TEN_SECONDS)
            KingbaseRestoreService.clear_useless_role_record(db_system_user, db_install_path)
            # 新位置恢复清理主备关系后，数据库会暂时处于recovery状态，需等待60s再做后续操作
            time.sleep(KbConst.WAIT_SIX_SECONDS)
            KingbaseRestoreService.primary_register(db_system_user, db_install_path)
        if tgt_role == str(RoleType.STANDBY.value):
            KingbaseRestoreService.standby_register(db_system_user, db_install_path)
            KingbaseRestoreService.start_kingbase_monitor(db_system_user, db_install_path, db_data_path)
        KingbaseRestoreService.record_task_result(pid, "Execute restore task success",
                                                  code=ExecuteResultEnum.SUCCESS.value)
        KingbaseRestoreService.write_progress_info(cache_path, "QueryRestoreProcess",
                                                   RestoreProgress(progress=NumberConst.HUNDRED,
                                                                   message="exec restore completed"))

    @staticmethod
    def reset_recovery_config(tgt_data_path, is_pit, role=RoleType.NONE_TYPE):
        LOGGER.info(f"Try to reset recovery config, role: {role}.")
        if role == str(RoleType.STANDBY.value):
            LOGGER.info("Current node is standby, no need reset recovery config.")
            return
        if role == str(RoleType.PRIMARY.value):
            kb_conf = KbConst.CLUSTER_CONF_FILE_NAME
        else:
            kb_conf = KbConst.KINGBASE_CONF_FILE_NAME
        kb_conf_file = os.path.realpath(os.path.join(tgt_data_path, kb_conf))
        if not is_pit:
            # 复位恢复命令时log路径没有用，所以设为“”
            KingbaseRestoreService.set_restore_command(kb_conf_file, "", rollback=True)
            LOGGER.info("Reset full copy recovery config success.")
            return
        KingbaseRestoreService.set_restore_command(kb_conf_file, "", rollback=True)
        # 复位恢复命令时恢复目标时间没有用，所以设为“”
        KingbaseRestoreService.set_pitr_target_time(kb_conf_file, "", rollback=True)
        KingbaseRestoreService.set_recovery_target_action(kb_conf_file, "pause", rollback=True)
        LOGGER.info("Reset point-in-time recovery config success.")
