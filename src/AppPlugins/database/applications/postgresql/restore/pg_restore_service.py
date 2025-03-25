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

import functools
import json
import os
import shutil
import time
import uuid

import pexpect

from common import cleaner
from common import common as restore_svc_common
from common.common import exter_attack, check_del_dir, execute_cmd_list, execute_cmd, clean_dir_not_walk_link
from common.common_models import SubJobDetails, ActionResult, SubJobModel, LogDetail
from common.const import RepositoryDataTypeEnum, SubJobStatusEnum, ExecuteResultEnum, RoleType, SubJobTypeEnum, \
    SubJobPolicyEnum, ReportDBLabel, DBLogLevel, CMDResult
from common.enums.common_enums import DeployTypeEnum
from common.file_common import change_path_permission
from common.logger import Logger
from common.number_const import NumberConst
from common.util import check_user_utils
from common.util import check_utils
from common.util.checkout_user_utils import get_path_owner
from common.util.cmd_utils import cmd_format, get_livemount_path
from common.util.exec_utils import exec_cp_cmd, exec_mkdir_cmd, exec_mv_cmd, read_lines_cmd, exec_overwrite_file
from postgresql.common.const import PgConst, CmdRetCode, RestoreAction, RestoreSubJob, \
    ConfigKeyStatus, DirAndFileNameConst, InstallDeployType
from postgresql.common.error_code import ErrorCode
from postgresql.common.models import RestoreConfigParam, RestoreProgress
from postgresql.common.pg_exception import ErrCodeException
from postgresql.common.util.get_sensitive_utils import get_env_variable
from postgresql.common.util.pg_common_utils import PostgreCommonUtils
from postgresql.resource.cluster_node_checker import ClusterNodesChecker

LOGGER = Logger().get_logger(filename="postgresql.log")
RESTORE_TASK_PROGRESS_FILE_MAP = {
    "exec_pre_task": RestoreAction.QUERY_RESTORE_PRE,
    "exec_pre_subtask": RestoreAction.QUERY_RESTORE,
    "exec_restore_subtask": RestoreAction.QUERY_RESTORE,
    "exec_post_subtask": RestoreAction.QUERY_RESTORE,
    "exec_post_task": RestoreAction.QUERY_RESTORE_POST
}
RESTORE_REPORT_LABEL_MAP = {
    RestoreAction.QUERY_RESTORE_PRE: ReportDBLabel.PRE_REQUISIT_FAILED,
    RestoreAction.QUERY_RESTORE: ReportDBLabel.RESTORE_SUB_FAILED,
    RestoreAction.QUERY_RESTORE_POST: ReportDBLabel.RESTORE_SUB_FAILED
}
PRE_PROGRESS_MAP = {
    "one": {
        "False": {
            "progress_file": RestoreAction.QUERY_RESTORE_PRE,
            "fail": {"progress": NumberConst.TEN, "message": "check database not running failed"},
            "success": {"progress": NumberConst.TWENTY, "message": "check database not running success"},
        },
        "True": {
            "progress_file": RestoreAction.QUERY_RESTORE,
            "fail": {"progress": NumberConst.ONE, "message": "check database not running failed"},
            "success": {"progress": NumberConst.TWO, "message": "check database not running success"},
        }
    },
    "one_two": {
        "False": {
            "progress_file": RestoreAction.QUERY_RESTORE_PRE,
            "fail": {"progress": NumberConst.TEN, "message": "check database port not listening failed"},
            "success": {"progress": NumberConst.TWENTY, "message": "check database port not listening success"},
        },
        "True": {
            "progress_file": RestoreAction.QUERY_RESTORE,
            "fail": {"progress": NumberConst.ONE, "message": "check database port not listening failed"},
            "success": {"progress": NumberConst.TWO, "message": "check database port not listening success"},
        }
    },
    "one_three": {
        "True": {
            "progress_file": RestoreAction.QUERY_RESTORE,
            "fail": {"progress": NumberConst.ONE, "message": "check pgpool port not listening failed"},
            "success": {"progress": NumberConst.TWO, "message": "check pgpool port not listening success"},
        }
    },
    "one_four": {
        "True": {
            "progress_file": RestoreAction.QUERY_RESTORE,
            "fail": {"progress": NumberConst.ONE, "message": "check patroni pitr yml config failed"},
            "success": {"progress": NumberConst.TWO, "message": "check patroni pitr yml config success"},
        }
    },
    "two": {
        "False": {
            "progress_file": RestoreAction.QUERY_RESTORE_PRE,
            "fail": {"progress": NumberConst.THIRTY, "message": "check target data path space failed"},
            "success": {"progress": NumberConst.FORTY, "message": "check target data path space success"},
        },
        "True": {
            "progress_file": RestoreAction.QUERY_RESTORE,
            "fail": {"progress": NumberConst.THREE, "message": "check target data path space failed"},
            "success": {"progress": NumberConst.FOUR, "message": "check target data path space success"},
        }
    },
    "three": {
        "False": {
            "progress_file": RestoreAction.QUERY_RESTORE_PRE,
            "fail": {"progress": NumberConst.FIFTY, "message": "check version is matched failed"},
            "success": {"progress": NumberConst.SIXTY, "message": "check version is matched success"},
        },
        "True": {
            "progress_file": RestoreAction.QUERY_RESTORE,
            "fail": {"progress": NumberConst.FIVE, "message": "check version is matched failed"},
            "success": {"progress": NumberConst.SIX, "message": "check version is matched success"},
        }
    },
    "four": {
        "False": {
            "progress_file": RestoreAction.QUERY_RESTORE_PRE,
            "fail": {"progress": NumberConst.SEVENTY, "message": "check os user is same failed"},
            "success": {"progress": NumberConst.EIGHTY, "message": "check os user is same success"},
        },
        "True": {
            "progress_file": RestoreAction.QUERY_RESTORE,
            "fail": {"progress": NumberConst.SEVEN, "message": "check os user is same failed"},
            "success": {"progress": NumberConst.EIGHT, "message": "check os user is same success"},
        }
    },
    "five": {
        "False": {
            "progress_file": RestoreAction.QUERY_RESTORE_PRE,
            "fail": {"progress": NumberConst.NINETY, "message": "check target data path is rw failed"},
            "success": {"progress": NumberConst.HUNDRED, "message": "check target data path is rw success"},
        },
        "True": {
            "progress_file": RestoreAction.QUERY_RESTORE,
            "fail": {"progress": NumberConst.NINE, "message": "check target data path is rw failed"},
            "success": {"progress": NumberConst.TEN, "message": "check target data path is rw success"},
        }
    }
}


def not_handle_exception_decorator(func):
    @functools.wraps(func)
    def inner(*args, **kwargs):
        try:
            func(*args, **kwargs)
        except Exception:
            LOGGER.exception(f"Execute func: {func} exception.")

    return inner


def build_sub_job(job_id, job_name, priority, node_id):
    return SubJobModel(jobId=job_id, subJobId="", jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value, jobName=job_name,
                       jobPriority=priority, policy=SubJobPolicyEnum.FIXED_NODE.value, ignoreFailed=False,
                       execNodeId=node_id, jobInfo="") \
        .dict(by_alias=True)


class PostgreRestoreService:
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
        for obj in ordered_sub_jobs:
            tmp_node_id = obj.get("extendInfo", {}).get("hostId")
            sub_jobs.append(build_sub_job(job_id, RestoreSubJob.PREPARE, tmp_priority, tmp_node_id))
        # restore 先主，再依次从
        tmp_priority += 1
        for idx, obj in enumerate(ordered_sub_jobs):
            tmp_node_id = obj.get("extendInfo", {}).get("hostId")
            if idx == 0:
                sub_jobs.append(build_sub_job(job_id, RestoreSubJob.RESTORE, tmp_priority, tmp_node_id))
                continue
            tmp_priority += 1
            sub_jobs.append(build_sub_job(job_id, RestoreSubJob.RESTORE, tmp_priority, tmp_node_id))
        # post 先主，再同时从
        tmp_priority += 1
        for idx, obj in enumerate(ordered_sub_jobs):
            tmp_node_id = obj.get("extendInfo", {}).get("hostId")
            if idx == 0:
                sub_jobs.append(build_sub_job(job_id, RestoreSubJob.POST, tmp_priority, tmp_node_id))
                tmp_priority += 1
                continue
            sub_jobs.append(build_sub_job(job_id, RestoreSubJob.POST, tmp_priority, tmp_node_id))
        LOGGER.info(f"Execute generate sub job task of full copy success. Sub Jobs: {sub_jobs}.")
        restore_svc_common.output_result_file(pid, sub_jobs)

    @staticmethod
    def clear_data_dir(os_user, data_path):
        """
        恢复前处理数据目录，存在则清空；不存在则创建
        """
        # 新增黑名单校验
        PostgreCommonUtils.check_black_list(data_path)
        if not os.path.exists(data_path):
            LOGGER.warning(f"Try to create database data directory: {data_path} when it does not exist ...")
            # data目录权限必须为700
            exec_mkdir_cmd(data_path, mode=0o700, is_check_white_list=False)
            uid, gid = PostgreCommonUtils.get_uid_gid_by_os_user(os_user)
            LOGGER.info(f"Change owner of the data path: {data_path}, uid: {uid}, gid: {gid}.")
            os.lchown(data_path, uid, gid)
            LOGGER.info(f"Change owner of the data path: {data_path} success.")
        else:
            PostgreCommonUtils.clear_dir_when_exist(data_path)

    @staticmethod
    def standby_node_restore(pid, param_dict):
        cache_path = PostgreRestoreService.get_cache_mount_path(param_dict)
        # 1.清空data目录
        tgt_install_path, tgt_data_path, _ = PostgreRestoreService.get_db_install_and_data_path(param_dict)
        PostgreRestoreService.cleanup_archive_after_restore(param_dict)
        PostgreRestoreService.clear_data_dir(PostgreRestoreService.parse_os_user(param_dict), tgt_data_path)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=NumberConst.FIFTEEN,
                                                               message="clear data dir success"))
        # 1.1 清空tablespace目录
        PostgreRestoreService.clear_table_space_dir(cache_path=cache_path)

        install_deploy_type = param_dict.get("job", {}).get("targetEnv", {}).get("extendInfo", {}).get(
            "installDeployType", InstallDeployType.PGPOOL)
        if install_deploy_type != InstallDeployType.PATRONI:
            # 2.从主节点同步数据
            PostgreRestoreService.sync_master_data(pid, param_dict)
            PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                                   RestoreProgress(progress=NumberConst.SEVENTY,
                                                                   message="sync master data success"))

        # 3.删除目标实例无用的文件
        PostgreRestoreService.delete_useless_files_of_data_dir(tgt_data_path)
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=NumberConst.SEVENTY_ONE,
                                                               message="delete useless files success"))
        if install_deploy_type != InstallDeployType.PATRONI:
            # 4.配置恢复命令
            job_dict = param_dict.get("job", {})
            tgt_obj_extend_info_dict = job_dict.get("targetObject", {}).get("extendInfo", {})
            tgt_db_os_user = tgt_obj_extend_info_dict.get("osUsername", "")
            primary_ip, primary_inst_port = PostgreRestoreService.get_primary_ip_and_instance_port(param_dict)
            repl_user = job_dict.get("targetObject", {}).get("auth", {}).get("extendInfo", {}).get("dbStreamRepUser")
            repl_pwd = get_env_variable(f"job_targetObject_auth_extendInfo_dbStreamRepPwd_{pid}")
            try:
                cfg_extend_info = {
                    "primary_ip": primary_ip,
                    "primary_inst_port": primary_inst_port,
                    "db_stream_rep_user": repl_user,
                    "db_stream_rep_pwd": repl_pwd
                }
                tgt_obj_extend_info_dict = job_dict.get("targetObject", {}).get("extendInfo", {})
                tgt_version = tgt_obj_extend_info_dict.get("version", "")
                cfg_param = RestoreConfigParam(
                    system_user=tgt_db_os_user, target_version=tgt_version, target_install_path=tgt_install_path,
                    target_data_path=tgt_data_path, extend_info=cfg_extend_info
                )
                PostgreRestoreService.set_recovery_conf_file(param_dict, cfg_param, role=RoleType.STANDBY.value)
            finally:
                cleaner.clear(repl_pwd)
            PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                                   RestoreProgress(progress=NumberConst.SEVENTY_FIVE,
                                                                   message="set recovery command success"))
        PostgreRestoreService.start_db_after_restore(pid, param_dict, cache_path, node_role=RoleType.STANDBY.value)

    @staticmethod
    def prepare_data_dir_for_patroni(data_dir, user):
        copy_dir = os.path.join(os.path.dirname(data_dir), PgConst.OCEAN_PROTECT_DATA_COPY)
        LOGGER.info(f"Prepare for Patroni startup. Move file from {data_dir} to {copy_dir}.")
        ret = exec_mkdir_cmd(copy_dir, user, 0o700, is_check_white_list=False)
        if not ret:
            LOGGER.error(f"Create copy data path failed")
            raise Exception("Create copy data path failed.")
        LOGGER.info("Create copy data path success.")
        ret = exec_mv_cmd(data_dir, copy_dir, user_name=user, check_white_black_list_flag=False)
        if not ret:
            LOGGER.error(f"Move prepared data directory failed!")
            raise Exception("Move prepared data directory failed!")
        LOGGER.info(f"Move prepared data directory success.")
        LOGGER.info(f"Try to create empty data path: {data_dir}.")
        ret = exec_mkdir_cmd(data_dir, user, 0o700, is_check_white_list=False)
        if not ret:
            LOGGER.error(f"Create empty data path failed")
            raise Exception("Create empty data path failed.")
        LOGGER.info("Create empty data path success.")

    @staticmethod
    def delete_copy_dir_for_patroni(param_dict):
        _, data_dir, _ = PostgreRestoreService.get_db_install_and_data_path(param_dict)
        copy_dir = os.path.join(os.path.dirname(data_dir), PgConst.OCEAN_PROTECT_DATA_COPY)
        if os.path.exists(copy_dir):
            LOGGER.info("Start to delete copy dir for patroni.")
            PostgreCommonUtils.delete_path(copy_dir)

    @staticmethod
    def start_db_after_restore(pid, param_dict, cache_path, node_role=RoleType.NONE_TYPE.value):
        tgt_install_path, tgt_data_path, _ = PostgreRestoreService.get_db_install_and_data_path(param_dict)
        # 启动数据库实例
        tgt_obj_extend_info_dict = param_dict.get("job", {}).get("targetObject", {}).get("extendInfo", {})
        tgt_db_os_user = tgt_obj_extend_info_dict.get("osUsername", "")
        install_deploy_type = param_dict.get("job", {}).get("targetEnv", {}).get("extendInfo", {}).get(
            "installDeployType", InstallDeployType.PGPOOL)
        res_timestamp = param_dict.get("job", {}).get("extendInfo", {}).get("restoreTimestamp")
        if install_deploy_type == InstallDeployType.PATRONI:
            # 日志恢复时，生成Patroni bootstrap拷贝用的数据目录，原有数据目录置空
            if res_timestamp and (str(node_role) != str(RoleType.STANDBY.value)):
                PostgreRestoreService.prepare_data_dir_for_patroni(tgt_data_path, tgt_db_os_user)
            host_ips = PostgreCommonUtils.get_local_ips()
            nodes = param_dict.get("job", {}).get("restoreSubObjects", [{}])
            patroni_config, _, role = PostgreCommonUtils.get_patroni_config(host_ips, nodes)
            ret, std_out, std_err = execute_cmd('systemctl start patroni')
            if ret != CMDResult.SUCCESS:
                LOGGER.error("Execute start patroni command failed.")
                raise ErrCodeException(ErrorCode.EXEC_START_PATRONI_CMD_FAILED,
                                       message="Execute start patroni command failed")
            # 等待当前节点启动完成
            PostgreRestoreService.wait_start_over(tgt_data_path, tgt_db_os_user, host_ips, patroni_config)
        else:
            PostgreCommonUtils.start_postgresql_database(tgt_db_os_user, tgt_install_path, tgt_data_path, param_dict)
        LOGGER.info(f"Start patroni finally success.")
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=NumberConst.EIGHTY,
                                                               message="start database success"))

        # 检查数据库实例是否启动成功
        config_file = param_dict.get("job", {}).get("targetObject", {}).get("extendInfo", {}).get("configFile", "")
        is_running = PostgreRestoreService.check_is_running(install_deploy_type, tgt_data_path, tgt_db_os_user,
                                                            tgt_install_path, config_file)
        if not is_running:
            PostgreCommonUtils.write_progress_info(
                cache_path, RestoreAction.QUERY_RESTORE,
                RestoreProgress(progress=NumberConst.EIGHTY_FOUR,
                                message="check database running after restore failed",
                                err_code=ErrorCode.DB_STATUS_ERR_AFTER_RESTORE))
            PostgreRestoreService.record_task_result(pid, "The target database instance is not running after restore.",
                                                     err_code=ErrorCode.DB_STATUS_ERR_AFTER_RESTORE)
            return
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=NumberConst.EIGHTY_FIVE,
                                                               message="check database is running"))
        # 集群中存在主备切换状况，当备节点的归档目录下存在WAL日志时，将会导致恢复失败，主备节点都应执行归档目录清理操作
        PostgreRestoreService.cleanup_archive_after_restore(param_dict)
        tgt_version = tgt_obj_extend_info_dict.get("version", "")
        PostgreRestoreService.reset_recovery_config(param_dict.get("job", {}), tgt_version,
                                                    tgt_data_path, bool(res_timestamp), role=node_role)
        # 恢复后清理data目录无用文件
        PostgreRestoreService.delete_useless_files_of_data_dir(tgt_data_path, before_restore=False)

        LOGGER.info("Execute point-in-time recovery subtask success.")
        PostgreCommonUtils.write_progress_info(cache_path, RestoreAction.QUERY_RESTORE,
                                               RestoreProgress(progress=NumberConst.NINETY, message="completed"))
        PostgreRestoreService.record_task_result(pid, "Executing restore subtask success",
                                                 code=ExecuteResultEnum.SUCCESS.value)

    @staticmethod
    def check_is_running(install_deploy_type, tgt_data_path, tgt_db_os_user, tgt_install_path, config_file):
        if install_deploy_type == InstallDeployType.PGPOOL:
            is_running = PostgreCommonUtils.is_db_running(tgt_db_os_user, tgt_install_path, tgt_data_path,
                                                          config_file=config_file)
        else:
            for _ in range(12):
                is_running = PostgreCommonUtils.is_db_running(tgt_db_os_user, tgt_install_path, tgt_data_path)
                if is_running:
                    break
                time.sleep(10)
        return is_running

    @staticmethod
    def wait_start_over(tgt_data_path, user, host_ips, patroni_config):
        LOGGER.info(f"Begin to start patroni success.")
        time.sleep(120)
        while True:
            data_failed_name = f"{os.path.basename(tgt_data_path)}.failed"
            data_failed_path = os.path.join(os.path.dirname(tgt_data_path), data_failed_name)
            if os.path.exists(data_failed_path):
                LOGGER.error("Start patroni failed, data file is broken. Check patroni log for details.")
                ret = exec_mv_cmd(data_failed_path, tgt_data_path, user_name=user, check_white_black_list_flag=False)
                if not ret:
                    LOGGER.error(f"Rename failed data directory failed!")
                    raise Exception("Rename failed data directory failed!")
                LOGGER.info(f"Rename failed data directory success.")
                raise ErrCodeException(ErrorCode.DB_STATUS_ERR_AFTER_RESTORE,
                                       message="Start patroni failed. Please check patroni log for details.")
            cluster_nodes = ClusterNodesChecker.get_nodes(patroni_config)
            if not cluster_nodes:
                time.sleep(30)
                continue
            for cluster_node in cluster_nodes:
                hostname = cluster_node.get('hostname', "")
                status = cluster_node.get('status', "")
                if hostname in host_ips:
                    break
            if status in ['running', 'streaming']:
                break
            time.sleep(30)

    @staticmethod
    def pre_check_db_not_running(pid, param_dict, cache_path, is_cluster=False):
        progress_dict = PRE_PROGRESS_MAP.get("one", {}).get(str(is_cluster), {})
        db_os_user = param_dict.get("job", {}).get("targetObject", {}).get("extendInfo", {}).get("osUsername", "")
        db_install_path, db_data_path, _ = PostgreRestoreService.get_db_install_and_data_path(param_dict)
        config_file = param_dict.get("job", {}).get("targetObject", {}).get("extendInfo", {}).get("configFile", "")
        is_running = PostgreCommonUtils.is_db_running(db_os_user, db_install_path, db_data_path,
                                                      config_file=config_file)
        # 数据库正在运行，提示用户先停止数据库
        if is_running:
            PostgreCommonUtils.write_progress_info(cache_path, progress_dict.get("progress_file"), RestoreProgress(
                progress=progress_dict.get("fail", {}).get("progress"),
                message=progress_dict.get("fail", {}).get("message"),
                err_code=ErrorCode.DB_RUNNING_ERR_BEFORE_RESTORE))
            PostgreRestoreService.record_task_result(pid, "The target instance is still running when restoring.",
                                                     err_code=ErrorCode.DB_RUNNING_ERR_BEFORE_RESTORE)
            return False
        PostgreCommonUtils.write_progress_info(cache_path, progress_dict.get("progress_file"), RestoreProgress(
            progress=progress_dict.get("success", {}).get("progress"),
            message=progress_dict.get("success", {}).get("message")))
        return True

    @staticmethod
    def check_db_port_not_listen(pid, param_dict, cache_path, is_cluster=False):
        progress_dict = PRE_PROGRESS_MAP.get("one_two", {}).get(str(is_cluster), {})
        tgt_inst_port = PostgreRestoreService.get_db_port(param_dict)
        is_listened = PostgreCommonUtils.check_port_is_listen(tgt_inst_port)
        # 数据库端口被监听，提示用户先停止占用端口的进程
        if is_listened:
            PostgreCommonUtils.write_progress_info(cache_path, progress_dict.get("progress_file"), RestoreProgress(
                progress=progress_dict.get("fail", {}).get("progress"),
                message=progress_dict.get("fail", {}).get("message"),
                err_code=ErrorCode.DB_PORT_LISTEN_ERR_BEFORE_RESTORE))
            PostgreRestoreService.record_task_result(
                pid, "The target instance port is still listening when restoring.",
                err_code=ErrorCode.DB_PORT_LISTEN_ERR_BEFORE_RESTORE)
            return False
        PostgreCommonUtils.write_progress_info(cache_path, progress_dict.get("progress_file"), RestoreProgress(
            progress=progress_dict.get("success", {}).get("progress"),
            message=progress_dict.get("success", {}).get("message")))
        return True

    @staticmethod
    def check_pgpool_port_not_listen(pid, param_dict, cache_path):
        progress_dict = PRE_PROGRESS_MAP.get("one_three", {}).get("True", {})
        tgt_pgpool_port = PostgreRestoreService.get_pgpool_port(param_dict)
        pgpool_install_path = PostgreRestoreService.get_pgpool_install_path(param_dict)
        wd_port = PostgreRestoreService.get_pgpool_wd_port(pgpool_install_path)
        LOGGER.info(f"wd_port is: {wd_port}")
        is_listened = PostgreCommonUtils.check_port_is_listen(tgt_pgpool_port)
        is_wd_port_listened = PostgreCommonUtils.check_port_is_listen(wd_port)
        if is_listened or is_wd_port_listened:
            PostgreCommonUtils.write_progress_info(cache_path, progress_dict.get("progress_file"), RestoreProgress(
                progress=progress_dict.get("fail", {}).get("progress"),
                message=progress_dict.get("fail", {}).get("message"),
                err_code=ErrorCode.PGPOOL_PORT_LISTEN_ERR_BEFORE_RESTORE))
            PostgreRestoreService.record_task_result(pid, "The target pgpool port is still listening when restoring.",
                                                     err_code=ErrorCode.PGPOOL_PORT_LISTEN_ERR_BEFORE_RESTORE)
            return False
        # 删除Pgpool端口临时文件（如：/tmp/.s.PGSQL.9999，/tmp/.s.PGSQL.9898）, /tmp/.s.PGPOOLWD_CMD.9000，如果存在会影响Pgpool的启动
        PostgreCommonUtils.delete_path(f"/tmp/.s.PGSQL.{tgt_pgpool_port}")
        pcp_port = PostgreRestoreService.get_pgpool_pcp_port(pgpool_install_path)
        if pcp_port:
            PostgreCommonUtils.delete_path(f"/tmp/.s.PGSQL.{pcp_port}")
        wd_port = PostgreRestoreService.get_pgpool_wd_port(pgpool_install_path)
        if wd_port:
            PostgreCommonUtils.delete_path(f"/tmp/.s.PGPOOLWD_CMD.{wd_port}")
        PostgreCommonUtils.write_progress_info(cache_path, progress_dict.get("progress_file"), RestoreProgress(
            progress=progress_dict.get("success", {}).get("progress"),
            message=progress_dict.get("success", {}).get("message")))
        return True

    @staticmethod
    def check_patroni_pit_conf(pid, param_dict, cache_path):
        LOGGER.info("Begin to check patroni.yml before doing pit restore.")
        progress_dict = PRE_PROGRESS_MAP.get("one_four", {}).get("True", {})
        nodes = param_dict.get("job", {}).get("restoreSubObjects", [{}])
        host_ips = PostgreCommonUtils.get_local_ips()
        patroni_config, _, _ = PostgreCommonUtils.get_patroni_config(host_ips, nodes)
        expected_config = f"method: OceanProtectPITR"
        config_correct = False
        with open(patroni_config, 'r') as file:
            for line in file:
                if expected_config in line:
                    config_correct = True
        # 未正确配置patroni.yml文件，提醒用户参考文档进行正确配置
        if not config_correct:
            LOGGER.error(f"The patroni.yml file does not contain config: {expected_config}, path: {patroni_config}")
            PostgreCommonUtils.write_progress_info(cache_path, progress_dict.get("progress_file"), RestoreProgress(
                progress=progress_dict.get("fail", {}).get("progress"),
                message=progress_dict.get("fail", {}).get("message"),
                err_code=ErrorCode.PATRONI_YML_NOT_SET_BEFORE_PITR,
                err_code_param=["patroni.yml", "PostgreSQL"]))
            PostgreRestoreService.record_task_result(
                pid, "The patroni.yml is not set up properly. Please set up correctly according to the document",
                err_code=ErrorCode.PATRONI_YML_NOT_SET_BEFORE_PITR,
                body_err_params=["patroni.yml", "PostgreSQL"])
            return False
        LOGGER.info("Check patroni.yml succeed.")
        PostgreCommonUtils.write_progress_info(cache_path, progress_dict.get("progress_file"), RestoreProgress(
            progress=progress_dict.get("success", {}).get("progress"),
            message=progress_dict.get("success", {}).get("message")))
        return True

    @staticmethod
    def check_patroni_version():
        ret_code, std_out, std_err = execute_cmd("patronictl version")
        if ret_code != CMDResult.SUCCESS.value or PgConst.NOT_SUPPORT_PATRONI_VERSION in std_out:
            return False
        else:
            return True

    @staticmethod
    def pre_check_version_is_matched(pid, param_dict, cache_path, is_cluster=False):
        progress_dict = PRE_PROGRESS_MAP.get("three", {}).get(str(is_cluster), {})
        src_version = param_dict.get("job", {}).get("extendInfo", {}).get("copyProtectObjectVersion", "")
        tgt_version = param_dict.get("job", {}).get("targetObject", {}).get("extendInfo", {}).get("version", "")
        match_ver_ret = PostgreCommonUtils.is_db_version_matched(src_version, tgt_version)
        if not match_ver_ret:
            PostgreCommonUtils.write_progress_info(cache_path, progress_dict.get("progress_file"), RestoreProgress(
                progress=progress_dict.get("fail", {}).get("progress"),
                message=progress_dict.get("fail", {}).get("message"),
                err_code=ErrorCode.VERSION_NOT_MATCH_BEFORE_RESTORE))
            PostgreRestoreService.record_task_result(
                pid, "Database version of the copy does not match target instance version when restoring.",
                err_code=ErrorCode.VERSION_NOT_MATCH_BEFORE_RESTORE)
            return False
        PostgreCommonUtils.write_progress_info(cache_path, progress_dict.get("progress_file"), RestoreProgress(
            progress=progress_dict.get("success", {}).get("progress"),
            message=progress_dict.get("success", {}).get("message")))
        return True

    @staticmethod
    def pre_check_os_user_is_same(pid, param_dict, cache_path, is_cluster=False):
        progress_dict = PRE_PROGRESS_MAP.get("four", {}).get(str(is_cluster), {})
        src_os_user = PostgreRestoreService.parse_src_os_user(param_dict)
        tgt_os_user = PostgreRestoreService.parse_os_user(param_dict)
        LOGGER.info(f"Check if restore to same os user, src user: {src_os_user}, target user: {tgt_os_user}")
        if tgt_os_user != src_os_user:
            PostgreCommonUtils.write_progress_info(cache_path, progress_dict.get("progress_file"), RestoreProgress(
                progress=progress_dict.get("fail", {}).get("progress"),
                message=progress_dict.get("fail", {}).get("message"),
                err_code=ErrorCode.OS_USER_NOT_EQUAL_BEFORE_RESTORE))
            err_msg = f"The user of the database instance installation directory is different from " \
                      f"the system user running the database instance."
            PostgreRestoreService.record_task_result(pid, err_msg,
                                                     err_code=ErrorCode.OS_USER_NOT_EQUAL_BEFORE_RESTORE)
            return False
        PostgreCommonUtils.write_progress_info(cache_path, progress_dict.get("progress_file"), RestoreProgress(
            progress=progress_dict.get("success", {}).get("progress"),
            message=progress_dict.get("success", {}).get("message")))
        return True

    @staticmethod
    def pre_check_target_data_path_is_rw(pid, param_dict, cache_path, is_cluster=False):
        progress_dict = PRE_PROGRESS_MAP.get("five", {}).get(str(is_cluster), {})
        tgt_os_user = PostgreRestoreService.parse_os_user(param_dict)
        _, db_data_path, _ = PostgreRestoreService.get_db_install_and_data_path(param_dict)
        db_data_upper_path = os.path.realpath(os.path.join(db_data_path, ".."))
        is_dir_w_and_r = PostgreCommonUtils.is_dir_readable_and_writable_for_input_user(
            db_data_upper_path, tgt_os_user)
        LOGGER.info(f"Check if target dir: {db_data_upper_path} is readable and writable, os user: {tgt_os_user}.")
        if not is_dir_w_and_r:
            PostgreCommonUtils.write_progress_info(cache_path, progress_dict.get("progress_file"), RestoreProgress(
                progress=progress_dict.get("fail", {}).get("progress"),
                message=progress_dict.get("fail", {}).get("message"), err_code=ErrorCode.TARGET_NOT_RW_BEFORE_RESTORE))
            err_msg = f"The parent directory of the target instance data directory is not readable " \
                      f"and writable by the system user running the database instance."
            PostgreRestoreService.record_task_result(pid, err_msg,
                                                     err_code=ErrorCode.TARGET_NOT_RW_BEFORE_RESTORE)
            return False
        PostgreCommonUtils.write_progress_info(cache_path, progress_dict.get("progress_file"), RestoreProgress(
            progress=progress_dict.get("success", {}).get("progress"),
            message=progress_dict.get("success", {}).get("message")))
        return True

    @staticmethod
    def pre_check_table_space_path(pid, job_id, copy_mount_path):
        LOGGER.info(f"Begin to pre check table space path! job_id:{job_id}")
        table_spaces = PostgreRestoreService.get_table_spaces(copy_mount_path, None)
        if not table_spaces:
            LOGGER.info(f"There's no need to restore table space.job_id:{job_id}")
            return True
        for name, tb_path_info in table_spaces.items():
            os_username = tb_path_info[1]
            # 检查目录是否存在，存在时，检查目录权限
            real_path = os.path.realpath(tb_path_info[0])
            if os.path.exists(real_path):
                if not PostgreCommonUtils.get_path_owner(real_path) == os_username:
                    err_msg = f"The table space dir permission does not match.name:{name}, job_id:{job_id}"
                    PostgreRestoreService.record_task_result(pid, err_msg,
                                                             code=ErrorCode.TB_DIR_PERMISSION_IS_NOT_MATCH)
                    LOGGER.error(err_msg)
                    return False
            LOGGER.info(f"Clear table space dir success!name:{name}, job_id:{job_id}")
        return True

    @staticmethod
    def clear_table_space_dir(copy_mount_path=None, cache_path=None):
        LOGGER.info(f"Begin to clear table space dir!")
        table_spaces = PostgreRestoreService.get_table_spaces(copy_mount_path, cache_path)
        if not table_spaces:
            LOGGER.info(f"There's no need to restore table space.")
            return
        for name, tb_path_info in table_spaces.items():
            real_path = os.path.realpath(tb_path_info[0])
            # 目录存在时清空，不存在时创建
            try:
                PostgreRestoreService.clear_data_dir(tb_path_info[1], real_path)
            except Exception as ex:
                LOGGER.exception(f"Clear table space failed name:{name}, path:{real_path}")
                raise ErrCodeException(err_code=ErrorCode.CLEAR_TABLE_SPACE_DIR_FAILED,
                                       message="Clear table space dir failed!") from ex
            LOGGER.info(f"Clear table space dir success!name:{name}")
        return

    @staticmethod
    def get_table_spaces(copy_mount_path, cache_path):
        if copy_mount_path:
            table_space_path = os.path.realpath(os.path.join(copy_mount_path, DirAndFileNameConst.TABLE_SPACE_INFO_DIR,
                                                             DirAndFileNameConst.TABLE_SPACE_INFO_FILE))
        elif cache_path:
            table_space_path = os.path.realpath(os.path.join(cache_path, DirAndFileNameConst.TABLE_SPACE_INFO_FILE))
        else:
            table_space_path = ""

        if not os.path.exists(table_space_path):
            return {}
        with open(table_space_path, "r") as tb_file:
            tb_info = tb_file.read()
            table_spaces = json.loads(tb_info)
        return table_spaces

    @staticmethod
    @not_handle_exception_decorator
    def cleanup_archive_after_restore(param_dict):
        tgt_install_path, tgt_data_path, tgt_archive_dir = PostgreRestoreService.get_db_install_and_data_path(
            param_dict)
        config_file = param_dict.get("job", {}).get("targetObject", {}).get("extendInfo", {}).get("configFile", "")
        tgt_archive_path = PostgreCommonUtils.get_archive_path_offline(tgt_data_path, config_file=config_file,
                                                                       archive_dir=tgt_archive_dir)
        if not tgt_archive_path or not os.path.isdir(tgt_archive_path):
            LOGGER.warning(f"The obtained archive dir: {tgt_archive_path} is empty or does not exist.")
            return
        if not PostgreCommonUtils.check_black_list(tgt_archive_path):
            LOGGER.warning(f"The archive directory is invalid.")
            return
        PostgreCommonUtils.manual_cleanup_archive_dir(tgt_archive_path)

    @staticmethod
    def handle_cluster_instance_vip(param_dict, pgpool_install_path):
        # 获取vip
        vip = param_dict.get("job", {}).get("targetEnv", {}).get("endpoint")
        if not vip:
            LOGGER.warning("The vip obtained from the parameter is empty.")
            return
        local_ips = PostgreCommonUtils.get_local_ips()
        # vip已存在
        if vip in local_ips:
            LOGGER.info(f"The VIP of the cluster instance already exists, vip: {vip}.")
            return
        # vip不存在，尝试创建
        LOGGER.warning(f"There is no vip: {vip} on the current node, try to create")
        ori_if_up_cmd = PostgreRestoreService.get_pgpool_if_up_cmd(pgpool_install_path)
        if_up_cmd = ori_if_up_cmd.replace("$_IP_$", vip)
        PostgreCommonUtils.configure_vip(if_up_cmd)

    @staticmethod
    def parse_copies(job_dict):
        copies = job_dict.get("copies", [])
        if not copies:
            raise Exception("The copies value in the param file is empty or does not exist")
        return copies

    @staticmethod
    def parse_src_os_user(param_dict):
        """
        解析副本信息中的操作系统用户名
        """
        src_os_user = ""
        copies = param_dict.get("job", {}).get("copies", [])
        for tmp_copy in copies:
            tmp_os_user = tmp_copy.get("protectObject", {}).get("extendInfo", {}).get("osUsername", "")
            if tmp_os_user:
                src_os_user = tmp_os_user
                break
        return src_os_user

    @staticmethod
    def parse_os_user(param_dict):
        return param_dict.get("job", {}).get("targetObject", {}).get("extendInfo", {}).get("osUsername", "")

    @staticmethod
    def backup_conf_file(data_path, job_id):
        data_upper_path = os.path.realpath(os.path.join(data_path, ".."))
        for f_name in PgConst.NEED_BAK_CFG_FILES:
            tmp_src_file = os.path.realpath(os.path.join(data_path, f_name))
            if os.path.isfile(tmp_src_file):
                # 将配置文件备份到数据目录的上级目录
                PostgreCommonUtils.preserve_copy_file(tmp_src_file, data_upper_path,
                                                      tgt_file_name=f"{f_name}_{job_id}.bak")
                LOGGER.info(f"Backup config file: {f_name} successfully before restore.")
            else:
                LOGGER.warning(f"Backup config file: {f_name} on restore but doesn't exist.")

    @staticmethod
    def restore_conf_file(data_path, job_id, param_dict=None):
        data_upper_path = os.path.realpath(os.path.join(data_path, ".."))
        for f_name in PgConst.NEED_BAK_CFG_FILES:
            bak_f_name = f"{f_name}_{job_id}.bak"
            tmp_src_file = os.path.realpath(os.path.join(data_upper_path, bak_f_name))
            if os.path.isfile(tmp_src_file):
                # 将备份在数据目录上级目录的配置文件恢复到数据目录
                PostgreCommonUtils.preserve_copy_file(tmp_src_file, data_path, tgt_file_name=f_name)
                LOGGER.info(f"Restore config file: {bak_f_name} successfully.")
            else:
                LOGGER.warning(f"The config file: {bak_f_name} that has been backed up does not exist when restoring.")
        if param_dict:
            # 恢复的时候，看看那个位置上是否有配置文件，如果有，就用原来的，如果没有，就用看data目录的（也就是副本里的），并拷贝到记录的位置上。
            # 从资源中获取config_file，如果config_file不存在，就从data数据目录中拷贝到config_file所在的位置
            PostgreRestoreService.restore_config_file(data_path, param_dict, 'configFile')
            PostgreRestoreService.restore_config_file(data_path, param_dict, 'hbaFile')
            PostgreRestoreService.restore_config_file(data_path, param_dict, 'identFile')

    @staticmethod
    def restore_config_file(data_path, param_dict, config_name):
        config_file = param_dict.get("job", {}).get("targetObject", {}).get("extendInfo", {}).get(config_name, "")
        if config_file and not os.path.exists(config_file):
            # 从全量副本中获取配置文件的信息，并将配置文件移动到指定位置
            copies = PostgreRestoreService.parse_copies(param_dict.get("job", {}))
            copy_config_file = copies[0].get("extendInfo", {}).get(config_name, "")
            LOGGER.info(f"{config_name} : {copy_config_file}")
            if copy_config_file:
                source_config_file = os.path.join(os.path.abspath(data_path), os.path.basename(copy_config_file))
                if os.path.exists(source_config_file):
                    shutil.move(source_config_file, config_file)

    @staticmethod
    def merge_log_copies(os_user, log_copies, merged_path, job_id=""):
        if not PostgreCommonUtils.check_special_characters(os_user):
            raise Exception("String contains special characters!")
        if os.path.exists(merged_path):
            PostgreCommonUtils.delete_path(merged_path)
        LOGGER.info(f"Try to create copy merge path: {merged_path}.")
        LOGGER.info(f"Creating copy merge path: {merged_path}")
        ret = exec_mkdir_cmd(merged_path, os_user, 0o700, is_check_white_list=False)

        if not ret:
            LOGGER.error(f"Create copy merge path failed")
            raise Exception("Create copy merge path failed.")
        LOGGER.info("Create copy merge path success.")
        loop_time = 1
        for tmp_copy in log_copies:
            PostgreCommonUtils.copy_files(tmp_copy, merged_path, str(loop_time), job_id=job_id)
            loop_time += 1
        LOGGER.info("Merge log copies success.")

    @staticmethod
    def handle_log_copy(param_dict, job_id):
        job_dict = param_dict.get("job", {})
        copies = PostgreRestoreService.parse_copies(job_dict)
        log_copy_paths = []
        for tmp_copy in copies[1:]:
            tmp_log_copies = PostgreRestoreService.get_copy_mount_paths(
                tmp_copy, RepositoryDataTypeEnum.LOG_REPOSITORY.value)
            log_copy_paths.extend(tmp_log_copies)
        tgt_obj_extend_info_dict = job_dict.get("targetObject", {}).get("extendInfo", {})
        tgt_db_os_user = tgt_obj_extend_info_dict.get("osUsername", "")
        # 日志副本路径去重
        log_copy_paths = list(set(log_copy_paths))
        # log_copy_path处理成E6000需要的格式
        copy_mount_paths = []
        for log_copy_path in log_copy_paths:
            copy_mount_paths.append(get_livemount_path(job_id, log_copy_path))
        log_copy_paths = copy_mount_paths
        if log_copy_paths:
            cache_path = PostgreRestoreService.get_copy_mount_paths(
                copies[0], RepositoryDataTypeEnum.CACHE_REPOSITORY.value)[0]
            merged_path = os.path.realpath(os.path.join(cache_path, "merged_log_copies"))
            PostgreRestoreService.merge_log_copies(tgt_db_os_user, log_copy_paths, merged_path, job_id=job_id)
        else:
            LOGGER.error("No log copy mount path exists for point-in-time recovery.")
            raise Exception("No log copy mount path exists for point-in-time recovery.")
        PostgreRestoreService.change_owner_of_download_data(param_dict, merged_path, is_data_dir=False)
        return merged_path

    @staticmethod
    def restore_data(cache_path, src_path: str, tgt_path: str, wildcard=".", job_id=""):
        restore_table_space_res = PostgreRestoreService.restore_table_space(src_path, cache_path, job_id)
        if restore_table_space_res != ExecuteResultEnum.SUCCESS:
            LOGGER.error("Restore table space failed!")
            raise ErrCodeException(restore_table_space_res, message="Restore table space failed")
        try:
            PostgreCommonUtils.copy_directory(src_path, tgt_path, wildcard=wildcard, job_id=job_id)
        except Exception as ex:
            LOGGER.error(f'Restore data failed, wildcard: "{wildcard}", source: {src_path}, target: {tgt_path}.')
            raise ErrCodeException(ErrorCode.RESTORE_DATA_FAILED, message="Restore data failed") from ex

    @staticmethod
    def restore_pg_wal_dir(tgt_wal_path: str, tgt_wal_real_path: str, job_id=""):
        if tgt_wal_path == tgt_wal_real_path:
            return
        try:
            if not os.path.islink(tgt_wal_real_path):
                clean_dir_not_walk_link(tgt_wal_real_path)
            PostgreCommonUtils.copy_directory(tgt_wal_path, tgt_wal_real_path, wildcard=".", job_id=job_id)
            clean_dir_not_walk_link(tgt_wal_path)
            shutil.rmtree(tgt_wal_path)
            os.symlink(tgt_wal_real_path, tgt_wal_path)
        except Exception as ex:
            LOGGER.error(f'Restore data failed, tgt_wal_path: {tgt_wal_path}, tgt_wal_real_path: {tgt_wal_real_path}.')
            raise ErrCodeException(ErrorCode.RESTORE_DATA_FAILED, message="Restore data failed") from ex

    @staticmethod
    def restore_table_space(src_path, cache_path, job_id):
        """1.检查文件系统中是否有table_space,没有的话直接return
            2.获取文件系统中table_space的权限等信息
            3.检查本地是否存在table_space路径，存在的话需要校验权限跟源端是否一样
            4.一样的话执行备份table_space
            5.所有的table_space备份完成后，删除clone文件系统中的pgsql_table_space目录
        """
        table_space_path = os.path.join(src_path, DirAndFileNameConst.TABLE_SPACE_INFO_DIR,
                                        DirAndFileNameConst.TABLE_SPACE_INFO_FILE)
        if not os.path.exists(table_space_path):
            LOGGER.info("There is no need to restore table space.")
            return ExecuteResultEnum.SUCCESS
        with open(table_space_path, "r") as tb_file:
            tb_info = tb_file.read()
            table_spaces = json.loads(tb_info)
        # 恢复表空间目录
        if not PostgreRestoreService.restore_table_spaces_data(job_id, table_spaces, src_path):
            return ErrorCode.TB_DIR_PERMISSION_IS_NOT_MATCH
        LOGGER.info(f"Restore all table space success! job_id:{job_id}")

        # 将表空间信息放入cache仓
        if not PostgreRestoreService.copy_table_space_info_to_cache_repository(table_space_path, cache_path):
            return ErrorCode.RESTORE_TABLE_SPACE_FAILED
        LOGGER.info(f"Copy table space info to cache repository success! job_id:{job_id}")

        # 清空clone文件系统中的表空间信息
        if not PostgreRestoreService.del_table_space_info_in_filesystem(src_path):
            return ErrorCode.RESTORE_TABLE_SPACE_FAILED
        LOGGER.info(f"Clear table space info in clone fs success! job_id:{job_id}")
        return ExecuteResultEnum.SUCCESS

    @staticmethod
    def restore_table_spaces_data(job_id, table_spaces, src_path):
        for name, tb_info in table_spaces.items():
            path = os.path.realpath(os.path.join(tb_info[0], ".."))
            tb_path = os.path.join(src_path, DirAndFileNameConst.TABLE_SPACE_INFO_DIR, *path.split("/"))
            tb_job_id = str(uuid.uuid5(uuid.NAMESPACE_X500, job_id + DirAndFileNameConst.TABLE_SPACE_INFO_DIR + name))
            result = PostgreCommonUtils.copy_directory(tb_path, path, wildcard=".", job_id=tb_job_id)
            if not result:
                LOGGER.error(f"Restore table space :{name} failed!, job_id:{tb_job_id}")
                return False
        return True

    @staticmethod
    def copy_table_space_info_to_cache_repository(table_space_path, cache_path):
        ret = exec_cp_cmd(table_space_path, cache_path, option="-rf", is_check_white_list=False)
        if not ret:
            LOGGER.error(f"Copy PGSQL_TABLE_SPACE.info to cache repository failed.")
            return False
        return True

    @staticmethod
    def del_table_space_info_in_filesystem(src_path):
        tb_data_dir = os.path.join(src_path, DirAndFileNameConst.TABLE_SPACE_INFO_DIR)
        # 校验删除路径是否在白名单中
        res, tb_dir = PostgreCommonUtils.check_path_in_white_list(tb_data_dir)
        if not res:
            LOGGER.error(f"The table space path not in white list.")
            return False
        check_del_dir(tb_dir)
        return True

    @staticmethod
    def sync_master_data(pid, param_dict):
        enable_root = PostgreCommonUtils.get_root_switch()
        tgt_install_path, tgt_data_path, _ = PostgreRestoreService.get_db_install_and_data_path(param_dict)
        pg_backup_path = os.path.realpath(os.path.join(tgt_install_path, "bin", "pg_basebackup"))
        tgt_obj = param_dict.get("job", {}).get("targetObject", {})
        os_user = tgt_obj.get("extendInfo", {}).get("osUsername", "")
        primary_ip, inst_port = PostgreRestoreService.get_primary_ip_and_instance_port(param_dict)
        repl_user = tgt_obj.get("auth", {}).get("extendInfo", {}).get("dbStreamRepUser")
        repl_pwd = get_env_variable(f"job_targetObject_auth_extendInfo_dbStreamRepPwd_{pid}")
        copy_repl_user, copy_repl_pwd = PostgreCommonUtils.get_repl_info(param_dict.get("job", {}))
        if copy_repl_user:
            repl_user = copy_repl_user
            repl_pwd = copy_repl_pwd
        if not PostgreRestoreService.check_params_for_standby([os_user, repl_user], [tgt_install_path, tgt_data_path],
                                                              primary_ip, inst_port):
            raise ErrCodeException(ErrorCode.RESTORE_DATA_FAILED, message="Sync master data to target data path failed")
        if not PostgreCommonUtils.check_os_user(os_user, tgt_install_path, enable_root)[0]:
            raise ErrCodeException(ErrorCode.RESTORE_DATA_FAILED, message="Os username is not exist!")
        PostgreCommonUtils.check_file_path(pg_backup_path)
        PostgreCommonUtils.check_path_islink(pg_backup_path)
        if not enable_root:
            if not check_user_utils.check_path_owner(pg_backup_path, [os_user]):
                LOGGER.error(f"Pg backup path and os user is not matching.os user:{os_user}")
                raise ErrCodeException(ErrorCode.RESTORE_DATA_FAILED,
                                       message="Pg backup path and os user is not matching.")
        PostgreCommonUtils.check_dir_path(tgt_data_path)
        PostgreCommonUtils.check_path_islink(tgt_data_path)
        if not enable_root:
            if not check_user_utils.check_path_owner(tgt_data_path, [os_user]):
                LOGGER.error(f"Data dir and os user is not matching.os user:{os_user}")
                raise ErrCodeException(ErrorCode.RESTORE_DATA_FAILED, message="Data dir and os user is not matching.")
        try:
            # force password prompt
            install_deploy_type = param_dict.get("job", {}).get("targetEnv", {}).get("extendInfo", {}).get(
                "installDeployType", InstallDeployType.PGPOOL)
            if install_deploy_type == InstallDeployType.PGPOOL:
                pg_backup_cmd = cmd_format('su - {} -c "{} -h {} -p {} -U {} -D {} -X stream -W"', os_user,
                                           pg_backup_path,
                                           primary_ip, inst_port, repl_user, tgt_data_path)
            else:
                pg_backup_cmd = cmd_format('su - {} -c "{} -h {} -p {} -U {} -D {} -R -X stream -W"', os_user,
                                           pg_backup_path,
                                           primary_ip, inst_port, repl_user, tgt_data_path)
            if not PostgreRestoreService.retry_sync_data(pg_backup_cmd, repl_pwd, param_dict):
                LOGGER.error("Sync master data to target data path failed.")
                raise ErrCodeException(ErrorCode.RESTORE_DATA_FAILED,
                                       message="Sync master data to target data path failed")
            if install_deploy_type == InstallDeployType.CLUP:
                local_node_ip = PostgreRestoreService.get_local_node_ip(param_dict)
                PostgreRestoreService.replace_primary_conninfo(tgt_data_path, local_node_ip)
        finally:
            cleaner.clear(repl_pwd)

    @staticmethod
    def replace_primary_conninfo(tgt_data_path, local_node_ip):
        postgresql_auto_conf_path = os.path.join(tgt_data_path, PgConst.POSTGRESQL_AUTO_CONF_FILE_NAME)
        ret, mount_list = read_lines_cmd(postgresql_auto_conf_path)
        for i, line in enumerate(mount_list):
            if line.startswith("primary_conninfo ="):
                mount_list[i] = mount_list[i].replace('user=', f'application_name={local_node_ip} user=')
                break
        lines_conn = '\n'.join(mount_list) + '\n'
        exec_overwrite_file(postgresql_auto_conf_path, lines_conn, json_flag=False)

    @staticmethod
    def check_params_for_standby(special_characters: list, paths: list, primary_ip: str, inst_port: str):
        for param in special_characters:
            if not PostgreCommonUtils.check_special_characters(param):
                LOGGER.error(f"sync master data param error!param:{param}")
                return False

        for path in paths:
            try:
                PostgreCommonUtils.check_path_exist(path)
            except Exception as ex:
                LOGGER.error(f"sync master data path error!path:{path}", ex)
                return False

        if not check_utils.is_ip_address(primary_ip):
            LOGGER.error(f"sync master primary ip error!primary ip:{primary_ip}")
            return False

        if not check_utils.is_port(inst_port):
            LOGGER.error(f"sync master inst port error!inst port:{inst_port}")
            return False
        return True

    @staticmethod
    def exec_sync_data_cmd(pg_backup_cmd, repl_pwd):
        LOGGER.info(f"Start executing sync master data command: {pg_backup_cmd}.")
        ssh = pexpect.spawn(pg_backup_cmd, encoding='utf-8')
        index = ssh.expect(['Password:', pexpect.EOF, pexpect.TIMEOUT], timeout=10)
        if index != NumberConst.ZERO:
            LOGGER.error(f"An incorrect prompt occurred while syncing the master data, result index: {index}, "
                         f"message: {ssh.before}.")
            raise Exception(f"An incorrect prompt occurred while syncing the master data")
        ssh.sendline(repl_pwd)
        index = ssh.expect([pexpect.EOF, pexpect.TIMEOUT], timeout=PgConst.CHECK_POINT_TIME_OUT)
        if index != NumberConst.ZERO:
            LOGGER.error(f"Sync master data to target data path failed, result index: {index}, message: {ssh.before}.")
            raise ErrCodeException(ErrorCode.RESTORE_DATA_FAILED,
                                   message="Sync master data to target data path failed")
        LOGGER.info(f"Sync master data to target data path success.")

    @staticmethod
    def retry_sync_data(pg_backup_cmd, repl_pwd, param_dict):
        """重试3次从主节点同步数据"""
        for i in range(3):
            if i != NumberConst.ZERO:
                # 重试需要清空data目录和tablespace目录
                time.sleep(NumberConst.THREE)
                PostgreRestoreService.clear_data_dir_in_slave(param_dict)
            try:
                PostgreRestoreService.exec_sync_data_cmd(pg_backup_cmd, repl_pwd)
            except ErrCodeException:
                LOGGER.warning(f"Sync data failed! Retry times is :{i}")
                continue
            if PostgreRestoreService.check_sync_data_is_success(param_dict):
                cleaner.clear(repl_pwd)
                return True
        LOGGER.error("Retry end, sync data failed!")
        cleaner.clear(repl_pwd)
        return False

    @staticmethod
    def check_sync_data_is_success(param_dict):
        _, tgt_data_path, _ = PostgreRestoreService.get_db_install_and_data_path(param_dict)
        pg_conf_file = os.path.realpath(os.path.join(tgt_data_path, PgConst.POSTGRESQL_CONF_FILE_NAME))
        if not os.path.isfile(pg_conf_file):
            LOGGER.warning("The postgresql.conf file does not exist in the database data directory")
            return False
        return True

    @staticmethod
    def clear_data_dir_in_slave(param_dict):
        # 1.清空data目录
        tgt_install_path, tgt_data_path, _ = PostgreRestoreService.get_db_install_and_data_path(param_dict)
        PostgreRestoreService.clear_data_dir(PostgreRestoreService.parse_os_user(param_dict), tgt_data_path)
        # 1.1 清空tablespace目录
        cache_path = PostgreRestoreService.get_cache_mount_path(param_dict)
        PostgreRestoreService.clear_table_space_dir(cache_path=cache_path)

    @staticmethod
    def is_restore_cluster(param_dict: dict):
        """判断是否在恢复集群
        """
        deploy_type = param_dict.get("job", {}).get("targetEnv", {}).get("extendInfo", {}).get("deployType")
        if deploy_type not in (DeployTypeEnum.SINGLE.value, DeployTypeEnum.AP.value):
            LOGGER.error(f"Deploy type param: {deploy_type} is invalid.")
            raise Exception("Deploy type param is invalid")
        return deploy_type == DeployTypeEnum.AP.value

    @staticmethod
    def clear_archive_status_dir(tgt_version: str, tgt_data_path: str):
        if not os.path.isdir(tgt_data_path):
            LOGGER.error(f"Clear archive_status dir, target data path: {tgt_data_path} is invalid.")
            raise Exception("Target data path is invalid when clearing archive_status directory")
        if not tgt_version or len(tgt_version.split('.')) < 2:
            LOGGER.error(f"Clear archive_status dir, target version: {tgt_version} is invalid.")
            raise Exception("Target version is invalid when clearing archive_status directory")
        major_ver = tgt_version.split(".")[0]
        if int(major_ver) <= PgConst.MAJOR_VERSION_NINE:
            archive_status_dir_list = PgConst.ARCHIVE_STATUS_DIR_V9_AND_BELOW_PATHS
        else:
            archive_status_dir_list = PgConst.ARCHIVE_STATUS_DIR_V10_AND_ABOVE_PATHS
        archive_status_dir = os.path.realpath(os.path.join(tgt_data_path, *archive_status_dir_list))
        PostgreCommonUtils.clear_dir_when_exist(archive_status_dir)

    @staticmethod
    def delete_useless_files_of_data_dir(tgt_data_path, before_restore=True):
        if before_restore:
            for file_name in PgConst.DELETE_FILE_NAMES_OF_DATA_DIR:
                tmp_path = os.path.realpath(os.path.join(tgt_data_path, file_name))
                PostgreCommonUtils.delete_path(tmp_path)
        else:
            for file_name in PgConst.DELETE_FILE_NAMES_OF_DATA_DIR_AFTER_RESTORE:
                tmp_path = os.path.realpath(os.path.join(tgt_data_path, file_name))
                PostgreCommonUtils.delete_path(tmp_path)

    @staticmethod
    def delete_useless_bak_files(tgt_data_path, job_id):
        for file_name in PgConst.NEED_BAK_CFG_FILES:
            bak_f_name = f"{file_name}_{job_id}.bak"
            tmp_path = os.path.realpath(os.path.join(tgt_data_path, "..", bak_f_name))
            PostgreCommonUtils.delete_path(tmp_path)

    @staticmethod
    def set_recovery_conf_file(param_dict, cfg_param: RestoreConfigParam, role=RoleType.NONE_TYPE):
        """设置恢复命令文件
        """
        install_deploy_type = param_dict.get("job", {}).get("targetEnv", {}).get("extendInfo", {}).get(
            "installDeployType", InstallDeployType.PGPOOL)
        tgt_version = cfg_param.target_version
        if not tgt_version or len(tgt_version.split('.')) < 2:
            LOGGER.error(f"Clear archive_status dir, target version: {tgt_version} is invalid.")
            raise Exception("Target version is invalid when clearing archive_status directory")
        major_ver = tgt_version.split(".")[0]
        # 12以下版本设置recovery.conf文件
        if int(major_ver) < PgConst.DATABASE_V12:
            PostgreRestoreService.set_recovery_conf_for_pg_below_v12(cfg_param, role=role)
        # 12及以上版本设置postgresql.conf文件
        else:
            PostgreRestoreService.set_recovery_conf_for_pg_v12_and_above(install_deploy_type, cfg_param, role=role)

    @staticmethod
    def set_restore_command(recovery_cmd_file, log_path, rollback=False):
        """设置restore_command值
        """
        PostgreCommonUtils.check_path_islink(recovery_cmd_file)
        file_owner = get_path_owner(recovery_cmd_file)
        key_status = PostgreCommonUtils.get_conf_item_status(recovery_cmd_file, "restore_command =")
        if key_status == ConfigKeyStatus.ANNOTATED:
            replace_str = f"s?^#restore_command =.*?restore_command = 'cp {log_path}/%f %p'?g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {recovery_cmd_file}']
        elif key_status == ConfigKeyStatus.CONFIGURED:
            replace_str = f"s?^restore_command =.*?restore_command = 'cp {log_path}/%f %p'?g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {recovery_cmd_file}']
        else:
            cmd_list = []
        if rollback:
            replace_str = f"s?^restore_command =.*?#restore_command = ''?g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {recovery_cmd_file}']
        LOGGER.info(f"Setting restore command ... command: {cmd_list}")
        if not cmd_list:
            cmd_content = f"restore_command = 'cp {log_path}/%f %p'"
            PostgreCommonUtils.write_content_to_file(recovery_cmd_file, cmd_content)
            LOGGER.info("append restore_command to file success.")
            return
        return_code, out, err = execute_cmd_list(cmd_list)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Execute set restore command failed, return code: {return_code}, "
                         f"out: {out}, err: {err}")
            raise Exception("Set restore command failed")
        LOGGER.info("Set restore command success.")

    @staticmethod
    def set_standby_mode_command(recovery_cmd_file):
        """
        从节点恢复设置standby_mode值为on
        注：v12以下有该字段，v12及以上无该字段
        """
        PostgreCommonUtils.check_path_islink(recovery_cmd_file)
        file_owner = get_path_owner(recovery_cmd_file)
        key_status = PostgreCommonUtils.get_conf_item_status(recovery_cmd_file, "standby_mode =")
        if key_status == ConfigKeyStatus.ANNOTATED:
            replace_str = f"s?^#standby_mode =.*?standby_mode = on?g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {recovery_cmd_file}']
        elif key_status == ConfigKeyStatus.CONFIGURED:
            replace_str = f"s?^standby_mode =.*?standby_mode = on?g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {recovery_cmd_file}']
        else:
            cmd_list = []
        LOGGER.info(f"Setting standby_mode ... command: {cmd_list}")
        if not cmd_list:
            cmd_content = f"standby_mode = on"
            PostgreCommonUtils.write_content_to_file(recovery_cmd_file, cmd_content)
            LOGGER.info("append standby_mode to file success.")
            return
        return_code, out, err = execute_cmd_list(cmd_list)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Execute set standby_mode failed, return code: {return_code}, "
                         f"out: {out}, err: {err}")
            raise Exception("Set primary_conninfo failed")
        LOGGER.info("Set standby_mode success.")

    @staticmethod
    def set_primary_conn_info_command(recovery_cmd_file, cfg_param: RestoreConfigParam):
        """从节点恢复设置primary_conninfo值
        """
        PostgreCommonUtils.check_path_islink(recovery_cmd_file)
        file_owner = get_path_owner(recovery_cmd_file)
        primary_ip = cfg_param.extend_info.get("primary_ip")
        primary_inst_port = cfg_param.extend_info.get("primary_inst_port")
        repl_user = cfg_param.extend_info.get("db_stream_rep_user")
        repl_pwd = cfg_param.extend_info.get("db_stream_rep_pwd")
        if not PostgreCommonUtils.check_special_characters(repl_user):
            raise Exception("String contains special characters!")
        primary_conn_info = f"host={primary_ip} port={primary_inst_port} user={repl_user} password={repl_pwd}"
        key_status = PostgreCommonUtils.get_conf_item_status(recovery_cmd_file, "primary_conninfo =")
        if key_status == ConfigKeyStatus.ANNOTATED:
            replace_str = f"s?^#primary_conninfo =.*?primary_conninfo = '{primary_conn_info}'?g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {recovery_cmd_file}']
        elif key_status == ConfigKeyStatus.CONFIGURED:
            replace_str = f"s?^primary_conninfo =.*?primary_conninfo = '{primary_conn_info}'?g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {recovery_cmd_file}']
        else:
            cmd_list = []
        LOGGER.info(f"Setting primary_conninfo ... command: {cmd_list}")
        if not cmd_list:
            cmd_content = f"primary_conninfo = '{primary_conn_info}'"
            PostgreCommonUtils.write_content_to_file(recovery_cmd_file, cmd_content)
            LOGGER.info("append primary_conninfo to file success.")
            return
        return_code, out, err = execute_cmd_list(cmd_list)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Execute set primary_conninfo failed, return code: {return_code}, "
                         f"out: {out}, err: {err}")
            raise Exception("Set primary_conninfo failed")
        LOGGER.info("Set primary_conninfo success.")

    @staticmethod
    def set_recovery_target_param(recovery_cmd_file, recovery_tgt, rollback=False):
        """恢复设置recovery_target值
        """
        PostgreCommonUtils.check_path_islink(recovery_cmd_file)
        file_owner = get_path_owner(recovery_cmd_file)
        key_status = PostgreCommonUtils.get_conf_item_status(recovery_cmd_file, "recovery_target =")
        if key_status == ConfigKeyStatus.ANNOTATED:
            replace_str = f"s?^#recovery_target =.*?recovery_target = '{recovery_tgt}'?g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {recovery_cmd_file}']
        elif key_status == ConfigKeyStatus.CONFIGURED:
            replace_str = f"s?^recovery_target =.*?recovery_target = '{recovery_tgt}'?g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {recovery_cmd_file}']
        else:
            cmd_list = []
        if rollback:
            replace_str = f"s?^recovery_target =.*?#recovery_target = ''?g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {recovery_cmd_file}']
        LOGGER.info(f"Setting recovery_target param ... command: {cmd_list}")
        if not cmd_list:
            cmd_content = f"recovery_target = '{recovery_tgt}'"
            PostgreCommonUtils.write_content_to_file(recovery_cmd_file, cmd_content)
            LOGGER.info("append recovery_target param to file success.")
            return
        return_code, out, err = execute_cmd_list(cmd_list)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Execute set recovery_target param failed, return code: {return_code}, "
                         f"out: {out}, err: {err}")
            raise Exception("Set recovery_target param failed")
        LOGGER.info("Set recovery_target param success.")

    @staticmethod
    def set_recovery_target_timeline_param(recovery_cmd_file, timeline, rollback=False):
        """恢复设置recovery_target_timeline值
        """
        PostgreCommonUtils.check_path_islink(recovery_cmd_file)
        file_owner = get_path_owner(recovery_cmd_file)
        key_status = PostgreCommonUtils.get_conf_item_status(recovery_cmd_file, "recovery_target_timeline =")
        if key_status == ConfigKeyStatus.ANNOTATED:
            replace_str = f"s?^#recovery_target_timeline =.*?recovery_target_timeline = '{timeline}'?g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {recovery_cmd_file}']
        elif key_status == ConfigKeyStatus.CONFIGURED:
            replace_str = f"s?^recovery_target_timeline =.*?recovery_target_timeline = '{timeline}'?g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {recovery_cmd_file}']
        else:
            cmd_list = []
        if rollback:
            replace_str = f"s?^recovery_target_timeline =.*?#recovery_target_timeline = ''?g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {recovery_cmd_file}']
        LOGGER.info(f"Setting recovery_target_timeline param ... command: {cmd_list}")
        if not cmd_list:
            cmd_content = f"recovery_target_timeline = '{timeline}'"
            PostgreCommonUtils.write_content_to_file(recovery_cmd_file, cmd_content)
            LOGGER.info("append recovery_target_timeline param to file success.")
            return
        return_code, out, err = execute_cmd_list(cmd_list)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Execute set recovery_target_timeline param failed, return code: {return_code}, "
                         f"out: {out}, err: {err}")
            raise Exception("Set recovery_target_timeline param failed")
        LOGGER.info("Set recovery_target_timeline param success.")

    @staticmethod
    def set_hot_standby_command(pg_cfg_file):
        PostgreCommonUtils.check_path_islink(pg_cfg_file)
        file_owner = get_path_owner(pg_cfg_file)
        key_status = PostgreCommonUtils.get_conf_item_status(pg_cfg_file, "hot_standby =")
        if key_status == ConfigKeyStatus.ANNOTATED:
            replace_str = f"s?^#hot_standby =.*?hot_standby = on?g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {pg_cfg_file}']
        elif key_status == ConfigKeyStatus.CONFIGURED:
            replace_str = f"s?^hot_standby =.*?hot_standby = on?g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {pg_cfg_file}']
        else:
            cmd_list = []
        LOGGER.info(f"Setting hot_standby ... command: {cmd_list}")
        if not cmd_list:
            cmd_content = f"hot_standby = on"
            PostgreCommonUtils.write_content_to_file(pg_cfg_file, cmd_content)
            LOGGER.info("append hot_standby to file success.")
            return
        return_code, out, err = execute_cmd_list(cmd_list)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Execute set hot_standby failed, return code: {return_code}, "
                         f"out: {out}, err: {err}")
            raise Exception("Set hot_standby failed")
        LOGGER.info("Set hot_standby success.")

    @staticmethod
    def set_pitr_target_time(cfg_file, recovery_target_time, rollback=False):
        """设置按时间点恢复的recovery_target_time参数
        """
        PostgreCommonUtils.check_path_islink(cfg_file)
        file_owner = get_path_owner(cfg_file)
        key_status = PostgreCommonUtils.get_conf_item_status(cfg_file, "recovery_target_time =")
        if key_status == ConfigKeyStatus.ANNOTATED:
            replace_str = f"s/^#recovery_target_time =.*/recovery_target_time = '{recovery_target_time}'/g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {cfg_file}']
        elif key_status == ConfigKeyStatus.CONFIGURED:
            replace_str = f"s/^recovery_target_time =.*/recovery_target_time = '{recovery_target_time}'/g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {cfg_file}']
        else:
            cmd_list = []
        if rollback:
            replace_str = f"s/^recovery_target_time =.*/#recovery_target_time = ''/g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {cfg_file}']
        LOGGER.info(f"Setting recovery target time ... command: {cmd_list}")
        if not cmd_list:
            cmd_content = f"recovery_target_time = '{recovery_target_time}'"
            PostgreCommonUtils.write_content_to_file(recovery_target_time, cmd_content)
            LOGGER.info("append recovery target time to file success.")
            return
        return_code, out, err = execute_cmd_list(cmd_list)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Execute set recovery target time failed, return code: {return_code}, "
                         f"out: {out}, err: {err}")
            raise Exception("Set recovery target time failed")
        LOGGER.info(f"Set recovery target time success.")

    @staticmethod
    def set_recovery_target_action(cfg_file, tgt_action, rollback=False):
        """设置recovery_target_action参数（针对9.5及以上版本）
        """
        PostgreCommonUtils.check_path_islink(cfg_file)
        file_owner = get_path_owner(cfg_file)
        key_status = PostgreCommonUtils.get_conf_item_status(cfg_file, "recovery_target_action =")
        if key_status == ConfigKeyStatus.ANNOTATED:
            replace_str = f"s/^#recovery_target_action =.*/recovery_target_action = '{tgt_action}'/g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {cfg_file}']
        elif key_status == ConfigKeyStatus.CONFIGURED:
            replace_str = f"s/^recovery_target_action =.*/recovery_target_action = '{tgt_action}'/g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {cfg_file}']
        else:
            cmd_list = []
        if rollback:
            replace_str = f"s/^recovery_target_action =.*/#recovery_target_action = '{tgt_action}'/g"
            cmd_list = [f'su - {file_owner}', f'sed -i "{replace_str}" {cfg_file}']
        LOGGER.info(f"Setting recovery target action ... command: {cmd_list}")
        if not cmd_list:
            cmd_content = f"recovery_target_action = '{tgt_action}'"
            PostgreCommonUtils.write_content_to_file(tgt_action, cmd_content)
            LOGGER.info("append recovery target action to file success.")
            return
        return_code, out, err = execute_cmd_list(cmd_list)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Execute set recovery target action failed, return code: {return_code}, "
                         f"out: {out}, err: {err}")
            raise Exception("Set recovery target action failed")
        LOGGER.info(f"Set recovery target action success.")

    @staticmethod
    def create_signal_file(cfg_param, role):
        signal_file = os.path.realpath(os.path.join(cfg_param.target_data_path, PgConst.RECOVERY_SIGNAL_FILE_NAME))
        if str(role) == str(RoleType.STANDBY.value):
            LOGGER.info("Current node is standby node.")
            signal_file = os.path.realpath(os.path.join(cfg_param.target_data_path, PgConst.STANDBY_SIGNAL_FILE_NAME))
        if not PostgreCommonUtils.check_special_characters(cfg_param.system_user):
            raise Exception("String contains special characters!")
        create_signal_file_cmd = cmd_format("su - {} -c 'touch {}'", cfg_param.system_user, signal_file)
        LOGGER.info(f"Creating signal file ... command: {create_signal_file_cmd}")
        return_code, out, err = restore_svc_common.execute_cmd(create_signal_file_cmd)
        if return_code != CmdRetCode.EXEC_SUCCESS.value:
            LOGGER.error(f"Create signal file failed, return code: {return_code}, "
                         f"out: {out}, err: {err}")
            raise Exception("Create signal file failed")
        LOGGER.info("Create signal file success.")

    @staticmethod
    def set_recovery_conf_for_pg_v12_and_above(deploy_type, cfg_param: RestoreConfigParam, role=RoleType.NONE_TYPE):
        LOGGER.info("Start setting recovery conf for postgresql v12 and above ...")
        tgt_data_path = cfg_param.target_data_path

        # 检查postgresql.conf文件是否存在
        if deploy_type == InstallDeployType.PATRONI:
            pg_conf_file = os.path.realpath(os.path.join(tgt_data_path, PgConst.POSTGRESQL_BASE_CONF_FILE))
        else:
            config_file = cfg_param.extend_info.get("configFile", "")
            pg_conf_file = config_file if config_file else os.path.realpath(
                os.path.join(tgt_data_path, PgConst.POSTGRESQL_CONF_FILE_NAME))
        if not os.path.isfile(pg_conf_file):
            LOGGER.error("The postgresql.conf file does not exist in the database data directory")
            raise Exception("Postgresql configuration file does not exist in the database data directory")
        LOGGER.info("The postgresql.conf file exists in the database data directory")

        # 创建recovery.signal或者standby.signal文件
        PostgreRestoreService.create_signal_file(cfg_param, role)

        if str(role) == str(RoleType.STANDBY.value):
            LOGGER.info("Current node is standby node.")
            PostgreRestoreService.set_primary_conn_info_command(pg_conf_file, cfg_param)
            PostgreRestoreService.set_recovery_target_timeline_param(pg_conf_file, PgConst.RECOVERY_TGT_TIMELINE_LATEST)
            PostgreRestoreService.set_hot_standby_command(pg_conf_file)

        # 非时间点恢复，无需设置restore_command和recovery_target_time
        recovery_target_time = cfg_param.recovery_target_time
        if not recovery_target_time:
            wal_dir_name = PostgreCommonUtils.get_wal_dir_name_by_version(cfg_param.target_version)
            wal_path = os.path.realpath(os.path.join(cfg_param.target_data_path, wal_dir_name))
            PostgreRestoreService.set_restore_command(pg_conf_file, wal_path)
            # 非时间点恢复清理恢复时间
            PostgreRestoreService.set_pitr_target_time(pg_conf_file, '', rollback=True)
            PostgreRestoreService.set_recovery_target_action(pg_conf_file, 'pause', rollback=True)
            LOGGER.info("Set recovery conf for postgresql v12 and above success, "
                        "restore task is not point-in-time recovery")
            return
        LOGGER.info(f"The recovery target time is {recovery_target_time} when setting recovery param.")
        PostgreRestoreService.set_restore_command(pg_conf_file, cfg_param.log_copy_path)
        major_ver = cfg_param.target_version.split(".")[0]
        # （1）PostgreSQL 12版本；（2）PostgreSQL 13及以上版本，日志副本中存在指定时间之后的事务
        if int(major_ver) < PgConst.MAJOR_VERSION_THIRTEEN \
                or PostgreCommonUtils.check_transaction_after_target_time(cfg_param.system_user,
                                                                          cfg_param.target_install_path,
                                                                          cfg_param.log_copy_path,
                                                                          recovery_target_time):
            PostgreRestoreService.set_pitr_target_time(pg_conf_file, recovery_target_time)
            # 按时间点恢复需要设置recovery_target_action值为promote，防止达到恢复点后暂停
            PostgreRestoreService.set_recovery_target_action(pg_conf_file, "promote")
        # PostgreSQL 13及以上版本，日志副本中不存在指定时间之后的事务，按增量备份处理
        else:
            PostgreRestoreService.set_pitr_target_time(pg_conf_file, '', rollback=True)
        LOGGER.info("Set recovery conf for postgresql v12 and above success.")

    @staticmethod
    def reset_recovery_config(job_dict, tgt_version, tgt_data_path, is_pit, role=RoleType.NONE_TYPE):
        LOGGER.info(f"Try to reset recovery config, role: {role}.")
        deploy_type = job_dict.get("targetEnv", {}).get("extendInfo", {}).get(
            "installDeployType", InstallDeployType.PGPOOL)
        if str(role) == RoleType.STANDBY.value:
            LOGGER.info("Current node is standby, no need reset recovery config.")
            return
        major_ver = tgt_version.split(".")[0]
        # PostgreSQL 12以下版本使用recovery.conf文件，无需复位恢复配置信息
        if int(major_ver) < PgConst.DATABASE_V12:
            LOGGER.info("Instance version is below 12, no need reset recovery config.")
            return
        # patroni集群的配置文件不同
        if deploy_type == InstallDeployType.PATRONI:
            pg_conf_file = os.path.realpath(os.path.join(tgt_data_path, PgConst.POSTGRESQL_BASE_CONF_FILE))
        else:
            config_file = job_dict.get("targetObject", {}).get("extendInfo", {}).get("configFile", "")
            pg_conf_file = config_file if config_file else os.path.realpath(
                os.path.join(tgt_data_path, PgConst.POSTGRESQL_CONF_FILE_NAME))
        if not is_pit:
            # 复位恢复命令时log路径没有用，所以设为“”
            PostgreRestoreService.set_restore_command(pg_conf_file, "", rollback=True)
            LOGGER.info("Reset full copy recovery config success.")
            return
        PostgreRestoreService.set_restore_command(pg_conf_file, "", rollback=True)
        # 复位恢复命令时恢复目标时间没有用，所以设为“”
        PostgreRestoreService.set_pitr_target_time(pg_conf_file, "", rollback=True)
        PostgreRestoreService.set_recovery_target_action(pg_conf_file, 'pause', rollback=True)
        LOGGER.info("Reset point-in-time recovery config success.")

    @staticmethod
    def set_recovery_conf_for_pg_below_v12(cfg_param: RestoreConfigParam, role=RoleType.NONE_TYPE):
        enable_root = PostgreCommonUtils.get_root_switch()
        LOGGER.info(f"Start setting recovery conf for postgresql below v12, role: {role}.")
        # 创建recovery.conf文件
        recovery_conf_sample_file = os.path.realpath(
            os.path.join(cfg_param.target_install_path, *PgConst.RECOVERY_CONF_SAMPLE_PATHS))
        recovery_conf_file = os.path.realpath(
            os.path.join(cfg_param.target_data_path, PgConst.RECOVERY_CONF_FILE_NAME))
        if not PostgreCommonUtils.check_special_characters(cfg_param.system_user):
            raise Exception("String contains special characters!")
        if not PostgreCommonUtils.check_os_user(cfg_param.system_user, cfg_param.target_install_path, enable_root)[0]:
            raise Exception("Os username os not exist!")
        if not enable_root:
            if not check_user_utils.check_path_owner(cfg_param.target_data_path, [cfg_param.system_user]):
                raise Exception("Data dir and os user is not matching!")
        if os.path.isfile(recovery_conf_sample_file):
            LOGGER.info(
                f"Creating recovery conf file. Copy file from {recovery_conf_sample_file} to {recovery_conf_file}")
            ret = exec_cp_cmd(recovery_conf_sample_file, recovery_conf_file, is_check_white_list=False)
            if not ret:
                raise Exception("Create recovery conf file failed")
            LOGGER.info(f"Create recovery conf file success.")
        else:
            LOGGER.warning("Recovery conf sample does not exist in database instance")
            sample_cfg_file = PostgreCommonUtils.get_local_recovery_conf_sample_by_ver(cfg_param.target_version)
            LOGGER.info(f"Creating recovery conf file. Copy file from {sample_cfg_file} to {recovery_conf_file}")
            ret = exec_cp_cmd(sample_cfg_file, recovery_conf_file, is_check_white_list=False)
            if not ret:
                raise Exception("Create recovery conf file failed")
            LOGGER.info(f"Create recovery conf file success.")

        # 使用插件中recovery.conf.sample样例，需要修改用户用户组、权限
        PostgreRestoreService.change_owner_and_mode_of_recovery_conf(cfg_param, recovery_conf_file)

        if str(role) == str(RoleType.STANDBY.value):
            PostgreRestoreService.set_recovery_cfg_for_standby_node(cfg_param, recovery_conf_file)
            return

        # 非时间点恢复，无需设置restore_command和recovery_target_time
        recovery_target_time = cfg_param.recovery_target_time
        if not recovery_target_time:
            wal_dir_name = PostgreCommonUtils.get_wal_dir_name_by_version(cfg_param.target_version)
            wal_path = os.path.realpath(os.path.join(cfg_param.target_data_path, wal_dir_name))
            PostgreRestoreService.set_restore_command(recovery_conf_file, wal_path)
            LOGGER.info("Set recovery conf for postgresql below v12 success, "
                        "restore task is not point-in-time recovery")
            return
        PostgreRestoreService.set_restore_command(recovery_conf_file, cfg_param.log_copy_path)
        PostgreRestoreService.set_pitr_target_time(recovery_conf_file, recovery_target_time)
        if PostgreCommonUtils.check_version_gt_nine_dot_four(cfg_param.target_version):
            # 按时间点恢复需要设置recovery_target_action值为promote，防止达到恢复点后暂停
            PostgreRestoreService.set_recovery_target_action(recovery_conf_file, "promote")
        LOGGER.info("Set recovery conf for postgresql below v12 success.")

    @staticmethod
    def set_recovery_cfg_for_standby_node(cfg_param, recovery_conf_file):
        LOGGER.info("Current node is standby node.")
        PostgreRestoreService.set_standby_mode_command(recovery_conf_file)
        PostgreRestoreService.set_primary_conn_info_command(recovery_conf_file, cfg_param)
        PostgreRestoreService.set_recovery_target_timeline_param(
            recovery_conf_file, PgConst.RECOVERY_TGT_TIMELINE_LATEST)
        tgt_data_path = cfg_param.target_data_path
        pg_conf_file = os.path.realpath(os.path.join(tgt_data_path, PgConst.POSTGRESQL_CONF_FILE_NAME))
        if not os.path.isfile(pg_conf_file):
            LOGGER.error("The postgresql.conf file does not exist in the database data directory")
            raise Exception("Postgresql configuration file does not exist in the database data directory")
        PostgreRestoreService.set_hot_standby_command(pg_conf_file)
        LOGGER.info("Set recovery conf for postgresql below v12 success, current node is standby node.")

    @staticmethod
    def change_owner_of_download_data(param_dict, input_path, is_data_dir=True):
        # 全量副本处理权限为“000”的文件；日志副本处理所有WAL日志文件
        PostgreCommonUtils.change_path_mode(input_path, 0o600, is_data_dir=is_data_dir)
        restore_mode = param_dict.get("job", {}).get("extendInfo", {}).get("restoreMode")
        LOGGER.info(f"Check restore mode: {restore_mode}.")
        os_user = PostgreRestoreService.parse_os_user(param_dict)
        uid, gid = PostgreCommonUtils.get_uid_gid_by_os_user(os_user)
        PostgreCommonUtils.change_path_owner(param_dict, input_path, uid, gid, is_data_dir=is_data_dir)

    @staticmethod
    def change_owner_and_mode_of_recovery_conf(cfg_param, recovery_conf_file):
        PostgreCommonUtils.check_path_islink(recovery_conf_file)
        # 修改用户和用户组
        uid, gid = PostgreCommonUtils.get_uid_gid_by_os_user(cfg_param.system_user)
        LOGGER.info(f"Change owner of the recovery conf file: {recovery_conf_file}, uid: {uid}, gid: {gid}.")
        os.lchown(recovery_conf_file, uid, gid)
        LOGGER.info(f"Change owner of the recovery conf file: {recovery_conf_file} success.")
        # recovery.conf文件默认权限644
        LOGGER.info(f"Change mode of the recovery conf file: {recovery_conf_file}.")
        change_path_permission(recovery_conf_file, mode=0o644)
        LOGGER.info(f"Change mode of the recovery conf file: {recovery_conf_file} success.")

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
            LOGGER.error(f"The copy mount path list: {copy_mount_paths} is incorrrect.")
            raise Exception("The copy mount path list is empty")
        LOGGER.info(f"Get copy mount path success, paths: {copy_mount_paths}, repository type: {repo_type}.")
        return copy_mount_paths

    @staticmethod
    def get_cache_mount_path(param_dict):
        copies = PostgreRestoreService.parse_copies(param_dict.get("job", {}))
        cache_path = PostgreRestoreService.get_copy_mount_paths(
            copies[0], RepositoryDataTypeEnum.CACHE_REPOSITORY.value)[0]
        if not check_utils.check_path_in_white_list(cache_path):
            raise Exception(f"The cache mount path is incorrect :{cache_path}")
        return cache_path

    @staticmethod
    def get_pgpool_install_path(param_dict):
        install_path = "/usr/local/pgpool"
        sub_objs = param_dict.get("job", {}).get("restoreSubObjects", [])
        local_ips = PostgreCommonUtils.get_local_ips()
        for obj in sub_objs:
            tmp_node_ip = obj.get("extendInfo", {}).get("endpoint", "")
            if tmp_node_ip in local_ips:
                install_path = obj.get("extendInfo", {}).get("pgpoolClientPath", "/usr/local/pgpool")
                break
        return install_path

    @staticmethod
    def get_db_install_and_data_path(param_dict):
        install_path, data_path, archive_dir = "", "", ""
        if PostgreRestoreService.is_restore_cluster(param_dict):
            sub_objs = param_dict.get("job", {}).get("restoreSubObjects", [])
            nodes = param_dict.get("job", {}).get("targetEnv", {}).get("nodes", [])
            local_ips = PostgreCommonUtils.get_local_ips()
            for obj in sub_objs:
                host_id = obj.get("extendInfo", {}).get("hostId", "")
                tmp_node_ip = PostgreCommonUtils.get_tmp_node_ip_from_nodes(nodes, host_id)
                if tmp_node_ip in local_ips:
                    install_path = obj.get("extendInfo", {}).get("clientPath", "")
                    data_path = obj.get("extendInfo", {}).get("dataDirectory", "")
                    data_real_path = os.path.realpath(data_path) if data_path else data_path
                    archive_dir = obj.get("extendInfo", {}).get("archiveDir", "")
                    break
        else:
            tgt_env_extend_info_dict = param_dict.get("job", {}).get("targetObject", {}).get("extendInfo", {})
            install_path = tgt_env_extend_info_dict.get("clientPath", "")
            data_path = tgt_env_extend_info_dict.get("dataDirectory", "")
            data_real_path = os.path.realpath(data_path) if data_path else data_path
            archive_dir = tgt_env_extend_info_dict.get("extendInfo", {}).get("archiveDir", "")
        data_path = os.path.join(install_path, "data") if not data_real_path and install_path else data_real_path
        return install_path, data_path, archive_dir

    @staticmethod
    def get_db_pg_wal_dir_real_path(param_dict, tgt_data_path):
        tgt_version = param_dict.get("job", {}).get("targetObject", {}).get("extendInfo", {}).get("version", "")
        if not os.path.isdir(tgt_data_path):
            LOGGER.error(f"Get postgresql database wal directory, target data path: {tgt_data_path} is invalid.")
            raise Exception("Target data path is invalid when getting postgresql database wal directory")
        if not tgt_version or len(tgt_version.split('.')) < 2:
            LOGGER.error(f"Get postgresql database wal directory, target version: {tgt_version} is invalid.")
            raise Exception("Target version is invalid when getting postgresql database wal directory")
        major_ver = tgt_version.split(".")[0]
        if int(major_ver) <= PgConst.MAJOR_VERSION_NINE:
            pg_wal_dir_list = PgConst.PG_XLOG_V9_AND_BELOW_PATHS
        else:
            pg_wal_dir_list = PgConst.PG_WAL_V10_AND_ABOVE_PATHS
        pg_wal_path = os.path.join(tgt_data_path, pg_wal_dir_list)
        pg_wal_real_path = os.path.realpath(os.path.join(tgt_data_path, pg_wal_dir_list))
        return pg_wal_path, pg_wal_real_path

    @staticmethod
    def get_db_port(param_dict):
        db_port = None
        if PostgreRestoreService.is_restore_cluster(param_dict):
            sub_objs = param_dict.get("job", {}).get("restoreSubObjects", [])
            local_ips = PostgreCommonUtils.get_local_ips()
            for obj in sub_objs:
                tmp_node_ip = obj.get("extendInfo", {}).get("endpoint", "")
                if tmp_node_ip in local_ips:
                    db_port = obj.get("extendInfo", {}).get("instancePort")
                    break
        else:
            tgt_env_extend_info_dict = param_dict.get("job", {}).get("targetObject", {}).get("extendInfo", {})
            db_port = tgt_env_extend_info_dict.get("instancePort")
        return int(db_port) if db_port else PgConst.DB_DEFAULT_PORT

    @staticmethod
    def get_pgpool_port(param_dict):
        return param_dict.get("job", {}).get("targetObject", {}).get("extendInfo", {}) \
            .get("instancePort", PgConst.PGPOOL_DEFAULT_PORT)

    @staticmethod
    def get_primary_ip_and_instance_port(param_dict):
        primary_ip, primary_inst_port = "", "5432"
        sub_objs = param_dict.get("job", {}).get("restoreSubObjects", [])
        for obj in sub_objs:
            node_role = obj.get("extendInfo", {}).get("role", "")
            if str(node_role) == str(RoleType.PRIMARY.value):
                primary_ip = obj.get("extendInfo", {}).get("serviceIp", "")
                primary_inst_port = obj.get("extendInfo", {}).get("instancePort", "")
                break
        return primary_ip, primary_inst_port

    @staticmethod
    def get_local_node_ip(param_dict):
        sub_objs = param_dict.get("job", {}).get("restoreSubObjects", [])
        host_ips = PostgreCommonUtils.get_local_ips()
        for obj in sub_objs:
            service_ip = obj.get("extendInfo", {}).get("serviceIp", "")
            if service_ip in host_ips:
                return service_ip
        return ''

    @staticmethod
    def write_progress_info_for_ex(task_name, param_dict, err_code=None, err_msg=None):
        if task_name not in RESTORE_TASK_PROGRESS_FILE_MAP:
            return
        cache_path = PostgreRestoreService.get_cache_mount_path(param_dict)
        progress_file = RESTORE_TASK_PROGRESS_FILE_MAP.get(task_name)
        if PostgreRestoreService.is_restore_cluster(param_dict):
            progress_file = RestoreAction.QUERY_RESTORE
        restore_progress = RestoreProgress(progress=NumberConst.HUNDRED)
        if err_code:
            restore_progress.err_code = err_code
        err_msg = err_msg if err_msg else f"Execute {task_name} task failed"
        restore_progress.message = err_msg if "failed" in err_msg else f"failed: {err_msg}"
        PostgreCommonUtils.write_progress_info(cache_path, progress_file, restore_progress)

    @staticmethod
    @exter_attack
    def report_progress(task_name, pid, job_id, param_dict, sub_job_id=None):
        LOGGER.info(f"[query]Start querying progress, task name: {task_name}, pid: {pid}, job id: {job_id}.")
        cache_path = PostgreRestoreService.get_cache_mount_path(param_dict)
        sub_job_details = SubJobDetails(taskId=job_id, subTaskId=sub_job_id, progress=0, logDetail=list(),
                                        taskStatus=SubJobStatusEnum.RUNNING.value)
        task_status = SubJobStatusEnum.RUNNING.value
        progress = NumberConst.ZERO
        err_code = None
        err_code_param = None
        if task_name in PgConst.RESTORE_PROGRESS_ACTIONS:
            process_cxt = PostgreCommonUtils.read_process_info(cache_path, task_name)
            progress, message, err_code, err_code_param = process_cxt.get("progress"), process_cxt.get("message", ""), \
                process_cxt.get("err_code"), process_cxt.get("err_code_param", [])
            LOGGER.debug(f"[query]Executing {task_name} task, progress: {progress}, message: {message}, "
                         f"error code: {err_code}, err_code_param: {err_code_param}.")
            if message == "completed":
                task_status = SubJobStatusEnum.COMPLETED.value
                # 当前恢复子任务结束，上报成功label
                if task_name == RestoreAction.QUERY_RESTORE:
                    job_detail = LogDetail(logInfo=ReportDBLabel.SUB_JOB_SUCCESS, logInfoParam=[sub_job_id],
                                           logTimestamp=int(time.time()), logDetail=err_code,
                                           logLevel=DBLogLevel.INFO.value)
                    sub_job_details.log_detail.append(job_detail)
            elif "failed" in message:
                task_status = SubJobStatusEnum.FAILED.value
        else:
            LOGGER.error(f"[query]Unsupported progress query task: {task_name}, pid: {pid}, job id: {job_id}.")
            task_status = SubJobStatusEnum.FAILED.value
            message = "Unsupported progress query task"
        sub_job_details.task_status = task_status
        sub_job_details.progress = progress
        if err_code:
            job_detail = LogDetail(logInfo=RESTORE_REPORT_LABEL_MAP.get(task_name, ReportDBLabel.RESTORE_SUB_FAILED),
                                   logInfoParam=[sub_job_id],
                                   logTimestamp=int(time.time()), logDetail=err_code, logLevel=DBLogLevel.ERROR.value,
                                   logDetailParam=err_code_param)
            sub_job_details.log_detail.append(job_detail)
        LOGGER.info(f"[query]Executing query progress task success, task name: {task_name}, pid: {pid}, "
                    f"job id: {job_id}, status: {task_status}, process: {progress}, message: {message}.")
        restore_svc_common.output_result_file(pid, sub_job_details.dict(by_alias=True))

    @staticmethod
    def record_task_result(pid, err_msg, code=None, err_code=None, body_err_params=None):
        action_result = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR.value if code is None else code,
                                     message=err_msg)
        if err_code:
            action_result.body_err = err_code
        if body_err_params:
            action_result.body_err_params = body_err_params
        restore_svc_common.output_result_file(pid, action_result.dict(by_alias=True))

    @staticmethod
    def get_sub_instance_role(param_dict):
        role = str(RoleType.NONE_TYPE.value)
        sub_objs = param_dict.get("job", {}).get("restoreSubObjects", [])
        local_ips = PostgreCommonUtils.get_local_ips()
        for obj in sub_objs:
            tmp_node_ip = obj.get("extendInfo", {}).get("endpoint", "")
            if tmp_node_ip in local_ips:
                role = obj.get("extendInfo", {}).get("role", str(RoleType.NONE_TYPE.value))
                break
        return role

    @staticmethod
    def get_pgpool_cfg_lines(pgpool_install_path):
        lines = []
        pgpool_conf_file = os.path.realpath(os.path.join(pgpool_install_path, "etc", "pgpool.conf"))
        if not os.path.isfile(pgpool_conf_file):
            LOGGER.warning(f"Pgpool conf file: {pgpool_conf_file} does not exist.")
            return lines
        with open(pgpool_conf_file, 'r') as f:
            lines = f.readlines()
        return lines

    @staticmethod
    def get_pgpool_log_dir(pgpool_install_path):
        pgpool_log_dir = None
        pgpool_cfg_lines = PostgreRestoreService.get_pgpool_cfg_lines(pgpool_install_path)
        for i in pgpool_cfg_lines:
            if str(i).strip().startswith("logdir = "):
                pgpool_log_dir = str(i.strip()).split("=")[1].strip()
                break
        return pgpool_log_dir

    @staticmethod
    def get_pgpool_pcp_port(pgpool_install_path):
        pcp_port = None
        pgpool_cfg_lines = PostgreRestoreService.get_pgpool_cfg_lines(pgpool_install_path)
        for i in pgpool_cfg_lines:
            if str(i).strip().startswith("pcp_port = "):
                pcp_port = str(i.strip()).split("=")[1].strip()
                break
        return pcp_port

    @staticmethod
    def get_pgpool_wd_port(pgpool_install_path):
        pcp_port = None
        pgpool_cfg_lines = PostgreRestoreService.get_pgpool_cfg_lines(pgpool_install_path)
        for i in pgpool_cfg_lines:
            if str(i).strip().startswith("wd_port = ") or str(i).strip().startswith("wd_port0 = "):
                pcp_port = str(i.strip()).split("=")[1].strip()
                break
        return pcp_port

    @staticmethod
    def get_pgpool_if_up_cmd(pgpool_install_path):
        if_up_cmd = ""
        pgpool_cfg_lines = PostgreRestoreService.get_pgpool_cfg_lines(pgpool_install_path)
        for i in pgpool_cfg_lines:
            if str(i).strip().startswith("if_up_cmd = "):
                if_up_cmd = str(i.strip()).split("=")[1].strip().strip("\'\"")
                break
        return if_up_cmd
