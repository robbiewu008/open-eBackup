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
import stat
import time

from common.common import check_command_injection, check_path_legal, execute_cmd_list, execute_cmd_list_out_to_file, \
    is_ubuntu
from common.const import RepositoryDataTypeEnum, SubJobStatusEnum, ParamConstant
from common.file_common import change_path_permission, copy_user_file_to_dest_path
from common.util.check_utils import is_port, check_file_path
from common.util.cmd_utils import cmd_format
from common.util.exec_utils import exec_mkdir_cmd, su_exec_rm_cmd, exec_overwrite_file, exec_cat_cmd, \
    exec_append_newline_file, exec_mount_cmd, exec_umount_cmd
from mysql import log
from mysql.src.common.constant import MySQLExecPower, MySQLJsonConstant, ExecCmdResult, \
    MySQLProgressFileType, MysqlParentPath, MariaDBConstant, MysqlBackupToolName, MysqlConfigFileKey
from mysql.src.common.error_code import MySQLErrorCode
from mysql.src.common.execute_cmd import exec_cmd_nowait, exec_rc_tool_cmd, safe_get_environ, \
    check_path_in_white_list, retry_cmd_list
from mysql.src.protect_mysql_base import MysqlBase
from mysql.src.protect_mysql_base_utils import check_tool_exist
# 定义生成即时挂载所需要的my.cnf时，需要屏蔽的字段
from mysql.src.utils.mysql_service_info_utils import MysqlServiceInfoUtil
from mysql.src.utils.mysql_utils import MysqlUtils

EXCLUDE_SET = {
    "innodb_log_checksum_algorithm",
    "innodb_fast_checksum",
    "innodb_log_block_size",
    "redo_log_version",
    "server_uuid",
    "master_key_id"
}

MARIADB_VERSION_5_EXCLUDE_SET = {
    "innodb_checksum_algorithm",
    "innodb_undo_tablespaces",
    "innodb_undo_directory",
    "innodb_log_checksum_algorithm",
    "innodb_fast_checksum",
    "innodb_log_block_size",
    "redo_log_version",
    "server_uuid",
    "master_key_id"
}

SLEEP_TIME = 5


def append_config(config_path, *params):
    for param in params:
        exec_append_newline_file(file_path=config_path, data=param)


def live_mount_process_exists(mount_job_id):
    cmd = [
        "ps -ef",
        "grep mysqld",
        cmd_format("grep '{}'", mount_job_id),
        "grep -v grep"
    ]
    log.info(f"grep cmd:{cmd}")
    return execute_cmd_list(cmd)


class MysqlLiveMount(MysqlBase):

    def __init__(self, p_id, job_id, sub_job_id, json_param):
        super().__init__(p_id, job_id, sub_job_id, json_param)
        self._data_path = ""
        self._cache_path = ""
        self._mysql_user = ""
        self._mysql_pwd = ""
        self._mysql_port = 0
        self._error_code = 0

    def get_mount_path_by_live_mount(self, repositories_type):
        if not self._json_param:
            return ""
        job_json = self._json_param.get(MySQLJsonConstant.JOB, {})
        if not job_json:
            return ""
        repositories_json = job_json.get(MySQLJsonConstant.COPY, [{}])[0]. \
            get(MySQLJsonConstant.REPORITTORIES, [])
        repositories_num = len(repositories_json)
        for i in range(repositories_num):
            if repositories_json[i].get(MySQLJsonConstant.REPORITORYTYPE, "") \
                    == repositories_type:
                path_array = repositories_json[i].get(MySQLJsonConstant.PATH, [])
                if not path_array:
                    return ""
                if not path_array[0] or not check_path_legal(path_array[0], MysqlParentPath.TMPOCEANPROTECT):
                    log.error(f"Get path not legal. type:{repositories_type}. ")
                    return ""
                return path_array[0]
        return ""

    def get_data_path_by_live_mount(self):
        if self._data_path:
            return
        self._data_path = self.get_mount_path_by_live_mount(RepositoryDataTypeEnum.DATA_REPOSITORY.value)

    def get_cache_path_by_live_mount(self):
        if self._cache_path:
            return
        self._cache_path = self.get_mount_path_by_live_mount(RepositoryDataTypeEnum.CACHE_REPOSITORY.value)

    def set_mysql_param_by_live_mount(self):
        try:
            if self._json_param:
                self._mysql_user = (
                    safe_get_environ(f"{MySQLJsonConstant.JOB}_{MySQLJsonConstant.TARGETOBJECT}"
                                     f"_{MySQLJsonConstant.AUTH}_{MySQLJsonConstant.AUTHKEY}_{self._p_id}"))
                self._mysql_pwd = f"{MySQLJsonConstant.JOB}_{MySQLJsonConstant.TARGETOBJECT}" \
                                  f"_{MySQLJsonConstant.AUTH}_{MySQLJsonConstant.AUTHPWD}_{self._p_id}"
                self._mysql_port = int(self._json_param.get(MySQLJsonConstant.JOB, {}). \
                                       get(MySQLJsonConstant.TARGETOBJECT, {}). \
                                       get(MySQLJsonConstant.AUTH, {}).get(MySQLJsonConstant.EXTENDINFO, {}). \
                                       get(MySQLJsonConstant.INSTANCEPORT, 0))
        except Exception as exception_str:
            log.error(f"Set mysql param failed:{str(exception_str)} \
                pid:{self._p_id} jobId{self._job_id}")
            return False
        return True

    def init_param(self):
        self.get_cache_path_by_live_mount()
        _ = self.set_mysql_param_by_live_mount()
        if not self._cache_path or not self._mysql_port:
            return False
        if not self._mysql_pwd or not self._mysql_user:
            return False
        if not is_port(self._mysql_port):
            log.error("Check live mount port failed.")
            return False
        return True

    def report_progress_live_mount(self):
        """
        上报备份进度
        :return:
        """
        return self.report_progress_comm_ex(self.get_cache_path_by_live_mount)

    def prepare_live_mount(self):
        """
        准备即时挂载，检查前置条件
        :return:
        """
        ret = MysqlServiceInfoUtil.check_mysql_is_running()
        if ret:
            self._error_code = MySQLErrorCode.CHECK_MYSQL_NOT_CLOSE
            log.error(f"Mysql is running. pid: %s jobId: %s", self._p_id, self._job_id)
            return False

        ret = check_tool_exist(MysqlBackupToolName.MYSQLD)
        if not ret:
            self._error_code = MySQLErrorCode.CHECK_MYSQLD_TOOL_FAILED
            log.error(f"Check mysqld failed. pid: %s jobId: %s", self._p_id, self._job_id)
            return False

        ret = check_tool_exist(MysqlBackupToolName.MYSQLADMIN)
        if not ret:
            self._error_code = MySQLErrorCode.CHECK_MYSQL_ADMIN_TOOL_FAILED
            log.error(f"Check mysqladmin failed. pid: %s jobId: %s", self._p_id, self._job_id)
            return False
        return True

    def get_bin_log_name(self, copy_path):
        binlog_file = os.path.join(copy_path, "xtrabackup_binlog_info")
        if not os.access(binlog_file, os.F_OK):
            log.error(f"Get binlog file failed. pid:{self._p_id} jobid:{self._job_id}")
            return False, ""
        ret, out = exec_cat_cmd(binlog_file)
        if not ret:
            log.error(f"Read binlog file failed. ret:{ret} pid:{self._p_id} jobId:{self._job_id}")
            return False, ""
        bin_log_name = (out.split("\n")[0]).split(".")[0]
        return True, bin_log_name

    def get_mysql_version_in_copy(self):
        """
        从备份的副本中，获取备份数据所对应的mysql的版本
        :return:
        """
        job_json = self._json_param.get(MySQLJsonConstant.JOB, {})
        protect_object = job_json.get(MySQLJsonConstant.COPY, [{}])[0].get(MySQLJsonConstant.PROTECTOBJECT, {})
        return protect_object.get(MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.VERSION, "")

    def exec_live_mount(self, copy_path):
        # 生成即时挂载需要使用的my.cnf
        # 校验copy_path中是否有特殊字符：
        process = None
        if check_command_injection(copy_path):
            log.error(f"Copy path injection. pid:{self._p_id} jobid:{self._job_id}")
            return False, process
        backup_my_cnf_path = os.path.join(copy_path, "backup-my.cnf")
        if not os.access(backup_my_cnf_path, os.F_OK):
            log.error(f"Backup my.cnf not exist. pid:{self._p_id} jobid:{self._job_id}")
            return False, process
        tmp_path = os.path.join(self._data_path, "tmp_live")
        if not os.path.exists(tmp_path) and not exec_mkdir_cmd(tmp_path):
            log.error(f"create file[{tmp_path}] failed.")
            return False, ""
        live_mount_cnf_path = os.path.join(tmp_path, f"{self._job_id}_my.cnf")

        if not check_file_path(backup_my_cnf_path):
            log.error("The input_file param(%s) is invalid.", backup_my_cnf_path)
            return False, ""

        if not os.path.isfile(backup_my_cnf_path):
            log.error("The input_file param(%s) not exist.", backup_my_cnf_path)
            return False, ""

        create_cnf_cmd = [f"cat {backup_my_cnf_path}"]
        version = self.get_mysql_version_in_copy()
        log.info(f"Start live mount mysql version: {version}")
        # mariadb 5.x的配置和mysql不同，部分参数没有
        if version and MariaDBConstant.MARIADB in version and version.startswith("5."):
            for exclude_str in MARIADB_VERSION_5_EXCLUDE_SET:
                create_cnf_cmd.append(f"grep -v {exclude_str}")
        else:
            for exclude_str in EXCLUDE_SET:
                create_cnf_cmd.append(f"grep -v {exclude_str}")
        ret, err_out = execute_cmd_list_out_to_file(create_cnf_cmd, live_mount_cnf_path)
        if ret != ExecCmdResult.SUCCESS:
            log.error(f"Create live mount my.cnf failed. ret:{ret} \
                err:{err_out} pid:{self._p_id} jobid:{self._job_id}")
            return False, process
        ret, big_log_name = self.get_bin_log_name(copy_path)
        if not ret:
            return False, process
        # 执行挂载,因为该子进程会一直运行 所以不判断返回值
        append_config(live_mount_cnf_path, "basedir=/usr/", f"log-bin={big_log_name}", f"port={self._mysql_port}")
        self.append_extra_config(live_mount_cnf_path)
        # 适配ubuntu 8.0.33
        mount_cmd = self.build_mount_cmd(live_mount_cnf_path, copy_path, tmp_path)
        log.info(f"Mount cmd: {mount_cmd}")
        process = exec_cmd_nowait(mount_cmd)
        return True, process

    def append_extra_config(self, live_mount_cnf_path):
        config_file = os.path.join(self._data_path, "backup.cnf")
        log.info(f"config_file:{config_file}")
        if not os.path.exists(config_file):
            return
        with open(config_file, "r", encoding='UTF-8') as f:
            for line in f.readlines():
                value = MysqlUtils.get_kv_by_keys(line, MysqlConfigFileKey.EXTRA_CONFIG)
                if value != "":
                    append_config(live_mount_cnf_path, value)
                else:
                    continue

    def build_mount_cmd(self, live_mount_cnf_path, copy_path, tmp_path):
        # 适配ubuntu 8.0.33
        if is_ubuntu():
            tmp_dir = f"/tmp/{self._job_id}"
            exec_mkdir_cmd(tmp_dir, is_check_white_list=False)
            new_data_path = os.path.join(tmp_dir, "data")
            exec_mkdir_cmd(new_data_path, is_check_white_list=False)
            exec_mount_cmd(copy_path, new_data_path)
            tmp_cnf = f"{tmp_dir}/my.cnf"
            append_config(live_mount_cnf_path, f"pid-file={tmp_dir}/mysql.pid",
                          f"socket={tmp_dir}/mysql.sock", f"datadir={new_data_path}",
                          f"log-error={tmp_dir}/error.log")
            copy_user_file_to_dest_path(live_mount_cnf_path, tmp_cnf)
            mount_cmd = cmd_format("mysqld --defaults-file={} --user=root",
                                   tmp_cnf)
        else:
            append_config(live_mount_cnf_path, f"pid-file={copy_path}/mysql.pid", f"socket={tmp_path}/mysql.sock",
                          f"log-error={tmp_path}/error.log", f"datadir={copy_path}")
            mount_cmd = cmd_format("mysqld --defaults-file={} --user=root",
                                   live_mount_cnf_path)
        return mount_cmd

    def live_mount_ex(self):
        # 先自己挂载目录，在执行即时挂载
        ret = self.init_param()
        if not ret:
            log.error(f"Init param failed. pid:{self._p_id} jobid:{self._job_id}")
            return False
        self.write_progress_file(SubJobStatusEnum.RUNNING.value, 5,
                                 MySQLProgressFileType.COMMON)
        ret = self.prepare_live_mount()
        if not ret:
            return False
        log.info(f"Init param success. pid:{self._p_id} jobid:{self._job_id}")
        ret = self.mount_local_path()
        if not ret:
            _ = self.unmount_local_path(self._job_id)
            log.error(f"Mount local path failed. pid:{self._p_id} jobid:{self._job_id}")
            return False
        log.info(f"Mount local path success. pid:{self._p_id} jobid:{self._job_id}")
        try:
            ret = self.live_mount()
        except Exception as exception_str:
            log.error(f"Fun live mount exception. pid:{self._p_id} jobId:{self._job_id},error:{exception_str}")
            ret = False
        if not ret:
            # 任务失败之后，需要卸载
            _ = self.unmount_local_path(self._job_id)
            return False
        log.info(f"Live Mount success. pid:{self._p_id} jobid:{self._job_id}")
        return True

    def live_mount(self):
        full_copy_path = self.get_full_copy_path()
        log.info(f"full_copy_path:{full_copy_path}")
        if not full_copy_path:
            log.error(f"Get full copy path failed. pid:{self._p_id} jobId:{self._job_id}")
            return False

        ret, process = self.exec_live_mount(full_copy_path)
        if not ret:
            log.error(f"Exec live mount failed. pid:{self._p_id} jobId:{self._job_id}")
            return False

        # 等待5s钟，判断一下拉起的Mysql进程是否存在，存在才返回成功
        time.sleep(SLEEP_TIME)
        cmd = [
            "ps -ef",
            "grep mysqld",
            cmd_format("grep '{}'", self._job_id),
            "grep -v grep"
        ]
        log.info(f"grep cmd:{cmd}")
        ret, output, _ = execute_cmd_list(cmd)
        if ret != ExecCmdResult.SUCCESS:
            log.error(f"Progress mysqld not exist. ret:{ret} pid:{self._p_id} jobId:{self._job_id}")
            self._error_code = MySQLErrorCode.EXEC_BACKUP_RECOVER_LIVEMOUNT_CMD_FAIL
            err_data = process.stderr.read()
            self.set_log_detail_params(["Livemount", err_data if err_data else "Failed to Start the MySQL Service"])
            return False
        log.info(f"Live mount success. pid:{self._p_id} jobId:{self._job_id}")
        return True

    def cancel_live_mount(self):
        log.info(f"start to cancel_live_mount, jobId:{self._job_id}")
        ret = self.init_param()
        if not ret:
            log.error(f"Init param failed. pid:{self._p_id} jobid:{self._job_id}")
            return False
        self.write_progress_file(SubJobStatusEnum.RUNNING.value, 5,
                                 MySQLProgressFileType.COMMON)
        ret, mount_job_id = self.get_mount_job_id()
        log.info(f"ret:{ret},mount_job_id:{mount_job_id}")
        if not ret or not mount_job_id:
            log.error(f"Get mount job id failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        result, std_out, std_err = live_mount_process_exists(mount_job_id)
        log.info(f"result:{result},std_out:{std_out},std_err:{std_err}")
        if result == ExecCmdResult.SUCCESS:
            ret, tmp_path = self.stop_live_mount_process(mount_job_id)
            if is_ubuntu():
                tmp_data_dir = os.path.join("/tmp", mount_job_id, "data")
                exec_umount_cmd(tmp_data_dir, "-l")
            self.clean_dir(tmp_path)
        # 清除cache仓
        self.clean_dir(self._cache_path)
        # 需要卸载本地目录
        ret = self.unmount_local_path(f"{mount_job_id}_live_mount")
        if not ret:
            return False
        log.info(f"Cancel live mount success. pid:{self._p_id} jobId:{self._job_id}")
        return True

    def stop_live_mount_process(self, mount_job_id):
        self._data_path = os.path.join(MySQLExecPower.MYSQL_LIVEMOUNT_PATH, f"{mount_job_id}_live_mount")
        if not is_ubuntu():
            tmp_path = os.path.join(self._data_path, "tmp_live")
        else:
            tmp_path = os.path.join("/tmp", f"{mount_job_id}_live_mount")
        log.info(f"tmp_path:{tmp_path}")
        full_copy_path = self.get_full_copy_path()
        if not full_copy_path:
            log.error(f"Get full copy path failed. pid:{self._p_id} jobId:{self._job_id}")
            return False, tmp_path
        # 适配ubuntu
        cmds = [
            cmd_format("mysqladmin shutdown -u{} -p", self._mysql_user),
            cmd_format("mysqladmin shutdown -u{} -p -h127.0.0.1", self._mysql_user),
            cmd_format("mysqladmin -S {}/mysql.sock shutdown -u{} -p ", tmp_path, self._mysql_user),
            cmd_format("mysqladmin -S {}/mysql.sock shutdown -u{} -p -h127.0.0.1", tmp_path, self._mysql_user),
        ]
        ret, output = retry_cmd_list(self._mysql_pwd, cmds)
        if not ret:
            log.warning(f"Stop mysqld failed with localhost. ret:{ret} pid:{self._p_id} jobId:{self._job_id}")
            MysqlUtils.kill_mysql_process()
            return True, tmp_path
        return True, tmp_path

    def get_mount_job_id(self):
        mount_job_id = ""
        try:
            if self._json_param:
                mount_job_id = self._json_param.get(MySQLJsonConstant.JOB, {}). \
                    get(MySQLJsonConstant.TARGETOBJECT, {}). \
                    get(MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.MOUNTJOBID, "")
        except Exception as exception_str:
            log.error(f"Get mount job id failed:{str(exception_str)} \
                pid:{self._p_id} jobId{self._job_id}")
            return False, mount_job_id
        return True, mount_job_id

    def get_data_repositorie(self):
        if not self._json_param:
            return {}
        job_json = self._json_param.get(MySQLJsonConstant.JOB, {})
        if not job_json:
            return {}
        repositories_json = job_json.get(MySQLJsonConstant.COPY, [{}])[0]. \
            get(MySQLJsonConstant.REPORITTORIES, [])
        repositories_num = len(repositories_json)
        for i in range(repositories_num):
            if repositories_json[i].get(MySQLJsonConstant.REPORITORYTYPE, "") \
                    == RepositoryDataTypeEnum.DATA_REPOSITORY.value:
                return repositories_json[i]
        return {}

    def prepare_path(self, path):
        if os.path.exists(path):
            log.info(f"Already exists path, pid:{self._p_id} jobId:{self._job_id}")
            return True
        if not exec_mkdir_cmd(path):
            log.error(f"Mkdir[{path}] failed. \
                    pid:{self._p_id} jobId:{self._job_id}")
            return False

        try:
            change_path_permission(path, mode=stat.S_IRWXU)
        except Exception as exception_str:
            log.error(f"Chmod failed.  \
                    pid:{self._p_id} jobId:{self._job_id},error:{exception_str}")
            return False
        return True

    def clear_path(self, path):
        if not os.path.exists(path):
            log.info(f"Path not exist. pid:{self._p_id} jobId:{self._job_id}")
            return True

        if not su_exec_rm_cmd(path):
            log.error(f"Rm tree failed. path:[{path}]. pid:{self._p_id} jobId:{self._job_id}")
            return False
        return True

    def unmount_local_path(self, mount_job_id):
        path = os.path.join(MySQLExecPower.MYSQL_LIVEMOUNT_PATH, mount_job_id)
        if not check_path_legal(path, MysqlParentPath.TMP):
            log.error(f"Path is unlawful. pid:{self._p_id} jobId:{self._job_id}")
            return False
        cmd = [
            "mount",
            cmd_format("grep {}", path)
        ]
        return_code, out_info, _ = execute_cmd_list(cmd)
        if return_code == ExecCmdResult.SUCCESS:
            # 调用卸载脚本
            data_repository = self.get_data_repositorie()
            if not data_repository:
                log.error(f"Get data respository failed. pid:{self._p_id} jobId:{self._job_id}")
                return False
            path_array = [path]
            data_repository[MySQLJsonConstant.PATH] = path_array
            ret = self.call_rc_tool_cmd("UnMountRepositoryByPlugin", data_repository)
            if not ret:
                log.error(f"Unmount failed. pid:{self._p_id} jobId:{self._job_id}")
                self._error_code = MySQLErrorCode.CANCEL_LIVE_MOUNT_FAILED
                return False
        else:
            log.info(f"Mount not exist. pid:{self._p_id} jobId:{self._job_id}")
        # 清理本地目录，即时失败也不影响人物结果
        if not check_path_in_white_list(path):
            log.error(f"Invalid path.")
            return False
        _ = self.clear_path(path)
        return True

    def mount_local_path(self):
        data_repository = self.get_data_repositorie()
        if not data_repository:
            log.error(f"Get data respository failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        path = os.path.join(MySQLExecPower.MYSQL_LIVEMOUNT_PATH, self._job_id)
        ret = self.prepare_path(path)
        if not ret:
            return False
        # 调用挂载脚本
        ret = self.mount_by_script(path, data_repository)
        if not ret:
            return False
        self._data_path = path
        log.info(f"mount_local_path success:{self._data_path}")
        return True

    def create_param_file(self, param_path, param_json):
        if not check_path_in_white_list(param_path):
            log.error(f"Invalid param_path.")
            return False
        if os.path.exists(param_path) and not su_exec_rm_cmd(param_path):
            log.error(f"Rm file[{param_path}] failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        log.info(f"Write:{param_json} to param_path. pid:{self._p_id} jobId:{self._job_id}")

        if not exec_overwrite_file(param_path, param_json):
            log.error(f"Create file failed. path:{param_path}. pid:{self._p_id} jobId:{self._job_id}")
            return False
        return True

    def mount_by_script(self, local_path, data_repository):
        # 调用挂载脚本执行挂载
        path = [local_path]
        data_repository[MySQLJsonConstant.PATH] = path
        remote_host_array = data_repository.get(MySQLJsonConstant.REMOTEHOST, [])
        data_repository[MySQLJsonConstant.REMOTEHOST] = []
        if not remote_host_array:
            log.error(f"Remote host array error. pid:{self._p_id} jobId:{self._job_id}")
            return False
        for remote_host in remote_host_array:
            data_repository[MySQLJsonConstant.REMOTEHOST].append(remote_host)
            ret = self.call_rc_tool_cmd("MountRepositoryByPlugin", data_repository)
            if ret:
                log.info(f"Rctool mount success. pid:{self._p_id} jobId:{self._job_id}")
                return True
        log.error(f"Rctool mount failed. pid:{self._p_id} jobId:{self._job_id}")
        return False

    def get_extend_info_param(self):
        """
        MySQL适配：FC需求 即时挂载、即时恢复都是插件调agent的接口进行挂载的，需要各插件适配下，将advanceParams值透传给agent，让agent判断怎么挂载
        @return: fibreChannel参数
        """
        if not self._json_param:
            return {}
        job_json = self._json_param.get(MySQLJsonConstant.JOB, {})
        if not job_json:
            return {}
        return job_json.get(MySQLJsonConstant.EXTENDINFO, {})

    def call_rc_tool_cmd(self, cmd, data_repository):
        json_str = {
            "user": MySQLExecPower.MYSQL_USER,
            "fileMode": MySQLExecPower.MYSQL_FILE_MODE
        }
        param_json = {
            "repository": [data_repository],
            "permission": json_str,
            "extendInfo": self.get_extend_info_param()
        }
        param_path = os.path.join(ParamConstant.PARAM_FILE_PATH, f"param_{self._job_id}")
        result_path = os.path.join(ParamConstant.RESULT_PATH, f"result_{self._job_id}")
        ret = self.create_param_file(param_path, param_json)
        if not ret:
            return False

        ret = exec_rc_tool_cmd(cmd, param_path, result_path)
        if not ret:
            log.warning(f"Exec rc tool failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        if not check_path_in_white_list(param_path):
            log.error(f"Invalid param_path.")
            return False
        if not check_path_in_white_list(result_path):
            log.error(f"Invalid result_path.")
            return False

        if not su_exec_rm_cmd(param_path):
            # 删除参数文件失败 不认为命令执行失败
            log.warning(f"Remove param path[{param_path}] failed. pid:{self._p_id} jobId:{self._job_id}")

        if not su_exec_rm_cmd(result_path):
            # 删除参数文件失败 不认为命令执行失败
            log.warning(f"Remove result path[{result_path}] failed. pid:{self._p_id} jobId:{self._job_id}")
        return True
