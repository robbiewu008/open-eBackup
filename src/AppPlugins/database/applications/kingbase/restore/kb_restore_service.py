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

import datetime
import json
import locale
import os
import platform
import re
import shutil
import stat
import threading
import time
import uuid

import pexpect
import psutil

from common import common as res_base_common
from common.common import execute_cmd, get_host_sn
from common.common_models import ActionResult, SubJobModel
from common.const import DeployType, RepositoryDataTypeEnum, ExecuteResultEnum, RoleType, SubJobTypeEnum, \
    SubJobPolicyEnum
from common.enums.common_enums import DeployTypeEnum
from common.logger import Logger
from common.number_const import NumberConst
from common.parse_parafile import get_env_variable
from common.util import check_user_utils, check_utils
from common.util.backup import query_progress, backup, backup_files
from common.util.checkout_user_utils import checkout_user_and_execute_cmd, get_path_owner
from kingbase.common.const import KbConst, CmdRetCode, BackupStatus, DirAndFileNameConst, ConfigKeyStatus
from kingbase.common.error_code import ErrorCode
from kingbase.common.kb_exception import ErrCodeException
from kingbase.common.models import RestoreConfigParam
from kingbase.common.models import RestoreProgress
from kingbase.common.util import resource_util
from kingbase.common.util.get_html_result_utils import execute_cmd_and_parse_res
from kingbase.common.util.resource_util import check_file_path, check_dir_path, get_conf_item_status, \
    check_black_list, \
    check_is_path_exists, create_soft_link
from kingbase.common.util.resource_util import check_special_character, check_white_list, get_parallel_process, \
    get_sys_rman_configuration_item, get_db_version_id_and_system_id

if platform.system() == "Linux":
    import pwd

LOGGER = Logger().get_logger(filename="kingbase.log")
GLOBAL_PG_PROCESS_FILE_LOCK = threading.Lock()


def build_sub_job(job_id, job_name, priority, node_id):
    return SubJobModel(jobId=job_id, subJobId="", jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value, jobName=job_name,
                       jobPriority=priority, policy=SubJobPolicyEnum.FIXED_NODE.value, ignoreFailed=False,
                       execNodeId=node_id, jobInfo="") \
        .dict(by_alias=True)


class KingbaseRestoreService:
    @staticmethod
    def gen_sub_job(pid, job_id, param_dict):
        LOGGER.info("Start executing generate restore sub job ...")
        sub_objs = param_dict.get("job", {}).get("restoreSubObjects", [])
        # 将集群子实例排序，主节点实例放在首位
        ordered_sub_jobs = list()
        for obj in sub_objs:
            if str(obj.get("extendInfo", {}).get("role")) == str(RoleType.PRIMARY.value):
                ordered_sub_jobs.insert(0, obj)
                continue
            ordered_sub_jobs.append(obj)
        tmp_priority = 1
        sub_jobs = list()
        # prepare 各节点同时执行
        for idx, obj in enumerate(ordered_sub_jobs):
            tmp_node_id = obj.get("extendInfo", {}).get("hostId")
            if idx == 0:
                sub_jobs.append(build_sub_job(job_id, "prepare", tmp_priority, tmp_node_id))
                continue
            tmp_priority += 1
            sub_jobs.append(build_sub_job(job_id, "prepare", tmp_priority, tmp_node_id))
        # restore 先主，再依次从
        tmp_priority += 1
        for idx, obj in enumerate(ordered_sub_jobs):
            tmp_node_id = obj.get("extendInfo", {}).get("hostId")
            if idx == 0:
                sub_jobs.append(build_sub_job(job_id, "restore", tmp_priority, tmp_node_id))
                continue
            tmp_priority += 1
            sub_jobs.append(build_sub_job(job_id, "restore", tmp_priority, tmp_node_id))
        # init 只在主节点执行，重新初始化
        tmp_priority += 1
        for idx, obj in enumerate(ordered_sub_jobs):
            tmp_node_id = obj.get("extendInfo", {}).get("hostId")
            if idx == 0:
                sub_jobs.append(build_sub_job(job_id, "init", tmp_priority, tmp_node_id))
                continue
            tmp_priority += 1
            sub_jobs.append(build_sub_job(job_id, "init", tmp_priority, tmp_node_id))
        # post 先主，再同时从
        tmp_priority += 1
        for idx, obj in enumerate(ordered_sub_jobs):
            tmp_node_id = obj.get("extendInfo", {}).get("hostId")
            if idx == 0:
                sub_jobs.append(build_sub_job(job_id, "post", tmp_priority, tmp_node_id))
                tmp_priority += 1
                continue
            sub_jobs.append(build_sub_job(job_id, "post", tmp_priority, tmp_node_id))
        LOGGER.info(f"Execute generate sub job task of full or incr copy success. Sub Jobs: {sub_jobs}.")
        res_base_common.output_result_file(pid, sub_jobs)

    @staticmethod
    def is_restore_cluster(job_dict: dict):
        """判断是否在恢复集群
        """
        deploy_type = job_dict.get("targetEnv", {}).get("extendInfo", {}).get("deployType")
        if deploy_type not in (DeployTypeEnum.SINGLE.value, DeployTypeEnum.AP.value):
            LOGGER.error(f"Deploy type param: {deploy_type} is invalid.")
            raise Exception("Deploy type param is invalid")
        return deploy_type == DeployTypeEnum.AP.value

    @staticmethod
    def get_current_node_role(job_dict):
        tgt_role = ''
        ips = KingbaseRestoreService.get_local_ips()
        nodes = job_dict.get("targetEnv", {}).get("nodes", [])
        for node in nodes:
            if resource_util.get_tmp_node_ip(node) in ips:
                tgt_role = node.get("extendInfo", {}).get("role", "")
                break
        LOGGER.info(f"Current node's role is: {tgt_role}.")
        return tgt_role

    @staticmethod
    def get_primary_node_ip(job_dict):
        primary_ip = ''
        nodes = job_dict.get("targetEnv", {}).get("nodes", [])
        for node in nodes:
            if node.get("extendInfo", {}).get("role", "") == str(RoleType.PRIMARY.value):
                primary_ip = node.get("extendInfo", {}).get("serviceIp", "")
                break
        LOGGER.info(f"Primary node's ip is: {primary_ip}.")
        check_special_character([primary_ip])
        if not primary_ip or not check_utils.is_ip_address(primary_ip):
            raise Exception(f"Primary ip is in incorrect!")
        return primary_ip

    @staticmethod
    def clear_dir_when_exist(clear_path):
        LOGGER.info(f"Start clearing path: {clear_path}")
        if not os.path.isdir(clear_path):
            LOGGER.info(f"The path: {clear_path} does not exist when trying to clear it.")
            return
        check_black_list([clear_path])
        res_base_common.clean_dir(clear_path)

    @staticmethod
    def clear_data_dir(os_user, data_path):
        """
        恢复前处理数据目录，存在则清空；不存在则创建
        """
        if not os.path.exists(data_path):
            LOGGER.warning(f"Try to create database data directory: {data_path} when it does not exist ...")
            # data目录权限必须为700
            os.makedirs(data_path, mode=0o700)
            uid, gid = KingbaseRestoreService.get_uid_gid_by_os_user(os_user)
            LOGGER.info(f"Change owner of the data path: {data_path}, uid: {uid}, gid: {gid}.")
            os.lchown(data_path, uid, gid)
            LOGGER.info(f"Change owner of the data path: {data_path} success.")
        else:
            KingbaseRestoreService.clear_dir_when_exist(data_path)

    @staticmethod
    def get_archive_path_offline(deploy_type, job_dict):
        archive_path = ""
        kb_conf_file_path = ""
        copies = job_dict.get("copies")
        copy_mount_path = KingbaseRestoreService.get_copy_mount_paths(
            copies[0], RepositoryDataTypeEnum.DATA_REPOSITORY.value)[0]
        if deploy_type == DeployTypeEnum.SINGLE.value:
            kb_conf_file_path = os.path.join(copy_mount_path, KbConst.KINGBASE_CONF_FILE_NAME)
        elif deploy_type == DeployTypeEnum.AP.value:
            kb_conf_file_path = os.path.join(copy_mount_path, KbConst.CLUSTER_CONF_FILE_NAME)
        if not os.path.isfile(kb_conf_file_path):
            LOGGER.error(f"Kingbase config file path: {kb_conf_file_path} is invalid.")
            raise Exception("Kingbase config file path is invalid")
        with open(kb_conf_file_path, "r", encoding="utf-8") as file:
            lines = file.readlines()
            for i in lines:
                if not i.strip() or i.strip().startswith("#"):
                    continue
                if all([i.strip().startswith("archive_command"), "=" in i, "%p" in i, "/%f" in i]):
                    tmp_splits = i.split("%p")
                    archive_path = tmp_splits[1].split("/%f")[0].strip().strip("\'\"") if len(tmp_splits) > 1 else ""
                    break
        if not os.path.exists(archive_path):
            LOGGER.error("Path: %s not exist.", archive_path)
            raise ErrCodeException(ErrorCode.ARCHIVE_LOG_PATH_IS_NOT_EXIST,
                                   message="Path: %s not exist." % archive_path)
        return archive_path

    @staticmethod
    def get_latest_redo_wal_file(os_user, control_data_tool, db_data_path):
        wal_file_name = ""
        LOGGER.info(f"Try to get latest redo wal file name ...")
        kb_ctl_data_cmd = f'su - {os_user} -c "{control_data_tool} -D {db_data_path}"'
        LOGGER.info(f"Get latest redo wal file name command: {kb_ctl_data_cmd}.")
        return_code, std_out, std_err = res_base_common.execute_cmd(kb_ctl_data_cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Execute kb_controldata command failed, return code: {return_code}, out: {std_out}, "
                         f"err: {std_err}.")
            return wal_file_name
        items = str(std_out).split(os.linesep)
        for i in items:
            if "REDO WAL file" in str(i).strip():
                wal_file_name = str(i).strip().split(":")[1].strip()
                break
        LOGGER.info(f"Get latest redo wal file name: {wal_file_name} success.")
        return wal_file_name

    @staticmethod
    def is_wal_file(file_name):
        """
        是否WAL文件名称
        WAL segment file文件名称为24个字符，由3部分组成，每个部分是8个字符，每个字符是一个16进制值（即0~F）
        第1部分是TimeLineID，取值范围是0x00000000 -> 0xFFFFFFFF
        第2部分是逻辑文件ID，取值范围是0x00000000 -> 0xFFFFFFFF
        第3部分是物理文件ID，取值范围是0x00000000 -> 0x000000FF
        """
        if not file_name:
            return False
        file_name = str(file_name)
        if not re.match(r"^[0-9A-F]{24}$", file_name):
            return False
        first_name = file_name[:8]
        second_name = file_name[8:16]
        third_name = file_name[-8:]
        if (0x00000000 <= int(first_name, 16) <= 0xFFFFFFFF) \
                and (0x00000000 <= int(second_name, 16) <= 0xFFFFFFFF) \
                and (0x00000000 <= int(third_name, 16) <= 0x000000FF):
            return True
        return False

    @staticmethod
    def manual_cleanup_archive_dir(tgt_archive_path, wal_file_name):
        LOGGER.info(f"Try to cleanup archive directory: {tgt_archive_path} manually ...")
        if not KingbaseRestoreService.is_wal_file(wal_file_name):
            LOGGER.warning(f"File name: {wal_file_name} is not a valid WAL.")
            return
        all_files = os.listdir(tgt_archive_path)
        # 将十六进制整数转换为十进制整数
        wal_int_val = int(wal_file_name, 16)
        for f_n in all_files:
            tmp_file_path = os.path.realpath(os.path.join(tgt_archive_path, f_n))
            is_file_need_delete = KingbaseRestoreService.is_wal_file(f_n) or tmp_file_path.endswith('.backup')
            if os.path.isfile(tmp_file_path) and is_file_need_delete and int(f_n[:24], 16) < wal_int_val:
                LOGGER.debug(f"Remove file: {f_n} from archive directory.")
                os.remove(tmp_file_path)
        LOGGER.info("Manually cleanup archive directory success.")

    @staticmethod
    def primary_register(db_sys_user, db_install_path):
        LOGGER.info("Try to start primary register...")
        repmgr_path = os.path.join(db_install_path, "bin", "repmgr")
        check_is_path_exists(repmgr_path)
        if not check_user_utils.check_path_owner(repmgr_path, [db_sys_user]):
            raise Exception("Repmgr and os user is not matching.")

        register_cmd = f"su - {db_sys_user} -c '{repmgr_path} primary register -F'"
        LOGGER.info(f"Start primary register command: {register_cmd}")
        return_code, std_out, std_err = res_base_common.execute_cmd(register_cmd)
        LOGGER.info(f"Execute primary register command, return code: {return_code}, out: {std_out}, err: {std_err}.")
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error("Execute primary register command failed.")
            raise Exception("Execute primary register command failed")
        LOGGER.info("Primary register success.")
        return True

    @staticmethod
    def standby_register(db_sys_user, db_install_path):
        LOGGER.info("Try to start standby register...")
        repmgr_path = os.path.join(db_install_path, "bin", "repmgr")
        check_is_path_exists(repmgr_path)
        if not check_user_utils.check_path_owner(repmgr_path, [db_sys_user]):
            raise Exception("Repmgr and os user is not matching.")

        register_cmd = f"su - {db_sys_user} -c '{repmgr_path} standby register --force'"
        LOGGER.debug(f"Start standby register command: {register_cmd}")
        return_code, std_out, std_err = res_base_common.execute_cmd(register_cmd)
        LOGGER.info(f"Execute standby register command, return code: {return_code}, out: {std_out}, err: {std_err}.")
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error("Execute standby register command failed.")
            raise Exception("Execute standby register command failed")
        LOGGER.info("Standby register success.")
        return True

    @staticmethod
    def set_cluster_recovery_conf_file(cfg_param: RestoreConfigParam):
        """设置恢复命令文件
        """
        LOGGER.info("Start setting recovery conf for kingbase cluster...")
        tgt_data_path = cfg_param.target_data_path

        # 检查es_rep.conf文件是否存在
        kb_conf_file = os.path.join(tgt_data_path, KbConst.CLUSTER_CONF_FILE_NAME)
        if not os.path.isfile(kb_conf_file):
            LOGGER.error("The es_rep.conf file does not exist in the database data directory")
            raise Exception("Kingbase cluster configuration file does not exist in the database data directory")
        LOGGER.info("The es_rep.conf file exists in the database data directory")

        # 设置restore_command
        KingbaseRestoreService.set_restore_command(kb_conf_file, cfg_param.log_copy_path)

        # 非时间点恢复，无需设置recovery_target_time
        recovery_target_time = cfg_param.recovery_target_time
        if not recovery_target_time:
            LOGGER.info("Current restore task is not point-in-time recovery when setting recovery conf")
            return

        KingbaseRestoreService.set_pitr_target_time(kb_conf_file, recovery_target_time)
        KingbaseRestoreService.set_recovery_target_action(kb_conf_file, "promote")

        # 创建recovery.signal文件
        KingbaseRestoreService.create_recovery_signal_file(cfg_param.system_user, tgt_data_path)
        LOGGER.info("Set recovery conf for kingbase success.")

    @staticmethod
    def handle_log_copy(job_dict, job_id, cache_path, db_install_path):
        # 最近全量备份时间点后的日志
        copies = job_dict.get("copies", [])
        log_copy_paths = []
        for tmp_copy in copies[1:]:
            tmp_log_copies = KingbaseRestoreService.get_copy_mount_paths(
                tmp_copy, RepositoryDataTypeEnum.LOG_REPOSITORY.value)
            log_copy_paths.extend(tmp_log_copies)
        log_copy_paths = list(set(log_copy_paths))
        merged_path = os.path.realpath(os.path.join(cache_path, "merged_log_copies"))
        if log_copy_paths:
            KingbaseRestoreService.merge_log_copies(job_id, db_install_path, log_copy_paths, merged_path)
        else:
            LOGGER.error("No log copy mount path exists for point-in-time recovery.")
            raise Exception("No log copy mount path exists for point-in-time recovery.")
        return merged_path

    @staticmethod
    def merge_log_copies(job_id, db_install_path, log_copies, merged_path):
        if os.path.exists(merged_path):
            KingbaseRestoreService.delete_path(merged_path)
        os.makedirs(merged_path)
        # KingbaseES交互式终端ksql路径
        ksql_path = os.path.join(db_install_path, "bin", "ksql")
        # 获取ksql用户ID、组ID
        user_id = os.stat(ksql_path).st_uid
        group_id = os.stat(ksql_path).st_gid
        # 赋权
        os.lchown(merged_path, user_id, group_id)
        check_white_list(log_copies)
        for copy_path in log_copies:
            log_job_id = str(uuid.uuid5(uuid.NAMESPACE_X500, job_id +
                                        DirAndFileNameConst.TABLE_SPACE_INFO_DIR + copy_path))
            KingbaseRestoreService.copy_files(copy_path, merged_path, wildcard=".", job_id=log_job_id)
        KingbaseRestoreService.change_path_mode(merged_path, 0o600, is_data_dir=False)
        KingbaseRestoreService.change_path_owner(merged_path, user_id, group_id)
        LOGGER.info("Merge log copies success.")

    @staticmethod
    def copy_files(src_path: str, target_path: str, wildcard=".", job_id=""):
        LOGGER.info(f"Start copying file: {src_path} to path: {target_path}")
        check_is_path_exists(target_path)
        if os.path.isdir(src_path):
            src_path = f"{src_path}" if src_path.endswith("/") else f"{src_path}/"
        res = backup_files(job_id, [src_path], target_path, write_meta=True)
        if not res:
            LOGGER.error(f"Failed to copy files, jobId: {job_id}.")
            return False
        return KingbaseRestoreService.get_restore_status(job_id)

    @staticmethod
    def get_copy_mount_paths(copy_dict: {}, repo_type):
        copy_mount_paths = []
        for repo in copy_dict.get("repositories", []):
            tmp_repo_type = repo.get("repositoryType")
            if tmp_repo_type != repo_type:
                continue
            if not repo.get("path"):
                LOGGER.error(f"The path value in repository is empty, repository type: {tmp_repo_type}.")
                raise Exception("The path value in repository is empty")
            copy_mount_paths.append(repo.get("path")[0])
        if not copy_mount_paths and not check_utils.check_path_in_white_list(copy_mount_paths[0]):
            LOGGER.error(f"The copy mount path list: {copy_mount_paths} is empty.")
            raise Exception("The copy mount path list is empty")
        LOGGER.info(f"Get copy mount path success, paths: {copy_mount_paths}, repository type: {repo_type}.")
        return copy_mount_paths

    @staticmethod
    def copy_directory(src_path: str, target_path: str, wildcard=".", job_id=""):
        LOGGER.info(f"Start copying dir: {src_path} to path: {target_path}")
        check_is_path_exists(src_path)
        check_is_path_exists(target_path)
        check_white_list([src_path])
        src_path = f"{src_path}" if src_path.endswith("/") else f"{src_path}/"
        LOGGER.info(f"Copying dir: {src_path} to path: {target_path}")
        res = backup(job_id, src_path, target_path)
        if not res:
            LOGGER.error(f"Failed to start backup, jobId: {job_id}.")
            return False
        return KingbaseRestoreService.get_restore_status(job_id)

    @staticmethod
    def get_restore_status(job_id):
        restore_status = False
        while True:
            time.sleep(10)
            status, progress, data_size = query_progress(job_id)
            LOGGER.info(f"Get restore result: status:{status}, progress:{progress}, data_size:{data_size}")
            if status == BackupStatus.COMPLETED:
                LOGGER.info(f"Restore completed, jobId: {job_id}.")
                restore_status = True
                break
            elif status == BackupStatus.RUNNING:
                continue
            elif status == BackupStatus.FAILED:
                LOGGER.error(f"Restore failed, jobId: {job_id}.")
                restore_status = False
                break
            else:
                LOGGER.error(f"Backup failed, status error jobId: {job_id}.")
                restore_status = False
                break
        return restore_status

    @staticmethod
    def set_restore_command(cmd_file, log_path, rollback=False):
        """设置restore_command值
        """
        resource_util.check_path_islink(cmd_file)
        file_owner = get_path_owner(cmd_file)
        LOGGER.info(f"Get file owner:{file_owner}, recovery_cmd_file:{cmd_file}")
        key_status = get_conf_item_status(cmd_file, "restore_command =")
        if not rollback:
            if key_status == ConfigKeyStatus.ANNOTATED:
                set_restore_command = \
                    f"""sed -i "s?^#restore_command =.*?restore_command = 'cp "'"{log_path}/%f"'" %p'?g" {cmd_file}"""
            elif key_status == ConfigKeyStatus.CONFIGURED:
                set_restore_command = \
                    f"""sed -i "s?^restore_command =.*?restore_command = 'cp "'"{log_path}/%f"'" %p'?g"
                {cmd_file}"""
            else:
                set_restore_command = f"\nrestore_command = 'cp {log_path}/%f %p'"
                KingbaseRestoreService.write_conf_file(cmd_file, set_restore_command)
                LOGGER.info(f"Set restore command success when command is not exist, cmd :{set_restore_command}")
                return
        else:
            set_restore_command = f"""sed -i "s?^restore_command =.*?#restore_command = ''?g" {cmd_file}"""
        LOGGER.info(f"Setting restore command ... command: {set_restore_command}")
        checkout_user_and_execute_cmd(file_owner, set_restore_command)
        LOGGER.info("Set restore command success.")

    @staticmethod
    def set_pitr_target_time(cfg_file, recovery_target_time, rollback=False):
        """设置按时间点恢复的recovery_target_time参数
        """
        resource_util.check_path_islink(cfg_file)
        file_owner = get_path_owner(cfg_file)
        LOGGER.info(f"Get file owner:{file_owner}, cfg_file:{cfg_file}")
        key_status = get_conf_item_status(cfg_file, "recovery_target_time =")
        if not rollback:
            if key_status == ConfigKeyStatus.ANNOTATED:
                set_tgt_time_cmd = f"""
                sed -i "s/^#recovery_target_time =.*/recovery_target_time = '{recovery_target_time}'/g" {cfg_file}"""
            elif key_status == ConfigKeyStatus.CONFIGURED:
                set_tgt_time_cmd = f"""
                sed -i "s/^recovery_target_time =.*/recovery_target_time = '{recovery_target_time}'/g" {cfg_file}"""
            else:
                set_tgt_time_cmd = f"\nrecovery_target_time = '{recovery_target_time}'"
                KingbaseRestoreService.write_conf_file(cfg_file, set_tgt_time_cmd)
                LOGGER.info(f"Set recovery target time success when command is not exist.cmd :{set_tgt_time_cmd}")
                return
        else:
            set_tgt_time_cmd = f"""
                        sed -i "s/^recovery_target_time =.*/#recovery_target_time = ''/g" {cfg_file}"""
        LOGGER.info(f"Setting recovery target time ... command: {set_tgt_time_cmd}")
        checkout_user_and_execute_cmd(file_owner, set_tgt_time_cmd.strip())
        LOGGER.info(f"Set recovery target time success.")

    @staticmethod
    def set_recovery_target_action(cfg_file, tgt_action, rollback=False):
        """设置recovery_target_action参数（针对9.5及以上版本）
        """
        resource_util.check_path_islink(cfg_file)
        file_owner = get_path_owner(cfg_file)
        LOGGER.info(f"Get file owner:{file_owner}, cfg_file:{cfg_file}")
        if not rollback:
            key_status = get_conf_item_status(cfg_file, "recovery_target_action =")
            if key_status == ConfigKeyStatus.ANNOTATED:
                set_tgt_action_cmd = f"""
                   sed -i "s/^#recovery_target_action =.*/recovery_target_action = '{tgt_action}'/g" {cfg_file}"""
            elif key_status == ConfigKeyStatus.CONFIGURED:
                set_tgt_action_cmd = f"""
                   sed -i "s/^recovery_target_action =.*/recovery_target_action = '{tgt_action}'/g" {cfg_file}"""
            else:
                set_tgt_action_cmd = f"\nrecovery_target_action = '{tgt_action}'"
                KingbaseRestoreService.write_conf_file(cfg_file, set_tgt_action_cmd)
                LOGGER.info(f"Set recovery target action success when command is not exist.cmd :{set_tgt_action_cmd}")
                return
        else:
            set_tgt_action_cmd = f"""
                        sed -i "s/^recovery_target_action =.*/#recovery_target_action = '{tgt_action}'/g" {cfg_file}"""
        LOGGER.info(f"Setting recovery target action ... command: {set_tgt_action_cmd}")
        checkout_user_and_execute_cmd(file_owner, set_tgt_action_cmd.strip())
        LOGGER.info(f"Set recovery target action success.")

    @staticmethod
    def write_conf_file(cfg_file, set_cmd):
        flags = os.O_WRONLY | os.O_CREAT
        modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
        with os.fdopen(os.open(cfg_file, flags, modes), 'a+') as out_file:
            out_file.write(set_cmd)

    @staticmethod
    def set_single_inst_recovery_conf_file(cfg_param: RestoreConfigParam):
        """设置恢复命令文件
        """
        LOGGER.info("Start setting recovery conf for kingbase single inst...")
        tgt_data_path = cfg_param.target_data_path

        # 检查kingbase.conf文件是否存在
        kb_conf_file = os.path.join(tgt_data_path, KbConst.KINGBASE_CONF_FILE_NAME)
        if not os.path.isfile(kb_conf_file):
            LOGGER.error("The kingbase.conf file does not exist in the database data directory")
            raise Exception("Kingbase configuration file does not exist in the database data directory")
        LOGGER.info("The kingbase.conf file exists in the database data directory")

        # 设置restore_command
        KingbaseRestoreService.set_restore_command(kb_conf_file, cfg_param.log_copy_path)

        # 非时间点恢复，无需设置recovery_target_time
        recovery_target_time = cfg_param.recovery_target_time
        if not recovery_target_time:
            LOGGER.info("Current restore task is not point-in-time recovery when setting recovery conf")
            return

        # 设置恢复时间点
        KingbaseRestoreService.set_pitr_target_time(kb_conf_file, recovery_target_time)
        # 创建recovery.signal文件
        KingbaseRestoreService.create_recovery_signal_file(cfg_param.system_user, tgt_data_path)

        LOGGER.info("Set recovery conf for kingbase success.")

    @staticmethod
    def create_recovery_signal_file(kb_system_user: str, db_data_path: str):
        if not check_user_utils.check_path_owner(db_data_path, [kb_system_user]):
            raise Exception("Data dir and os user is not matching.")
        recovery_signal_file = os.path.join(db_data_path, KbConst.RECOVERY_SIGNAL_FILE_NAME)
        create_signal_file_cmd = f"su - {kb_system_user} -c 'touch {recovery_signal_file}'"
        LOGGER.info(f"Creating recovery signal file ... command: {create_signal_file_cmd}")
        return_code, out, err = res_base_common.execute_cmd(create_signal_file_cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Create recovery signal file failed, return code: {return_code}, "
                         f"out: {out}, err: {err}")
            raise Exception("Create recovery signal file failed")
        LOGGER.info("Create recovery signal file success.")

    @staticmethod
    def start_kingbase_database(kb_system_user: str, db_install_path: str, db_data_path: str):
        LOGGER.info("Try to start database ...")
        kb_ctl_path = os.path.join(db_install_path, "bin", "sys_ctl")
        check_is_path_exists(kb_ctl_path)
        if not check_user_utils.check_path_owner(kb_ctl_path, [kb_system_user]):
            raise ErrCodeException(ErrorCode.EXEC_START_DB_CMD_FAILED, message="Repmgr and os user is not matching.")

        if not check_user_utils.check_path_owner(db_data_path, [kb_system_user]):
            raise ErrCodeException(ErrorCode.EXEC_START_DB_CMD_FAILED, message="Repmgr and os user is not matching.")

        start_db_cmd = f"su - {kb_system_user} -c " \
                       f"'{kb_ctl_path} -t{KbConst.CHECK_POINT_TIME_OUT} -D {db_data_path} -l /dev/null start'"
        LOGGER.debug(f"Start database command: {start_db_cmd}")
        return_code, std_out, std_err = res_base_common.execute_cmd(start_db_cmd)
        LOGGER.info(f"Execute start database command, return code: {return_code}, out: {std_out}, err: {std_err}.")
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error("Execute start database command failed.")
            raise ErrCodeException(ErrorCode.EXEC_START_DB_CMD_FAILED, message="Execute start database command failed.")
        LOGGER.info("Start database success.")

    @staticmethod
    def start_kingbase_monitor(kb_system_user: str, db_install_path: str, db_data_path: str):
        LOGGER.info("Try to start database monitor...")
        kb_monitor_path = os.path.join(db_install_path, "bin", "sys_monitor.sh")
        check_is_path_exists(kb_monitor_path)
        if not check_user_utils.check_path_owner(kb_monitor_path, [kb_system_user]):
            raise ErrCodeException(ErrorCode.EXEC_START_DB_CMD_FAILED, message="Repmgr and os user is not matching.")

        start_monitor_cmd = f"su - {kb_system_user} -c '{kb_monitor_path} start'"
        LOGGER.debug(f"Start database monitor command: {start_monitor_cmd}")
        return_code, std_out, std_err = res_base_common.execute_cmd(start_monitor_cmd)
        LOGGER.info(f"Execute start kb monitor command, return code: {return_code}, out: {std_out}, err: {std_err}.")
        is_running = KingbaseRestoreService.is_db_running(kb_system_user, db_install_path, db_data_path)
        if not is_running:
            LOGGER.error("Start database monitor failed.")
            raise ErrCodeException(ErrorCode.DB_STATUS_ERR_AFTER_RESTORE, message="Start database monitor failed.")
        LOGGER.info("Start database monitor success.")

    @staticmethod
    def standby_clone(kb_system_user: str, db_install_path: str, primary_ip: str):
        LOGGER.info("Try to start standby clone...")
        kb_repmgr_path = os.path.join(db_install_path, "bin", "repmgr")
        check_is_path_exists(kb_repmgr_path)
        if not resource_util.check_os_name(kb_system_user, kb_repmgr_path):
            LOGGER.error("Repmgr path and os username is not matching.")
            raise Exception("Execute standby clone command failed, because os username is incorrect.")
        standby_clone_cmd = f"su - {kb_system_user} -c '{kb_repmgr_path} " \
                            f"standby clone -h {primary_ip} -U esrep -d esrep -F'"
        LOGGER.debug(f"Start standby clone command: {standby_clone_cmd}")
        return_code, std_out, std_err = res_base_common.execute_cmd(standby_clone_cmd)
        LOGGER.info(f"Execute standby clone command, return code: {return_code}, out: {std_out}, err: {std_err}.")
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error("Execute standby clone command failed.")
            raise Exception("Execute standby clone command failed")
        LOGGER.info("Standby clone success.")
        time.sleep(KbConst.WAIT_EIGHTY_SECONDS)
        return True

    @staticmethod
    def delete_path(del_path):
        LOGGER.info(f"Start deleting path: {del_path}")
        if not os.path.exists(del_path):
            LOGGER.warning(f"No need to delete, path: {del_path} not exists.")
            return
        if os.path.isfile(del_path):
            # 文件不存在会报错
            os.remove(del_path)
            LOGGER.info(f"Delete file: {del_path} success.")
        else:
            # 目录不存在会报错
            shutil.rmtree(del_path, ignore_errors=True)
            LOGGER.info(f"Delete directory: {del_path} success.")

    @staticmethod
    def is_db_running(kb_system_user: str, db_install_path: str, db_data_path: str) -> bool:
        LOGGER.info("Start checking database is running ...")
        kb_ctl_path = os.path.join(db_install_path, "bin", "sys_ctl")
        check_is_path_exists(kb_ctl_path)
        if not resource_util.check_os_name(kb_system_user, kb_ctl_path):
            LOGGER.error("Check db runing failed, because os username is incorrect!")
            return False
        if not check_user_utils.check_path_owner(db_data_path, [kb_system_user]):
            LOGGER.error("Data dir and os user is not exist.")
            return False
        check_db_status_cmd = f"su - {kb_system_user} -c '{kb_ctl_path} -D {db_data_path} status'"
        return_code, std_out, std_err = res_base_common.execute_cmd(check_db_status_cmd)
        LOGGER.info(f"Execute check database status command, return code: {return_code}, "
                    f"out: {std_out}, err: {std_err}.")
        if return_code == CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.info("Database is running.")
            return True
        LOGGER.info(f"Database is not running.")
        return False

    @staticmethod
    def get_path_owner(check_dir):
        check_is_path_exists(check_dir)
        return pwd.getpwuid(os.stat(check_dir).st_uid).pw_name

    @staticmethod
    def get_uid_gid_by_os_user(os_user):
        """
        根据系统用户名获取用户ID和用户组ID
        """
        try:
            user_info = pwd.getpwnam(os_user)
        except KeyError as ex:
            LOGGER.error(f"Current system does not have user: {os_user}.")
            raise Exception(f"Current system does not have user: {os_user}") from ex
        return user_info.pw_uid, user_info.pw_gid

    @staticmethod
    def is_dir_readable_and_writable_for_input_user(check_dir, input_user) -> bool:
        if not os.path.isdir(check_dir):
            LOGGER.error(f"Check the directory is readable and writable, "
                         f"input user: {input_user}, dir: {check_dir} is invalid.")
            raise Exception("Check the directory is readable and writable, input an invalid directory")
        try:
            user_info = pwd.getpwnam(input_user)
        except KeyError as ex:
            LOGGER.error(f"Current system does not have user: {input_user}")
            raise Exception(f"Current system does not have user: {input_user}") from ex
        uid = user_info.pw_uid
        gid = user_info.pw_gid
        stat_res = os.stat(check_dir)
        mode = stat_res[stat.ST_MODE]
        return all([
            (stat_res[stat.ST_UID] == uid and (mode & stat.S_IRUSR > 0))
            or (stat_res[stat.ST_GID] == gid and (mode & stat.S_IRGRP > 0))
            or (mode & stat.S_IROTH > 0),
            (stat_res[stat.ST_UID] == uid and (mode & stat.S_IWUSR > 0))
            or (stat_res[stat.ST_GID] == gid and (mode & stat.S_IWGRP > 0))
            or (mode & stat.S_IWOTH > 0)
        ])

    @staticmethod
    def get_local_ips() -> list:
        """获取本机所有IP
        """
        LOGGER.info(f"Start getting all local ips ...")
        local_ips = []
        ip_dict = psutil.net_if_addrs()
        for _, value in ip_dict.items():
            for ips in value:
                if ips[0] == 2 and ips[1] != '127.0.0.1':
                    local_ips.append(ips[1])
        LOGGER.info(f"Get all local ips: {local_ips} success.")
        return local_ips

    @staticmethod
    def convert_timestamp_to_datetime(s_timestamp):
        try:
            int_s_time = int(s_timestamp)
        except ValueError as ex:
            LOGGER.exception(f"Timestamp parameter: {s_timestamp} is invalid when converting.")
            raise Exception("Timestamp parameter is invalid") from ex
        return datetime.datetime.fromtimestamp(int_s_time).strftime(KbConst.RECOVERY_TARGET_TIME_FORMATTER)

    @staticmethod
    def write_progress_info(cache_path, file_name, context: RestoreProgress):
        file_path = os.path.join(cache_path, str(file_name))
        flags = os.O_WRONLY | os.O_CREAT
        modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
        with GLOBAL_PG_PROCESS_FILE_LOCK:
            with os.fdopen(os.open(file_path, flags, modes), 'w') as file:
                file.seek(0)
                file.truncate()
                file.write(context.json())

    @staticmethod
    def write_progress_info_for_ex(task_name, job_dict, err_code=None, err_msg=None):
        cache_path = KingbaseRestoreService.get_cache_mount_path(job_dict)
        process_stage = "QueryRestoreProcess"
        restore_progress = RestoreProgress(progress=NumberConst.HUNDRED)
        if err_code:
            restore_progress.err_code = err_code
        err_msg = err_msg if err_msg else f"Execute {task_name} task failed"
        restore_progress.message = err_msg if "failed" in err_msg else f"failed: {err_msg}"
        KingbaseRestoreService.write_progress_info(cache_path, process_stage, restore_progress)

    @staticmethod
    def read_process_info(cache_path, file_name):
        file_path = os.path.join(cache_path, file_name)
        if not os.path.isfile(file_path):
            return {}
        with GLOBAL_PG_PROCESS_FILE_LOCK:
            with open(file_path, 'r') as file:
                context = file.read()
                return json.loads(context) if context else {}

    @staticmethod
    def get_cache_mount_path(job_dict):
        copies = job_dict.get("copies", [])
        if not copies:
            raise Exception("The copies value in the param file is empty or does not exist")
        if KingbaseRestoreService.is_log_restore(job_dict):
            cache_path = KingbaseRestoreService.get_copy_mount_paths(
                copies[-2], RepositoryDataTypeEnum.CACHE_REPOSITORY.value)[0]
        else:
            cache_path = KingbaseRestoreService.get_copy_mount_paths(
                copies[-1], RepositoryDataTypeEnum.CACHE_REPOSITORY.value)[0]
        check_white_list([cache_path])
        return cache_path

    @staticmethod
    def parse_copies(job_dict):
        copies = job_dict.get("copies", [])
        if not copies:
            raise Exception("The copies value in the param file is empty or does not exist")
        return copies

    @staticmethod
    def parse_os_user(job_dict):
        os_user_name = ""
        if KingbaseRestoreService.is_restore_cluster(job_dict):
            sub_objs = job_dict.get("targetEnv", {}).get("nodes", [{}])
            for obj in sub_objs:
                node_role = obj.get("extendInfo", {}).get("role", "")
                if str(node_role) == str(RoleType.PRIMARY.value):
                    os_user_name = obj.get("extendInfo", {}).get("osUsername", "")
                    break
        else:
            os_user_name = \
                job_dict.get("targetEnv", {}).get("nodes", [{}])[0].get("extendInfo", {}).get("osUsername", "")
        return os_user_name

    @staticmethod
    def get_db_install_and_data_path(job_dict):
        LOGGER.info(f"Start getting db install path and data path ...")
        install_path, data_path = "", ""
        if KingbaseRestoreService.is_restore_cluster(job_dict):
            nodes = job_dict.get("targetEnv", {}).get("nodes", [])
            local_ips = KingbaseRestoreService.get_local_ips()
            for node in nodes:
                tmp_node_ip = resource_util.get_tmp_node_ip(node)
                if tmp_node_ip in local_ips:
                    install_path = os.path.realpath(node.get("extendInfo", {}).get("clientPath", ""))
                    data_path = os.path.realpath(node.get("extendInfo", {}).get("dataDirectory", ""))
                    break
        else:
            tgt_env_extend_info_dict = job_dict.get("targetObject", {}).get("extendInfo", {})
            install_path = os.path.realpath(tgt_env_extend_info_dict.get("clientPath", ""))
            data_path = os.path.realpath(tgt_env_extend_info_dict.get("dataDirectory", ""))
        check_is_path_exists(install_path)
        check_is_path_exists(data_path)
        check_special_character([install_path, data_path])
        check_black_list([install_path, data_path])
        return install_path, data_path

    @staticmethod
    def clear_table_space_dir(os_user, cache_path, src_path):
        table_space_path = os.path.join(src_path, DirAndFileNameConst.TABLE_SPACE_INFO_DIR,
                                        DirAndFileNameConst.TABLE_SPACE_INFO_FILE)
        if not os.path.exists(table_space_path):
            table_space_path = os.path.join(cache_path, DirAndFileNameConst.TABLE_SPACE_INFO_FILE)
        check_white_list([table_space_path])
        if not os.path.exists(table_space_path):
            LOGGER.info("There is no need to restore table space.")
            return ExecuteResultEnum.SUCCESS
        with open(table_space_path, "r") as tb_file:
            tb_info = tb_file.read()
            table_spaces = json.loads(tb_info)
        for _, tb_info in table_spaces.items():
            path = tb_info[0]
            tb_path_user = tb_info[1]
            LOGGER.info(f"Success get tablespace information, tb path user: {tb_path_user}.")
            # 表空间目录存在判断是否属于操作kingbase数据库的操作系统用户，是则清空；不存在则创建表空间目录
            if os.path.exists(path):
                if tb_path_user != os_user:
                    LOGGER.error(f"The tablespace path not belong to the os_user:{os_user}.")
                    return ErrorCode.TB_DIR_IS_NOT_EXIST_OR_PERMISSION_ERROR
            KingbaseRestoreService.clear_data_dir(os_user, path)
            LOGGER.info(f"Success clear or creat tablespace directory.")
        return ExecuteResultEnum.SUCCESS

    @staticmethod
    def restore_table_space(src_path, cache_path, job_id):
        """1.检查文件系统中是否有table_space,没有的话直接return
            2.获取文件系统中table_space的权限等信息
            3.检查本地是否存在table_space路径，存在的话需要校验权限跟源端是否一样
            4.一样的话执行备份table_space
            5.所有的table_space备份完成后，删除clone文件系统中的pgsql_table_space目录
        """
        table_space_path = os.path.realpath(os.path.join(src_path, DirAndFileNameConst.TABLE_SPACE_INFO_DIR,
                                                         DirAndFileNameConst.TABLE_SPACE_INFO_FILE))
        if not os.path.exists(table_space_path):
            LOGGER.info("There is no need to restore table space.")
            return ExecuteResultEnum.SUCCESS
        with open(table_space_path, "r") as tb_file:
            tb_info = tb_file.read()
            table_spaces = json.loads(tb_info)
        # 恢复表空间目录
        if not KingbaseRestoreService.restore_table_spaces_data(job_id, table_spaces, src_path):
            return ErrorCode.TB_DIR_IS_NOT_EXIST_OR_PERMISSION_ERROR
        LOGGER.info(f"Restore all table space success! job_id:{job_id}")

        # 将表空间信息放入cache仓
        if not KingbaseRestoreService.copy_table_space_info_to_cache_repository(table_space_path, cache_path):
            return ErrorCode.RESTORE_TABLE_SPACE_FAILED
        LOGGER.info(f"Copy table space info to cache repository success! job_id:{job_id}")

        # 清空clone文件系统中的表空间信息
        try:
            KingbaseRestoreService.del_table_space_info(src_path)
        except ErrCodeException as ex:
            return ex.error_code

        LOGGER.info(f"Clear table space info in clone fs success! job_id:{job_id}")
        return ExecuteResultEnum.SUCCESS

    @staticmethod
    def restore_table_spaces_data(job_id, table_spaces, src_path):
        for name, tb_info in table_spaces.items():
            path = os.path.realpath(os.path.join(tb_info[0], ".."))
            tb_path = os.path.join(src_path, DirAndFileNameConst.TABLE_SPACE_INFO_DIR, *path.split("/"))
            tb_job_id = str(uuid.uuid5(uuid.NAMESPACE_X500, job_id + DirAndFileNameConst.TABLE_SPACE_INFO_DIR + name))
            result = KingbaseRestoreService.copy_directory(tb_path, path, wildcard=".", job_id=tb_job_id)
            if not result:
                LOGGER.error(f"Restore table space :{name} failed!, job_id:{tb_job_id}")
                return False
        return True

    @staticmethod
    def copy_table_space_info_to_cache_repository(table_space_path, cache_path):
        copy_data_cmd = f'/bin/cp -rf {table_space_path} {cache_path}'
        return_code, std_out, std_err = res_base_common.execute_cmd(copy_data_cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Copy PGSQL_TABLE_SPACE.info to cache repository "
                         f"return code: {return_code}, err: {std_err}.")
            return False
        return True

    @staticmethod
    def del_table_space_info(src_path):
        tb_data_dir = os.path.realpath(os.path.join(src_path, DirAndFileNameConst.TABLE_SPACE_INFO_DIR))
        if os.path.exists(tb_data_dir):
            LOGGER.info(f"Del table space in {src_path} success!")
            try:
                KingbaseRestoreService.delete_path(tb_data_dir)
            except Exception as ex:
                LOGGER.exception("Del table space dir failed!")
                raise ErrCodeException(err_code=ErrorCode.RESTORE_TABLE_SPACE_FAILED,
                                       message="Restore table space failed!") from ex

    @staticmethod
    def change_auth_of_download_data(db_install_path, input_path):
        check_white_list([input_path])
        LOGGER.info(f"Change mode of the download data: {input_path}.")
        KingbaseRestoreService.change_path_mode(input_path, 0o600)
        LOGGER.info(f"Change mode of the download data: {input_path} success.")
        uid, gid = KingbaseRestoreService.get_path_uid_and_gid(os.path.join(db_install_path, "bin", "ksql"))
        LOGGER.info(f"Change owner of the download data: {input_path}, uid: {uid}, gid: {gid}.")

    @staticmethod
    def change_path_mode(input_path, mode, is_data_dir=True):
        LOGGER.info(f"Start changing mode of the path: {input_path}, mode: {mode}.")
        if not os.path.exists(input_path):
            LOGGER.warning(f"The input path: {input_path} does not exist.")
            return
        # 处理文件
        if os.path.isfile(input_path):
            os.chmod(input_path, mode)
            LOGGER.info(f"Change mode of the file: {input_path} success, mode: {mode}.")
            return
        # 处理目录下文件
        for root, _, files in os.walk(input_path):
            for tmp_file in files:
                tmp_file_path = os.path.join(root, tmp_file)
                if not is_data_dir:
                    os.chmod(tmp_file_path, mode)
                    continue
                if oct(os.stat(tmp_file_path).st_mode)[-3:] == '000':
                    os.chmod(tmp_file_path, mode)
        LOGGER.info(f"Change mode of the path: {input_path} success, mode: {mode}.")

    @staticmethod
    def get_path_uid_and_gid(check_path):
        """
        获取路径的用户ID和用户组ID
        """
        check_is_path_exists(check_path)
        return os.stat(check_path).st_uid, os.stat(check_path).st_gid

    @staticmethod
    def change_path_owner(input_path, uid, gid):
        LOGGER.info(f"Start changing owner of the path: {input_path}, uid:{uid}, gid: {gid}.")
        if not os.path.exists(input_path):
            LOGGER.warning(f"The input path: {input_path} does not exist.")
            return
        # 处理文件
        if os.path.isfile(input_path):
            os.lchown(input_path, uid, gid)
            LOGGER.info(f"Change owner of the file: {input_path} success, uid: {uid}, gid: {gid}.")
            return
        # 处理目录
        for root, dirs, files in os.walk(input_path):
            for tmp_dir in dirs:
                if tmp_dir in (".snapshot",):
                    continue
                os.lchown(os.path.join(root, tmp_dir), uid, gid)
            for tmp_file in files:
                os.lchown(os.path.join(root, tmp_file), uid, gid)
        LOGGER.info(f"Change owner of the path: {input_path} success, uid: {uid}, gid: {gid}.")

    @staticmethod
    def record_task_result(pid, err_msg, code=None, err_code=None):
        action_result = ActionResult(code=code if code else ExecuteResultEnum.INTERNAL_ERROR.value, message=err_msg)
        if err_code:
            action_result.body_err = err_code
        res_base_common.output_result_file(pid, action_result.dict(by_alias=True))

    @staticmethod
    def get_db_system_user(job_dict):
        deploy_type = job_dict.get("targetEnv", {}).get("extendInfo", {}).get("deployType")
        db_system_user = ""
        if deploy_type == DeployTypeEnum.SINGLE.value:
            db_system_user = job_dict.get("targetObject", {}).get("extendInfo", {}).get("osUsername", "")
            LOGGER.info(f"Current singleInst's db system user is: {db_system_user}.")
        elif deploy_type == DeployTypeEnum.AP.value:
            ips = KingbaseRestoreService.get_local_ips()
            nodes = job_dict.get("targetEnv", {}).get("nodes", [])
            for node in nodes:
                if resource_util.get_tmp_node_ip(node) in ips:
                    db_system_user = node.get("extendInfo", {}).get("osUsername", "")
                    break
            LOGGER.info(f"Current node's db system user is: {db_system_user}.")
        check_special_character([db_system_user])
        return db_system_user

    @staticmethod
    def backup_conf_file(data_path):
        data_upper_path = os.path.realpath(os.path.join(data_path, ".."))
        for f_name in KbConst.NEED_BAK_CFG_FILES:
            tmp_src_file = os.path.realpath(os.path.join(data_path, f_name))
            if os.path.isfile(tmp_src_file):
                # 将配置文件备份到数据目录的上级目录
                KingbaseRestoreService.preserve_copy_file(tmp_src_file, data_upper_path, tgt_file_name=f"{f_name}.bak")
                LOGGER.info(f"Backup config file: {f_name} successfully before restore.")
            else:
                LOGGER.warning(f"Backup config file: {f_name} on restore but doesn't exist.")

    @staticmethod
    def restore_conf_file(data_path):
        data_upper_path = os.path.realpath(os.path.join(data_path, ".."))
        for f_name in KbConst.NEED_BAK_CFG_FILES:
            bak_f_name = f"{f_name}.bak"
            tmp_src_file = os.path.realpath(os.path.join(data_upper_path, bak_f_name))
            if os.path.isfile(tmp_src_file):
                # 将备份在数据目录上级目录的配置文件恢复到数据目录
                KingbaseRestoreService.preserve_copy_file(tmp_src_file, data_path, tgt_file_name=f_name)
                LOGGER.info(f"Restore config file: {bak_f_name} successfully.")
            else:
                LOGGER.warning(f"The config file: {bak_f_name} that has been backed up does not exist when restoring.")

    @staticmethod
    def delete_useless_bak_files(tgt_data_path):
        for file_name in KbConst.DELETE_FILE_NAMES_OF_BAK_CONF:
            tmp_path = os.path.realpath(os.path.join(tgt_data_path, "..", file_name))
            KingbaseRestoreService.delete_path(tmp_path)

    @staticmethod
    def preserve_copy_file(src_file_path, tgt_dir, tgt_file_name=""):
        check_file_path(src_file_path)
        check_dir_path(tgt_dir)
        tgt_file_path = os.path.realpath(os.path.join(tgt_dir, tgt_file_name))
        copy_file_cmd = f'/bin/cp -fa {src_file_path} {tgt_file_path}'
        LOGGER.info(f"Execute copy file command: {copy_file_cmd}.")
        return_code, std_out, std_err = res_base_common.execute_cmd(copy_file_cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Copy file: {src_file_path} to {tgt_dir} failed, return code: {return_code}, "
                         f"out: {std_out}, err: {std_err}.")
            raise Exception("Copy file failed")
        LOGGER.info(f"Copy file: {src_file_path} to {tgt_dir} success.")

    @staticmethod
    def clear_useless_role_record(kb_system_user: str, db_install_path: str):
        """新位置恢复时，删除原位置无用的主备关系映射
        """
        LOGGER.info("Ready to clear useless_role_record.")
        ksql_path = os.path.join(db_install_path, "bin", "ksql")
        if not check_user_utils.check_path_owner(ksql_path, [kb_system_user]):
            raise ErrCodeException(ErrorCode.AGENT_INTERNAL_ERROR,
                                   message="Ksql path and os user is not matching.")
        exec_cmd = f"su - {kb_system_user} -c '{ksql_path}  -U esrep -d esrep'"
        sql_cmd = "delete from repmgr.nodes;"
        result = KingbaseRestoreService.exec_rep_sql_cmd(exec_cmd, sql_cmd)
        if result != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error("Exec delete role record failed!")
            raise ErrCodeException(ErrorCode.AGENT_INTERNAL_ERROR,
                                   message="Execute delete role record command failed.")
        # 清理等待备节点消息的删除请求（当前恢复状态备节点未启动，无需清理）
        KingbaseRestoreService.terminal_clean_process(kb_system_user)

    @staticmethod
    def terminal_clean_process(kb_system_user):
        pid_list = psutil.pids()
        for pid in pid_list:
            process = psutil.Process(pid)
            try:
                cmd = process.cmdline()
            except Exception as ex:
                LOGGER.error(f"The pid {pid} of process not exist! err:{ex}")
                continue
            key_list = [kb_system_user, "esrep", "DELETE waiting for"]
            if cmd and all(key in cmd[0] for key in key_list):
                try:
                    process.kill()
                except Exception as ex:
                    LOGGER.error(f"Kill process kill error!err:{ex}")
                    break
                LOGGER.info(f"The del record process terminated.")
                break

    @staticmethod
    def exec_rep_sql_cmd(exec_cmd, sql_cmd):
        """step1: 登录esrep数据库
        step2: 执行命令，备节点服务暂时关闭，只需在主节点执行即可
        """
        tmp_code = locale.getdefaultlocale()[1]
        process = pexpect.spawn(exec_cmd, encoding=tmp_code)
        index = process.expect([pexpect.EOF, pexpect.TIMEOUT, "=#", "=>"])
        if index != NumberConst.TWO and index != NumberConst.THREE:
            LOGGER.error("Login rep database error!")
            return CmdRetCode.EXEC_ERROR.value
        process.sendline(sql_cmd)
        index = process.expect([pexpect.EOF, pexpect.TIMEOUT, "The transaction has already committed"])
        if index == NumberConst.ZERO:
            LOGGER.error(f"Exec rep cmd failed!sql_cmd:{sql_cmd}")
            return CmdRetCode.EXEC_ERROR.value
        return CmdRetCode.EXEC_SUCCESS.value

    @staticmethod
    def get_max_connections(conf_file):
        with open(conf_file, 'r') as file:
            lines = file.readlines()
        for line in lines:
            line_content = line.strip()
            if re.match(r"\Amax_connections", line_content):
                max_connections = KingbaseRestoreService.check_max_connections_is_available(line_content)
                LOGGER.debug(f"Get max connections from :{conf_file} is {max_connections}")
                return max_connections
        return NumberConst.ZERO

    @staticmethod
    def check_max_connections_is_available(line_content):
        max_connections = line_content.split("=")[1].strip()
        if "#" in max_connections:
            max_connections = max_connections.split("#")[0].strip()
            return max_connections
        return max_connections

    @staticmethod
    def get_copy_os_username(copies):
        protect_object = copies[-1].get("protectObject", {})
        if protect_object.get("subType") == KbConst.KINGBASE_INSTANCE_TYPE:
            copy_system_user = protect_object.get("extendInfo").get("osUsername")
        else:
            copy_system_user = copies[-1].get("protectSubObjects")[0].get("extendInfo").get("osUsername")
        return copy_system_user

    @staticmethod
    def restore_single_or_primary_node_by_sys_rman(job_dict, pid, cache_path, db_install_path, db_data_path):
        LOGGER.info(f"Restore single or primary node...")
        # 0、副本仓挂载到kbbr_repo/backup/kingbase
        timestamp = job_dict.get("extendInfo", {}).get("restoreTimestamp")
        if KingbaseRestoreService.is_log_restore(job_dict):
            KingbaseRestoreService.log_restore_mount_copies(job_dict, cache_path, pid)
        else:
            KingbaseRestoreService.mount_copies(job_dict, pid)

        # 1、用sys_rman恢复备份副本
        sys_rman_conf_path = os.path.join(job_dict["repo_path"], KbConst.SYSRMAN_CONF_FILE_NAME)
        sys_rman_path = os.path.join(db_install_path, KbConst.BIN_DIR_NAME, KbConst.SYS_RMAN_NAME)
        recovery_timestamp = int(job_dict.get("copies", "")[-1].get("extendInfo", {}).get("backupTime"))
        recovery_time = KingbaseRestoreService.convert_timestamp_to_datetime(recovery_timestamp)
        parallel_process = get_parallel_process(job_dict)
        LOGGER.info(f"Begin to execute the restore_command, pid: {pid}.")
        sql_cmd = f'{sys_rman_path} --config={sys_rman_conf_path} --stanza=kingbase --type=time ' \
                  f'--target=\'{recovery_time}\' --process-max={parallel_process} --delta restore'
        ret, output, err = execute_cmd(sql_cmd)
        if ret != CmdRetCode.EXEC_SUCCESS.value or "errors" in err:
            raise Exception(f"Failed to execute the restore_command, sql_cmd:{sql_cmd}, err:{err}, pid: {pid}.")
        LOGGER.info(f"Success to execute the restore_command, output:{output}, pid: {pid}.")

        # 2、用sys_ctl启动数据库
        db_system_user = KingbaseRestoreService.get_db_system_user(job_dict)
        KingbaseRestoreService.operation_db_by_sys_ctl(db_system_user, db_install_path, db_data_path, "start", pid)

        # 5、select pg_wal_replay_resume();
        KingbaseRestoreService.exec_wal_replay_resume(job_dict.get("targetEnv", {}).get("nodes", {}),
                                                      db_install_path, db_system_user, pid)

        deploy_type = job_dict.get("targetEnv", {}).get("extendInfo", {}).get("deployType")
        if deploy_type == str(DeployType.CLUSTER_TYPE.value):
            # 3、注册primary到repmgr集群中
            time.sleep(KbConst.WAIT_TEN_SECONDS)
            KingbaseRestoreService.clear_useless_role_record(db_system_user, db_install_path)
            # 新位置恢复清理主备关系后，数据库会暂时处于recovery状态，需等待60s再做后续操作
            time.sleep(KbConst.WAIT_SIX_SECONDS)
            KingbaseRestoreService.primary_register(db_system_user, db_install_path)

            # 4、注释主节点kingbase.auto.conf中的recovery_*配置参数，包含restore_command,recovery_target,recovery_target_action等
            KingbaseRestoreService.modify_kingbase_auto_conf_file(job_dict, db_install_path, pid)

        # 6、删除本地repo文件夹，单机直接初始化，集群通过init子任务初始化sys_rman
        shutil.rmtree(job_dict["repo_path"])
        LOGGER.info(f"Delete repo_path in primary node.")
        if deploy_type == str(DeployType.SINGLE_TYPE.value):
            KingbaseRestoreService.init_sys_rman_task(job_dict, db_install_path, db_system_user, pid)

        LOGGER.info("Execute single or primary node restore task success.")
        KingbaseRestoreService.write_progress_info(cache_path, "QueryRestoreProcess",
                                                   RestoreProgress(progress=NumberConst.NINETY,
                                                                   message="standby restore completed"))

    @staticmethod
    def init_sys_rman_task(job_dict, db_install_path, db_system_user, job_id):
        new_copy = job_dict.get("copies", "")[-1].get("extendInfo", {}).get("new_copy")
        if new_copy == KbConst.IS_NEW_COPY:
            sys_backup_sh_path = os.path.join(db_install_path, "bin", "sys_backup.sh")
            sql_cmd = f"su - {db_system_user} -c'{sys_backup_sh_path} init'"
            LOGGER.info(f"Begin to init sys_rman tool, job_id:{job_id}.")
            ret, output, err = execute_cmd(sql_cmd)
            if ret != CmdRetCode.EXEC_SUCCESS.value or "errors" in err:
                raise Exception(f"Failed to init sys_rman, ret:{ret}, output:{output}, err:{err}, job_id: {job_id}.")

    @staticmethod
    def combine_backup_info_file(remote_backup_kingbase_path, pid):
        """
        从最新的副本集合中拿到最新的backup.info文件和backup.info.copy，拷贝到和副本同层
        """
        # 获取当前目录下所有副本
        latest_backup_set = None
        max_timestamp = 0
        for dir_name in os.listdir(remote_backup_kingbase_path):
            dir_path = os.path.join(remote_backup_kingbase_path, dir_name)
            if all([
                not os.path.isfile(dir_path),
                '.snapshot' not in remote_backup_kingbase_path,
                'backup.history' not in remote_backup_kingbase_path,
                os.path.getmtime(dir_path) > max_timestamp]
            ):
                latest_backup_set = dir_path
                max_timestamp = os.path.getmtime(dir_path)
        if not latest_backup_set:
            LOGGER.error(f"Backup set is empty!")
            return False
        LOGGER.info(f"Latest backup set is {latest_backup_set}")

        # 最新副本中backup.info和backup.info.copy拷贝到副本同层
        backup_info_file = os.path.join(latest_backup_set, KbConst.BACKUP_INFO_FILE_NAME)
        backup_info_copy_file = os.path.join(latest_backup_set, KbConst.BACKUP_INFO_COPY_FILE_NAME)
        try:
            shutil.copy2(backup_info_file, remote_backup_kingbase_path)
            shutil.copy2(backup_info_copy_file, remote_backup_kingbase_path)
        except Exception as exception_info:
            LOGGER.error(f"Combine backup.info and backup.info.copy err: {exception_info}, pid: {pid}.")
            return False
        LOGGER.info(f"Combine backup.info and backup.info.copy successfully, pid: {pid}.")
        return True

    @staticmethod
    def mount_copies(job_dict, pid):
        db_system_id = job_dict.get("copies", "")[-1].get("extendInfo", {}).get("db_system_id")
        backup_path = os.path.join(job_dict["repo_path"], KbConst.BACKUP_DIR_NAME)
        archive_path = os.path.join(job_dict["repo_path"], KbConst.ARCHIVE_DIR_NAME)
        os_user_name = KingbaseRestoreService.parse_os_user(job_dict)
        copies = job_dict.get("copies")
        if not copies:
            LOGGER.error(f"The copies value in the param file is empty or does not exist.")
            raise Exception("The copies value in the param file is empty or does not exist")
        copy_mount_path = KingbaseRestoreService.get_copy_mount_paths(
            copies[-1], RepositoryDataTypeEnum.DATA_REPOSITORY.value)[0]
        # backup/archive路径创建软连接
        KingbaseRestoreService.create_soft_links(copy_mount_path, db_system_id, backup_path, archive_path)
        LOGGER.info("Success to create the backup and archive soft link.")
        # 修改data仓system_id_dir，和本地backup、archive目录属主
        KingbaseRestoreService.system_id_dir_change_owner(os_user_name, copy_mount_path, db_system_id, backup_path,
                                                          archive_path)
        KingbaseRestoreService.combine_backup_info_file(
            os.path.join(copy_mount_path, db_system_id, KbConst.BACKUP_DIR_NAME, KbConst.KINGBASE_DIR_NAME), pid)

    @staticmethod
    def log_restore_mount_copies(job_dict, cache_path, pid):
        db_system_id = job_dict.get("copies", "")[0].get("extendInfo", {}).get("db_system_id", "")
        backup_path = os.path.join(job_dict["repo_path"], KbConst.BACKUP_DIR_NAME)
        archive_path = os.path.join(job_dict["repo_path"], KbConst.ARCHIVE_DIR_NAME)
        os_user_name = KingbaseRestoreService.parse_os_user(job_dict)
        copies = job_dict.get("copies", [])
        if not copies:
            LOGGER.error(f"The copies value in the param file is empty or does not exist.")
            raise Exception("The copies value in the param file is empty or does not exist")
        # 复制数据/日志副本至cache仓
        KingbaseRestoreService.copy_data_copy_to_cache_repository(copies, cache_path, db_system_id, pid)
        KingbaseRestoreService.copy_log_copy_to_cache_repository(copies, cache_path, db_system_id, pid)
        # backup/archive路径创建软连接
        KingbaseRestoreService.create_soft_links(cache_path, db_system_id, backup_path, archive_path)
        # 修改cache仓system_id_dir，和本地backup、archive目录属主
        KingbaseRestoreService.system_id_dir_change_owner(os_user_name, cache_path, db_system_id, backup_path,
                                                          archive_path)

    @staticmethod
    def copy_data_copy_to_cache_repository(copies, cache_path, db_system_id, pid):
        data_copy_mount_path = KingbaseRestoreService.get_copy_mount_paths(
            copies[-2], RepositoryDataTypeEnum.DATA_REPOSITORY.value)[0]
        data_copy_dir_path = os.path.join(data_copy_mount_path, db_system_id)
        return_code, out, std_err = execute_cmd(f'cp -r {data_copy_dir_path} {cache_path}')
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to copy data copy dir to cache, return code: {return_code}, "
                         f"out: {out}, err: {std_err}, pid: {pid}.")
            raise Exception("Failed to copy data copy dir to cache")
        LOGGER.info(f"succeeded to copy data copy dir to cache.")

    @staticmethod
    def copy_log_copy_to_cache_repository(copies, cache_path, db_system_id, pid):
        # 复制日志副本到cache仓
        log_copy_mount_paths = []
        for tmp_copy in copies[1:]:
            tmp_log_copies = KingbaseRestoreService.get_copy_mount_paths(
                tmp_copy, RepositoryDataTypeEnum.LOG_REPOSITORY.value)
            log_copy_mount_paths.extend(tmp_log_copies)
        log_copy_mount_paths = list(set(log_copy_mount_paths))
        LOGGER.info(f"log_copy_mount_paths size: {len(log_copy_mount_paths)}.")
        archive_info_file_path = os.path.join(log_copy_mount_paths[0], db_system_id, KbConst.ARCHIVE_DIR_NAME,
                                              KbConst.KINGBASE_DIR_NAME, "archive.info")
        archive_info_copy_file_path = os.path.join(log_copy_mount_paths[0], db_system_id, KbConst.ARCHIVE_DIR_NAME,
                                                   KbConst.KINGBASE_DIR_NAME, "archive.info.copy")
        target_kingbase_path = os.path.join(cache_path, db_system_id, KbConst.ARCHIVE_DIR_NAME,
                                            KbConst.KINGBASE_DIR_NAME)
        return_code, out, std_err = execute_cmd(f'/bin/cp -f {archive_info_file_path} '
                                                f'{archive_info_copy_file_path} {target_kingbase_path}')
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to copy archive.info to cache, return code: {return_code}, "
                         f"out: {out}, err: {std_err}, pid: {pid}.")
            raise Exception("Failed to copy archive.info to cache")
        db_version_id, _ = get_db_version_id_and_system_id(archive_info_file_path)
        target_db_version_id_path = os.path.join(cache_path, db_system_id, KbConst.ARCHIVE_DIR_NAME,
                                                 KbConst.KINGBASE_DIR_NAME, db_version_id)
        if not os.path.exists(target_db_version_id_path):
            os.makedirs(target_db_version_id_path)
        # 合并相同时间线的日志副本
        for log_copy_mount_path in log_copy_mount_paths:
            db_version_id_path = os.path.join(log_copy_mount_path, db_system_id, KbConst.ARCHIVE_DIR_NAME,
                                              KbConst.KINGBASE_DIR_NAME, db_version_id)
            for timeline_dir in os.listdir(db_version_id_path):
                source_timeline_dir_path = os.path.join(db_version_id_path, timeline_dir)
                target_timeline_dir_path = os.path.join(target_db_version_id_path, timeline_dir)
                if timeline_dir in os.listdir(target_db_version_id_path):
                    wal_file_path_list = [os.path.join(source_timeline_dir_path, wal_file)
                                          for wal_file in os.listdir(source_timeline_dir_path)]
                    wal_file_paths = ' '.join(wal_file_path_list)
                    cp_cmd = f'cp -r {wal_file_paths} {target_timeline_dir_path}'
                else:
                    cp_cmd = f'cp -r {source_timeline_dir_path} {target_db_version_id_path}'
                return_code, out, std_err = execute_cmd(cp_cmd)
                if return_code != CmdRetCode.EXEC_SUCCESS.value:
                    LOGGER.error(f"Failed to copy wal files to cache, return code: {return_code}, "
                                 f"out: {out}, err: {std_err}, pid: {pid}.")
                    raise Exception("Failed to copy wal files to cache")
        LOGGER.info(f"succeeded to copy wal files to cache.")

    @staticmethod
    def create_soft_links(target_path, db_system_id, backup_path, archive_path):
        real_backup_path = os.path.join(target_path, db_system_id, KbConst.BACKUP_DIR_NAME)
        real_archive_path = os.path.join(target_path, db_system_id, KbConst.ARCHIVE_DIR_NAME)
        create_soft_link(real_backup_path, backup_path)
        create_soft_link(real_archive_path, archive_path)

    @staticmethod
    def system_id_dir_change_owner(os_user_name, target_path, db_system_id, backup_path, archive_path):
        chown_cmd = (f"chown -R {os_user_name}:{os_user_name} "
                     f"{os.path.join(target_path, db_system_id)} {backup_path} {archive_path}")
        return_code, out, std_err = execute_cmd(chown_cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Failed to change owner, return code: {return_code}, "
                         f"out: {out}, err: {std_err}.")
            raise Exception("Failed to change owner")

    @staticmethod
    def is_log_restore(job_dict):
        timestamp = job_dict.get("extendInfo", {}).get("restoreTimestamp")
        return timestamp is not None

    @staticmethod
    def modify_kingbase_auto_conf_file(job_dict, db_install_path, job_id):
        # 指定需要注释的配置参数
        params_to_comment = ['restore_command', 'recovery_target', 'recovery_target_action']
        kingbase_auto_conf_path = os.path.join(db_install_path, KbConst.DATA_DIR_NAME, KbConst.KINGBASE_AUTO_CONF_FILE)
        try:
            check_is_path_exists(kingbase_auto_conf_path)
        except Exception:
            LOGGER.info(f"No kingbase.auto.conf need to comment:{params_to_comment}")
            return

        # 读取 kingbase.auto.conf 文件
        with open(kingbase_auto_conf_path, 'r') as file:
            lines = file.readlines()
        # 遍历文件的每一行，注释指定的配置参数
        for index, line in enumerate(lines):
            for param in params_to_comment:
                if param in line and not line.startswith('#'):
                    lines[index] = '#' + line

        # 将注释后的内容写回 kingbase.auto.conf 文件
        flags = os.O_WRONLY | os.O_CREAT | os.O_TRUNC
        mode = stat.S_IWUSR | stat.S_IRUSR
        with os.fdopen(os.open(kingbase_auto_conf_path, flags, mode), 'w') as file:
            file.writelines(lines)
        LOGGER.info(f"Succeed to modify kingbase.auto.conf, job id: {job_id}.")

    @staticmethod
    def exec_wal_replay_resume(nodes, db_install_path, db_system_user, pid=""):
        repo_ip = get_sys_rman_configuration_item(db_install_path, pid, "_repo_ip")
        db_port = get_sys_rman_configuration_item(db_install_path, pid, "_single_db_port")
        ksql_path = os.path.join(db_install_path, KbConst.BIN_DIR_NAME, "ksql")
        # 获取当前节点id
        node_id = get_host_sn()
        node_index = 0
        for index, node in enumerate(nodes):
            if str(node.get("id", {})) == str(node_id):
                node_index = index

        prefix_key = f"job_targetEnv_nodes_{node_index}_auth_authKey"
        prefix_pwd = f"job_targetEnv_nodes_{node_index}_auth_authPwd"
        db_user_name = get_env_variable(f'{prefix_key}_{pid}')
        check_special_character([db_user_name, ksql_path])

        # 新位置恢复清理主备关系后，数据库会暂时处于recovery状态，需等待60s再做后续操作
        time.sleep(KbConst.WAIT_SIX_SECONDS)
        # 查看当前数据库状态：读写off，只读on
        cmd = f"su - {db_system_user} -c '{ksql_path} -U {db_user_name} " \
              f"-h {repo_ip} -p {db_port} -d test -W -H -c \"SELECT pg_is_in_recovery();\"'"
        ret_code, current_setting = execute_cmd_and_parse_res(pid, cmd, prefix_pwd)
        if ret_code != ErrorCode.SUCCESS:
            LOGGER.error(
                f"Failed to exec cmd: SELECT pg_is_in_recovery(), return code: {ret_code}, pid: {pid}.")
            raise Exception("Failed to exec cmd: SELECT current_setting('transaction_read_only').")
        LOGGER.info(f"Current setting of transaction_read_only is {current_setting}!")

        if not current_setting or current_setting != "f":
            # on状态执行命令开启读写模式
            cmd = f"su - {db_system_user} -c '{ksql_path} -U {db_user_name} " \
                  f"-h {repo_ip} -p {db_port} -d test -W -H -c \"select pg_wal_replay_resume();\"'"
            ret_code, output = execute_cmd_and_parse_res(pid, cmd, prefix_pwd)
            if ret_code != ErrorCode.SUCCESS:
                LOGGER.error(f"Failed to exec pg_wal_replay_resume cmd: {cmd}, return code: {ret_code}, pid: {pid}.")
                raise Exception("Failed to exec pg_wal_replay_resume cmd.")
            else:
                LOGGER.info(f"Succeed to exec pg_wal_replay_resume cmd, job id: {pid}.")

    @staticmethod
    def restore_standby_node_by_sys_rman(job_dict, cache_path, db_install_path, db_data_path, pid=""):
        LOGGER.info(f"Restore standby node...")
        # 1、备节点执行repmgr standby clone克隆新的备库
        db_system_user = KingbaseRestoreService.get_db_system_user(job_dict)
        primary_ip = KingbaseRestoreService.get_primary_node_ip(job_dict)
        KingbaseRestoreService.standby_clone(db_system_user, db_install_path, primary_ip)

        # 2、用sys_ctl启动数据库
        KingbaseRestoreService.operation_db_by_sys_ctl(db_system_user, db_install_path, db_data_path, "start", pid)
        # 3、注册standby到repmgr集群中
        KingbaseRestoreService.standby_register(db_system_user, db_install_path)

        # 4、删除本地repo文件夹，为重新初始化sys_rman做准备
        shutil.rmtree(job_dict["repo_path"])
        LOGGER.info(f"Delete repo_path in standby node.")

        LOGGER.info("Execute standby restore task success.")
        KingbaseRestoreService.write_progress_info(cache_path, "QueryRestoreProcess",
                                                   RestoreProgress(progress=NumberConst.NINETY,
                                                                   message="standby restore completed"))

    @staticmethod
    def operation_db_by_sys_ctl(db_system_user, db_install_path, db_data_path, operation, job_id=""):
        sys_ctl_path = os.path.join(db_install_path, KbConst.BIN_DIR_NAME, KbConst.SYS_CTL)
        sql_cmd = f"su - {db_system_user} -c '{sys_ctl_path} -D {db_data_path} -l /dev/null {operation}'"
        ret, output, err = execute_cmd(sql_cmd)
        if ret != CmdRetCode.EXEC_SUCCESS.value or "errors" in err:
            raise Exception("Failed to execute the sys_ctl command.")
        LOGGER.info(f"Succeed to execute the sys_ctl command, job id: {job_id}.")
