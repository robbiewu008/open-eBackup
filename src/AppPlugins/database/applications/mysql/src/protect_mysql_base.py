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

import json
import os
import signal
import time

from common.cleaner import clear
from common.common import execute_cmd, exter_attack, check_path_legal, execute_cmd_list, \
    execute_cmd_list_communicate, is_clone_file_system
from common.const import ParamConstant, SubJobStatusEnum, RepositoryDataTypeEnum, \
    JobData, DBLogLevel, BackupTypeEnum, CopyDataTypeEnum
from common.file_common import get_user_info, exec_lchown_dir_recursively
from common.job_const import ParamKeyConst
from common.parse_parafile import ParamFileUtil
from common.util.check_utils import is_ip_address, is_port
from common.util.cmd_utils import cmd_format
from common.util.exec_utils import exec_overwrite_file, exec_cat_cmd
from common.util.exec_utils import su_exec_rm_cmd
from common.util.kmc_utils import Kmc
from mysql import log
from mysql.src.common.constant import MySQLProgressFileType, MySQLType, ExecCmdResult, RestorePath, RestoreType, \
    MySQLStrConstant, MySQLJsonConstant, MySQLParamType, MySQLClusterType, MysqlBackupToolName, MysqlParentPath, \
    RoleType, MySQLCmdStr, MySQLRestoreStep, MysqlLabel, MysqlProgress, IPConstant, \
    MySQLPreTableLockStatus, XtrbackupErrStr, SystemServiceType, SystemConstant
from mysql.src.common.error_code import BodyErr, MySQLCode, MySQLErrorCode
from mysql.src.common.execute_cmd import exec_rc_tool_cmd, get_config_value_from_job_param, \
    get_config_value_from_instance, get_passwd, is_port_in_use
from mysql.src.common.execute_cmd import exec_sql, mysql_get_status_output, safe_get_environ, \
    check_path_in_white_list, mysql_backup_files, get_charset_from_instance
from mysql.src.common.parse_parafile import ReadFile, RestoreParseParam, ExecSQLParseParam
from mysql.src.protect_mysql_base_utils import MysqlBaseUtils, get_lock_ddl_param, check_tool_exist
from mysql.src.protect_mysql_restore_common import MysqlRestoreCommon
from mysql.src.service.backup.backup_func import support_parameter
from mysql.src.utils.common_func import SQLParam, exec_cmd_spawn
from mysql.src.utils.mysql_job_param_util import MysqlJobParamUtil
from mysql.src.utils.mysql_utils import MysqlUtils
from mysql.src.utils.restore_func import parse_xtrabackup_info


def check_copy_complete(path):
    """
    查询备份出来的文件夹是否完整
    :return:
    """
    check_point_file = os.path.join(path, "xtrabackup_checkpoints")
    if os.access(check_point_file, os.F_OK):
        conf = ReadFile.read_conf(check_point_file)
        if conf.get("last_lsn") == conf.get("flushed_lsn") and conf.get("last_lsn"):
            return True
    return False


class MysqlBase(MysqlBaseUtils):

    def __init__(self, p_id, job_id, sub_job_id, json_param):
        self._job_id = job_id
        self._sub_job_id = sub_job_id
        self._p_id = p_id
        self._json_param = json_param
        self._cache_path = ""
        self._log_path = ""
        self._data_path = ""
        self._meta_path = ""
        self._mysql_port = 0
        self._mysql_charset = get_config_value_from_job_param(json_param, "charset", SystemConstant.DEFAULT_CHARSET)
        self._mysql_user = ""
        self._mysql_pwd = ""
        self._mysql_ip = IPConstant.LOCALHOST
        self._report_progress_thread_start = True
        self._master_status = {"master_binlog_file": "", "master_binlog_pos": -1}
        self._last_copy_size = 0
        self._last_time = 0
        self._average_speed = 0
        self._restore_step = 0
        self._error_code = 0
        self._internal_error = ""
        self._error_arg_list = []
        self._lable_report = []
        self._flush_time = 0
        self._lock_name = ""
        self._lock_time = ""
        self._data_size = 0
        self._log_detail_params = []
        self._restore_comm = MysqlRestoreCommon(p_id, job_id, sub_job_id, json_param)
        self._exec_sql_param = ExecSQLParseParam(host_ip=self._mysql_ip, port=self._mysql_port, user=self._mysql_user,
                                                 passwd_env=self._mysql_pwd, charset=self._mysql_charset)
        self.my_cnf_path = get_config_value_from_job_param(json_param, key="myCnfPath", default_value="")

    def set_log_detail_params(self, log_detail_params):
        self._log_detail_params = log_detail_params

    def get_log_comm(self):
        return f"pid:{self._p_id} jobId:{self._job_id} sub_job_id:{self._sub_job_id}"

    def exec_xtrabackup_cmd(self, cmd_param_str, tool_name=MysqlBackupToolName.XTRBACKUP2, passwd=""):
        """
        执行备份工具命令
        会优先去环境变量中执行，找不到再搜索可执行文件的位置,返回能成功运行命令的第一个
        :return: 
        """
        if passwd and tool_name in [MysqlBackupToolName.XTRBACKUP2, MysqlBackupToolName.MYSQLDUMP,
                                    MysqlBackupToolName.MYSQL, MysqlBackupToolName.XTRBACKUP8]:
            passwd = safe_get_environ(passwd)
            # 需要输密码的命令
            exec_fun = exec_cmd_spawn
        else:
            exec_fun = mysql_get_status_output
        env_cmd_str = f"{tool_name} {cmd_param_str}"
        log.info(f"env_cmd_str:{env_cmd_str}")
        ret, output = exec_fun(env_cmd_str, passwd)
        if ret == int(ExecCmdResult.SUCCESS):
            # 成功不再打印输出
            return True, output
        log.error(f"output: {output}")
        # 兼容1.2 1.2是在备份完成后prepare的， 重复prepare会报错，但是可以忽略，不影响恢复
        if XtrbackupErrStr.DATABASE_COPY_IS_PREPARED in output:
            return True, output
        self.convert_xtrbackup_error(output, tool_name)
        self._restore_comm.deal_xtrabackup_error(output, tool_name)
        if ret != int(ExecCmdResult.UNKNOWN_CMD):
            return False, f"Exec {tool_name} failed"
        # 表示没找到,去搜索如果找到多个 默认使用成功的第一个
        find_cmd_str = f"find / -type f -name {tool_name}"
        ret, output_find, _ = execute_cmd(find_cmd_str)
        if ret == ExecCmdResult.SUCCESS:
            for line in output_find.split("\n"):
                if not line:
                    continue
                exec_cmd_str = f"{line} {cmd_param_str}"
                ret, output = exec_fun(exec_cmd_str, passwd)
                if ret == int(ExecCmdResult.SUCCESS):
                    return True, output
            return False, "err found"
        return False, ""

    def query_backup_process_alive_log(self):
        cmd_str = ["ps -ef", f"grep '{self._job_id}'", "grep python3 ", f"grep -v '{self._p_id}'"]
        ret, output, _ = execute_cmd_list(cmd_str)
        if ret == ExecCmdResult.SUCCESS and output:
            return True, f"{output}"
        return False, ""

    def query_backup_process_alive(self):
        backup_type = ParamFileUtil.parse_backup_type(self._json_param.get(MySQLJsonConstant.JOB, {}). \
                                                      get(MySQLJsonConstant.JOBPARAM, ""))
        if backup_type == BackupTypeEnum.LOG_BACKUP.value:
            ret, output = self.query_backup_process_alive_log()
            return ret, output
        tool_name = self.get_backup_tool_name()
        cmd_str_xtrabackup = [
            "ps -ef",
            cmd_format("grep '{}'", self._job_id),
            cmd_format("grep '{}'", tool_name),
            "grep -v grep"
        ]
        ret_xtrabackup, output_xtrabackup, _ = execute_cmd_list(cmd_str_xtrabackup)
        if ret_xtrabackup == ExecCmdResult.SUCCESS and output_xtrabackup:
            return True, f"{output_xtrabackup}"
        return False, ""

    def get_id_and_sub_id(self):
        return self._job_id, self._sub_job_id

    def get_json_param(self):
        return self._json_param

    def get_p_id(self):
        return self._p_id

    def get_channel_number(self):
        try:
            job_param_extend = self.get_json_param().get(MySQLJsonConstant.JOB, {}).get("extendInfo")
            channel_number = job_param_extend.get("channel_number")
            if channel_number:
                log.info(f"channel_number:{channel_number}")
                return int(channel_number)
            backup_task_sla_str = job_param_extend.get("backupTask_sla")
            backup_task_sla = json.loads(backup_task_sla_str)
            policy_list = backup_task_sla.get("policy_list", [])
            channel_number = int(policy_list[0].get("ext_parameters", {}).get("channel_number"))
            log.info(f"channel_number:{channel_number}")
            return channel_number
        except Exception as err:
            log.warning(f"get_channel_number error,{err}")
            return 1

    def get_lock_ddl_cmd(self):
        lock_ddl_param = get_lock_ddl_param()
        if lock_ddl_param == MySQLPreTableLockStatus.OFF:
            return ''
        log.warning("Backup in case of emergency!")
        # mariadb 不支持lock-ddl参数
        if MysqlBackupToolName.MARIADBBACKUP == self.get_backup_tool_name():
            return f"--lock-ddl-per-table={lock_ddl_param}"
        return f"--lock-ddl=0 --lock-ddl-per-table={lock_ddl_param}"

    def set_json_param(self, json_param):
        self._json_param = json_param

    def set_mysql_port(self, _mysql_port):
        self._mysql_port = _mysql_port

    def set_mysql_charset(self, _mysql_charset):
        self._mysql_charset = _mysql_charset

    def set_mysql_ip(self, _mysql_ip):
        self._mysql_ip = _mysql_ip

    def set_mysql_user(self, _mysql_user):
        self._mysql_user = _mysql_user

    def set_mysql_pwd(self, _mysql_pwd):
        self._mysql_pwd = _mysql_pwd

    def set_mysql_param(self):
        if self._json_param:
            app_type = self._json_param.get(MySQLJsonConstant.JOB, {}). \
                get(MySQLJsonConstant.PROTECTOBJECT, {}).get(MySQLJsonConstant.SUBTYPE, "")
            if app_type == MySQLType.SUBTYPECLUSTER:
                return self.set_mysql_param_cluster()
            else:
                return self.set_mysql_param_instance()
        log.error("set mysql param failed.")
        return False

    def generate_exec_sql_param(self):
        exec_sql_param = ExecSQLParseParam(host_ip=self._mysql_ip, port=self._mysql_port, user=self._mysql_user,
                                           passwd_env=self._mysql_pwd, charset=self._mysql_charset)
        return exec_sql_param

    def generate_sql_param(self) -> SQLParam:
        sql_param = SQLParam(host=self._mysql_ip, port=self._mysql_port, passwd=safe_get_environ(self._mysql_pwd),
                             user=self._mysql_user, charset=self._mysql_charset)
        return sql_param

    def set_mysql_param_instance(self):
        # 设置mysql相关参数
        try:
            if self._json_param:
                mysql_ip = self._json_param.get(MySQLJsonConstant.JOB, {}). \
                    get(MySQLJsonConstant.PROTECTOBJECT, {}). \
                    get(MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.INSTANCEIP, "")
                self._mysql_ip = mysql_ip if mysql_ip else IPConstant.LOCALHOST
                self._mysql_user = safe_get_environ(f"job_protectObject_auth_authKey_{self._p_id}")

                self._mysql_pwd = f"job_protectObject_auth_authPwd_{self._p_id}"
                self._mysql_port = int(self._json_param.get(MySQLJsonConstant.JOB, {}). \
                                       get(MySQLJsonConstant.PROTECTOBJECT, {}). \
                                       get(MySQLJsonConstant.AUTH, {}).get(MySQLJsonConstant.EXTENDINFO, {}). \
                                       get(MySQLJsonConstant.INSTANCEPORT, 0))
                self._mysql_charset = get_config_value_from_job_param(self._json_param, "charset",
                                                                      SystemConstant.DEFAULT_CHARSET)
        except Exception as exception_str:
            log.error(f"Set_mysql_param failed:{str(exception_str)} pid:{self._p_id} jobId{self._job_id}")
            return False
        if not self._mysql_user or not self._mysql_pwd:
            log.error(f"get env failed. pid:{self._p_id} jobId{self._job_id}")
            return False
        if not is_ip_address(self._mysql_ip) or not is_port(self._mysql_port):
            log.error(f"Check ip or port failed. pid:{self._p_id} jobId{self._job_id}")
            return False
        return True

    def get_sql_info_by_nodes(self, node_list, ips):
        node_list_num = len(node_list)
        for i in range(node_list_num):
            if node_list[i].get(MySQLJsonConstant.ENDPOINT, "") in ips:
                mysql_ip = node_list[i].get(MySQLJsonConstant.EXTENDINFO, {}). \
                    get(MySQLJsonConstant.INSTANCEIP, "")
                self._mysql_ip = mysql_ip if mysql_ip else IPConstant.LOCALHOST
                self._mysql_user = safe_get_environ(f"job_protectEnv_nodes_{i}_auth_authKey_{self._p_id}")
                self._mysql_pwd = f"job_protectEnv_nodes_{i}_auth_authPwd_{self._p_id}"
                self._mysql_port = int(node_list[i].get(MySQLJsonConstant.AUTH, {}). \
                                       get(MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.INSTANCEPORT, 0))
                self._mysql_charset = get_charset_from_instance(node_list[i])
                self.my_cnf_path = get_config_value_from_instance(node_list[i], "myCnfPath", "")
                log.info("get cluster info success.")
                return True
        log.error("get cluster info failed.")
        return False

    def set_mysql_param_cluster(self):
        # 设置mysql集群相关参数
        try:
            if self._json_param:
                ips = self.get_local_ips()
                node_list = self._json_param.get(MySQLJsonConstant.JOB, {}). \
                    get(MySQLJsonConstant.APPENV, {}).get(MySQLJsonConstant.NODES, [])
                ret = self.get_sql_info_by_nodes(node_list, ips)
                if not ret:
                    return False
        except Exception as exception_str:
            log.error(f"Set_mysql_param failed:{str(exception_str)} \
                pid:{self._p_id} jobId{self._job_id}")
            return False
        if not self._mysql_user or not self._mysql_pwd:
            log.error(f"get env failed. pid:{self._p_id} jobId{self._job_id}")
            return False
        return True

    def get_mount_path(self, repositories_type):
        if not self._json_param:
            return ""
        job_json = self._json_param.get(MySQLJsonConstant.JOB, {})
        if job_json:
            job_param_json = job_json.get(MySQLJsonConstant.JOBPARAM, {})
            if job_param_json:
                job_type = job_param_json.get(MySQLJsonConstant.BACKUPTYPE, 0)
                job_restore_type = True if job_type == 0 else False
        if not job_restore_type:
            path = self.get_backup_mount_path(repositories_type, job_json)
        else:
            path = self.get_restore_mount_path(repositories_type, job_json)
        # 要求路径需要tmp目录下
        if path and not check_path_legal(path, MysqlParentPath.TMP):
            log.error(f"Get path not legal. type:{repositories_type}. ")
            return ""
        return path

    def get_backup_mount_path(self, repositories_type, job_json):
        if job_json:
            repositories_json = job_json.get(MySQLJsonConstant.REPORITTORIES, [])
        else:
            repositories_json = self._json_param.get(MySQLJsonConstant.REPORITTORIES, [])
        repositories_num = len(repositories_json)
        for i in range(repositories_num):
            if repositories_json[i].get(MySQLJsonConstant.REPORITORYTYPE, "") \
                    == repositories_type:
                path = repositories_json[i].get(MySQLJsonConstant.PATH, [""])[0]
                return path
        return ""

    def get_restore_mount_path(self, repositories_type, job_json):
        result_path = ""
        if not job_json:
            return ""
        copies_json = job_json.get(MySQLJsonConstant.COPIES, [])
        if not copies_json:
            return ""
        # 适配UBC改动，取data仓时，取最后一个副本的path，因为日志恢复，原生格式UBC也会下发依赖的
        # 所有副本，但是只有最后一个副本会克隆
        for copy_json in copies_json:
            repositories_json = copy_json.get(MySQLJsonConstant.REPORITTORIES, [])
            for repo in repositories_json:
                if repo.get(MySQLJsonConstant.REPORITORYTYPE, "") == repositories_type and \
                        repo.get(MySQLJsonConstant.PATH, "") != "":
                    path = repo.get(MySQLJsonConstant.PATH, [""])[0]
                    result_path = path if path else result_path
        if not result_path:
            log.info(f"Path is empty. pid:{self._p_id} jobId{self._job_id}")
        return result_path

    def set_cache_path(self):
        # 从参数中获取cache仓的位置
        if self._cache_path:
            return
        self._cache_path = self.get_mount_path(RepositoryDataTypeEnum.CACHE_REPOSITORY.value)

    def set_log_path(self):
        # 从参数中获取log仓的位置
        if self._log_path:
            return
        self._log_path = self.get_mount_path(RepositoryDataTypeEnum.LOG_REPOSITORY.value)

    def set_data_path(self):
        # 从参数中获取数据仓的位置
        if self._data_path:
            return
        self._data_path = self.get_mount_path(RepositoryDataTypeEnum.DATA_REPOSITORY.value)

    def set_meta_path(self):
        if self._meta_path:
            return
        self._meta_path = self.get_mount_path(RepositoryDataTypeEnum.META_REPOSITORY.value)

    def get_full_cache_path(self):
        return self.get_mount_path(RepositoryDataTypeEnum.CACHE_REPOSITORY.value)

    def output_action_result(self, code, body_err, message):
        self.output_action_result_ex(self._p_id, code, body_err, message)

    def output_other_result(self, json_str):
        """
        将json_str写入到结果文件,供框架读取
        :return: 
        """
        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{self._p_id}")
        exec_overwrite_file(file_path, json_str)

    def delete_copy(self):
        backup_type = self._json_param.get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.JOBPARAM, {}) \
            .get(MySQLJsonConstant.BACKUPTYPE, "")
        if backup_type == BackupTypeEnum.LOG_BACKUP.value:
            self.set_log_path()
            ret, copy_path = self.get_copy_path()
        else:
            self.set_data_path()
            ret, copy_path = self.get_copy_path()
        log.info(f"delete{copy_path}")
        if not check_path_in_white_list(copy_path):
            log.error(f"Invalid copy_path.")
            return
        if ret:
            self.check_and_del_target_dir(copy_path)

    def exec_rear_job(self):
        """
        执行后置任务
        :return: 
        """
        progress_type = self.get_progress_file_type()
        self.write_progress_file(SubJobStatusEnum.RUNNING.value, MysqlProgress.FIVE,
                                 progress_type)
        backup_job_ret = self._json_param.get(MySQLJsonConstant.BACKJOBRESLUT, 0)
        log.info(f"Backup job ret:{backup_job_ret}")
        # 若备份任务执行不成功，需要删除生成的目录
        if backup_job_ret != 0:
            log.info("Backup exec failed. delete copy.")
            self.delete_copy()
        # 清除cache目录
        self.set_cache_path()
        if self._cache_path:
            self.clean_dir(self._cache_path)
        self.write_progress_file(SubJobStatusEnum.COMPLETED.value, MysqlProgress.ONE_HUNDRED,
                                 progress_type)
        return True

    def kill_backup_progress(self):
        """
        kill 备份进程
        :return: 
        """
        ret, output = self.query_backup_process_alive()
        if ret:
            for line in output.splitlines():
                pid = int(line.split()[1])
                os.kill(pid, signal.SIGKILL)
                log.info(f"os.kill {pid} pid:{self._p_id} jobId{self._job_id}")

    def abort_job(self):
        self.write_progress_file(SubJobStatusEnum.ABORTING.value, 5, MySQLProgressFileType.ABORT)
        self.kill_backup_progress()
        ret, _ = self.query_backup_process_alive()
        if ret:
            self.write_progress_file(SubJobStatusEnum.ABORTED_FAILED.value, 100, MySQLProgressFileType.ABORT)
        else:
            self.write_progress_file(SubJobStatusEnum.ABORTED.value, 100, MySQLProgressFileType.ABORT)

    def write_copy_info(self, date_time, copy_path):
        self.set_cache_path()
        if not self._cache_path:
            return False
        copy_info_path = os.path.join(self._cache_path, f"copy_info_{self._job_id}")
        copy_json = self._json_param.get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.COPY, [{}])[0]
        copy_json[MySQLJsonConstant.TIMESTAMP] = date_time
        try:
            host_sn = self.get_host_sn()
        except Exception as exception_str:
            # 获取hostsn失败不影响当次备份
            log.warning(f"Get host sn failed. pid:{self._p_id} jobId{self._job_id},error:{exception_str}")
            host_sn = ""
        xtrabackup_info_path = os.path.join(copy_path, 'xtrabackup_info')
        cluster_type = MysqlUtils.get_cluster_type(self.get_json_param())
        if os.path.exists(xtrabackup_info_path) and cluster_type != MySQLClusterType.EAPP:
            xtrabackup_info = parse_xtrabackup_info(xtrabackup_info_path)
            backup_bin_log = str(xtrabackup_info.get("binlog_filename"))
        else:
            backup_bin_log = ""
        copy_json[MySQLJsonConstant.EXTENDINFO] = {
            MySQLJsonConstant.BACKUPTIME: date_time,
            MySQLJsonConstant.BACKUPHOSTSN: host_sn,
            MySQLJsonConstant.BINLOG_NAMES: backup_bin_log,
            MySQLJsonConstant.FIRSTFULLBACKUPTIME: self.get_first_full_backup_time(date_time)
        }
        exec_overwrite_file(copy_info_path, copy_json)
        return True

    def query_copy_info(self):
        """
        上报副本信息
        :return:
        """
        self.set_cache_path()
        if not self._cache_path:
            return False
        copy_info_path = os.path.join(self._cache_path, f"copy_info_{self._job_id}")
        if not os.path.exists(copy_info_path):
            log.error(f"copy info path not exist.")
            return False
        json_copy = ReadFile.read_param_file(copy_info_path)
        self.output_other_result(json_copy)
        return True

    def get_info_by_slave_status(self, idx):
        exec_sql_param = self.generate_exec_sql_param()
        exec_sql_param.sql_str = "show slave status"
        ret, output = exec_sql(exec_sql_param)
        if not ret:
            log.error(f"Exec sql failed. sql:show salve status \
                            ret:{ret}  pid:{self._p_id} jobId{self._job_id}")
            return set()
        master_ip = set()
        for line in output:
            master_ip.add(line[idx])
        return master_ip

    def check_cluster_sync_status(self):
        io_errors = self.get_info_by_slave_status(35)
        for io_error in io_errors:
            if io_error.strip():
                log.error("Data sync error. error:%s", io_error)
                return False, io_error
        return True, ''

    def check_mysql_and_backtools_version(self):
        """
        查询备份工具是否存在
        :return: 
        """
        tool_name = self.get_backup_tool_name()
        return check_tool_exist(tool_name)

    def write_progress_file(self, task_status, progress, progress_type, speed=0):
        """
        将进度写入进度文件供框架读取
        :return:
        """
        json_dict = self.create_progress_json(task_status, progress, True, speed)
        log.info(f"write file.task_status:{task_status} progress:{progress} pid:{self._p_id} jobId{self._job_id}")
        self.write_progress_to_file(json_dict, progress_type)

    def write_progress_file_ex(self, task_status, progress, progress_type):
        json_dict = self.create_progress_json(task_status, progress, False)
        log.info(f"write file.task_status:{task_status} progress:{progress} pid:{self._p_id} jobId{self._job_id}")
        self.write_progress_to_file(json_dict, progress_type)

    def write_progress_to_file(self, json_dict, progress_type):
        if not self._cache_path:
            self.set_cache_path()
        file_path = os.path.join(self._cache_path, progress_type)
        exec_overwrite_file(file_path, json_dict)

    def create_log_detail(self, task_status):
        log_detail_array = []
        label = MysqlLabel.label_dict.get(JobData.CMD, {}).get(task_status, "")
        log_level = DBLogLevel.INFO.value
        if not self.check_label(label):
            log.info("Check label failed.")
            return log_detail_array
        if task_status == SubJobStatusEnum.FAILED.value:
            log_level = DBLogLevel.ERROR.value
        if label and label not in self._lable_report:
            log_detail = {}
            log_detail[MySQLJsonConstant.LOGINFO] = label
            log_detail[MySQLJsonConstant.LOGINFOPARAM] = [self._sub_job_id]
            if label == MysqlLabel.BACKUP_LOCK_TABLE_DETAIL:
                if not self._lock_time:
                    return []
                log_detail[MySQLJsonConstant.LOGINFO] = MysqlLabel.BACKUP_LOCK_TABLE_DETAIL
                log_level = DBLogLevel.WARN.value
                log_detail[MySQLJsonConstant.LOGINFOPARAM] = [self._lock_name, self._lock_time]
            log_detail[MySQLJsonConstant.LOGLEVEL] = log_level
            if self._error_code:
                log_detail[MySQLJsonConstant.LOGDETAIL] = self._error_code
                log_detail[MySQLJsonConstant.LOGDETAILPARAM] = self._log_detail_params
            log_detail_array.append(log_detail)
            # 记录一下Lable上报的次数，目前都是一次，后续扩展
            self._lable_report.append(label)
        return log_detail_array

    def check_label(self, label):
        backup_type = self.get_json_param().get(MySQLJsonConstant.JOB, {}). \
            get(MySQLJsonConstant.JOBPARAM, {}).get(MySQLJsonConstant.BACKUPTYPE, "")
        # 日志备份不涉及锁表，锁库情况，返回false
        if label == MysqlLabel.BACKUP_LOCK_TABLE_DETAIL and backup_type == BackupTypeEnum.LOG_BACKUP:
            return False
        return True

    def write_log_detail_to_file(self, log_detail):
        if not self._cache_path:
            self.set_cache_path()
        try:
            file_path = os.path.join(self._cache_path, f"{MySQLJsonConstant.LOGDETAIL}_{self.get_job_tag()}")
            exec_overwrite_file(file_path, log_detail)
        except Exception as exception_str:
            # 写label失败不影响任务结果
            log.info(f"Write log detail failed. pid:{self._p_id} jobId{self._job_id},error:{exception_str}")
        log.info(f"Write log detail success. pid:{self._p_id} jobId{self._job_id}")

    def read_log_detail_from_file(self):
        file_path_comm = os.path.join(self._cache_path, f"{MySQLJsonConstant.LOGDETAIL}_{self.get_job_tag()}")
        if not check_path_legal(file_path_comm, MysqlParentPath.TMP):
            return []
        if not os.access(file_path_comm, os.F_OK):
            return []
        try:
            log_detail = ReadFile.read_param_file(file_path_comm)
        except Exception as exception_str:
            log.error(f"Read log detail file filed. pid:{self._p_id} jobId{self._job_id},error:{exception_str}")
            return []
        su_exec_rm_cmd(file_path_comm)
        return log_detail

    def get_job_tag(self):
        return self._sub_job_id if self._sub_job_id else self._job_id

    def create_progress_json(self, task_status, progress, need_sub_task_id, speed=0):
        """
        构造写入文件的Json体
        :return:
        """
        json_dict = {}
        log_detail_array = self.create_log_detail(task_status)
        if log_detail_array:
            self.write_log_detail_to_file(log_detail_array)
        extend_info = {MySQLJsonConstant.TIMESTAMP: int(time.time())}
        if self._average_speed != 0 and task_status == SubJobStatusEnum.COMPLETED:
            extend_info[MySQLJsonConstant.AVERAGE_SPEED] = self._average_speed
        json_dict[MySQLJsonConstant.TASKID] = self._job_id
        if need_sub_task_id:
            json_dict[MySQLJsonConstant.SUBTASKID] = self._sub_job_id
        else:
            json_dict[MySQLJsonConstant.SUBTASKID] = ""
        json_dict[MySQLJsonConstant.TASKSTATUS] = task_status
        json_dict[MySQLJsonConstant.PROGRESS] = int(progress)
        json_dict[MySQLJsonConstant.DATASIZE] = self._data_size
        json_dict[MySQLJsonConstant.SPEED] = speed
        json_dict[MySQLJsonConstant.EXTENDINFO] = extend_info
        json_dict[MySQLJsonConstant.LOGDETAIL] = log_detail_array
        return json_dict

    def check_progress_file_available(self, timestamp, progress):
        """
        查询进度文件是否可用，如果时间戳和进度都没有发生变化，认为进度文件不可用
        :return:
        """
        progress_time_file = os.path.join(self._cache_path, self.get_progress_time_file_name())
        if not os.access(progress_time_file, os.F_OK):
            log.info("Progress time file not exist.")
            return True
        time_json = ReadFile.read_param_file(progress_time_file)
        file_time_stamp = time_json.get(MySQLJsonConstant.TIMESTAMP, 0)
        file_progress = time_json.get(MySQLJsonConstant.PROGRESS, 0)
        if timestamp > file_time_stamp or file_progress != progress:
            log.info("Progress file is available.")
            return True
        return False

    def write_progress_timestamp(self, timestamp, progress):
        """
        更新记录时间戳和进度的文件
        :return:
        """
        json_dict = {
            MySQLJsonConstant.TIMESTAMP: timestamp,
            MySQLJsonConstant.PROGRESS: progress
        }
        progress_time_file = os.path.join(self._cache_path, self.get_progress_time_file_name())
        exec_overwrite_file(progress_time_file, json_dict)

    def get_progress_time_file_name(self):
        if MysqlUtils.get_cluster_type(self._json_param) == MySQLClusterType.EAPP:
            return self.get_host_sn()
        return MySQLProgressFileType.PROGRESSTIME

    def report_progress_comm(self):
        return self.report_progress_comm_ex(self.set_cache_path)

    def report_progress_comm_ex(self, get_cache_func):
        """
        上报备份进度
        :return: 
        """
        get_cache_func()
        if not self._cache_path:
            log.error("Get cache path filed.")
            return False
        progress_type = self.get_progress_file_type()
        file_path_comm = os.path.join(self._cache_path, progress_type)
        if not os.access(file_path_comm, os.F_OK):
            log.error("Report progress_comm failed.")
            return False
        json_str = ReadFile.read_param_file(file_path_comm)
        timestamp = json_str.get(MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.TIMESTAMP, 0)
        progress = json_str.get(MySQLJsonConstant.PROGRESS, 0)
        json_str[MySQLJsonConstant.LOGDETAIL] = self.read_log_detail_from_file()
        ret = self.check_progress_file_available(timestamp, progress)
        if not ret:
            log.error("Progress file is not available.")
            return False
        self.output_other_result(json_str)
        log.info(f"Report progress_comm success. progress:{progress} timestamp:{timestamp}")
        self.write_progress_timestamp(timestamp, progress)
        return True

    def get_progress_file_type(self):
        cluster_type = MysqlUtils.get_cluster_type(self._json_param)
        if cluster_type == MySQLClusterType.EAPP and self._sub_job_id:
            return self._sub_job_id
        return MySQLProgressFileType.COMMON

    def report_progress_abort(self):
        self.set_cache_path()
        if not self._cache_path:
            return False
        file_path_abort = os.path.join(self._cache_path, MySQLProgressFileType.ABORT)

        if os.access(file_path_abort, os.F_OK):
            json_str = ReadFile.read_param_file(file_path_abort)
            self.output_other_result(json_str)
            return True
        else:
            return False

    def find_data_dir(self):
        exec_sql_param = self.generate_exec_sql_param()
        exec_sql_param.sql_str = "show variables like '%datadir%'"
        ret, output = exec_sql(exec_sql_param)
        if not ret:
            log.error(f"Exec_sql failed. sql:show variables like '%datadir%' \
                ret:{ret} pid:{self._p_id} jobId{self._job_id}")
            return ""
        for i in output:
            if i[0] != 'datadir':
                continue
            try:
                return i[1]
            except Exception as exception_str:
                log.warning(f"Get datadir exception . reason:{exception_str} \
                    pid:{self._p_id} jobId:{self._job_id}")
            return ""
        return ""

    def get_mysql_version(self):
        """
        根据variables，获取mysql版本信息
        :return:
        """
        exec_sql_param = self.generate_exec_sql_param()
        exec_sql_param.sql_str = "show variables like 'version'"
        ret, output = exec_sql(exec_sql_param)
        if not ret:
            log.error(f"Exec_sql failed. sql:show variables like 'version' \
                        ret:{ret} pid:{self._p_id} jobId{self._job_id}")
            return ""
        version = ""
        for item in output:
            if item[0] == 'version':
                version = item[1]
                continue
        return version

    def get_self_average_speed(self):
        return self._average_speed

    def report_backup_progress_thread(self):
        log.info(f"Report thread begin. pid:{self._p_id} jobId:{self._job_id}")
        while self._report_progress_thread_start:
            time.sleep(5)
            ret, progress = self.calc_progress()
            if not ret:
                continue
            self.write_progress_file(SubJobStatusEnum.RUNNING.value, progress, self.get_progress_file_type())
        log.info(f"Report thread end. pid:{self._p_id} jobId:{self._job_id}")

    def prepare_sub_job(self, target_path):
        """
        判断备份子任务的前置条件
        :return: 
        """
        old_size = 0
        size = 0
        start_time = 0
        if not os.access(target_path, os.F_OK):
            return False
        # 判断进程是否运行
        while True:
            ret, _ = self.query_backup_process_alive()
            if not ret:
                break
            size = self.get_dir_size(target_path)
            if size != old_size:
                start_time = time.clock()
            # 2分钟目标目录没有发生变化，认为备份进程假死kill掉重做
            elif time.clock() - start_time >= 2 * 60 * 1000:
                log.error(f"Backup progress timeout. pid:{self._p_id} jobId{self._job_id}")
                self.kill_backup_progress()
                continue
            old_size = size
            time.sleep(0.05)
        # 判断目标副本是否完整
        ret = check_copy_complete(target_path)
        if ret:
            log.info(f"Backup copy is complete. pid:{self._p_id} jobId{self._job_id}")
            return True
        if not check_path_in_white_list(target_path):
            log.error(f"Invalid target_path.")
            return False
        self.check_and_del_target_dir(target_path)
        return False

    def get_copy_path(self):
        backup_type = self._json_param.get(MySQLJsonConstant.JOB, {}). \
            get(MySQLJsonConstant.JOBPARAM, {}).get(MySQLJsonConstant.BACKUPTYPE, "")
        copy_path = os.path.join(self._data_path, f"mysql_{self._job_id}_{backup_type}")
        return True, copy_path

    def set_backup_all_param(self):
        if not self._json_param:
            return False, ""
        self.set_data_path()
        if not self._data_path:
            return False, ""
        ret, copy_path = self.get_copy_path()
        log.info(f"Get copy path success. pid:{self._p_id} jobId{self._job_id}")
        ret = self.set_mysql_param()
        if not ret:
            return False, ""
        return True, copy_path

    def get_backup_database(self):
        """
        获取数据库备份的数据库名
        """
        if not self._json_param:
            return False, ""
        app_type = self._json_param.get(MySQLJsonConstant().JOB, {}). \
            get(MySQLJsonConstant.PROTECTOBJECT, {}).get(MySQLJsonConstant.SUBTYPE, "")
        if app_type == MySQLType.SUBTYPE:
            database_name = self._json_param.get(MySQLJsonConstant().JOB, {}). \
                get(MySQLJsonConstant.PROTECTOBJECT, {}).get(MySQLJsonConstant.NAME, "")
            return True, database_name
        return False, ""

    def set_mysql_restore_param(self):
        try:
            if self._json_param:
                mysql_user, mysql_pwd = self.get_user_pwd_by_restore()
                self._mysql_user = safe_get_environ(mysql_user)
                self._mysql_pwd = mysql_pwd
                ret, self._mysql_port, self._mysql_ip = self.get_port_and_ip_by_restore()
                self._mysql_charset = get_config_value_from_job_param(self._json_param, "charset",
                                                                      SystemConstant.DEFAULT_CHARSET)
                self.my_cnf_path = get_config_value_from_job_param(self._json_param, "myCnfPath",
                                                                   "")
                if not ret:
                    return False
        except Exception as exception_str:
            log.error(f"Set mysql restore param failed:{str(exception_str)} pid:{self._p_id} jobId{self._job_id}")
            return False
        if not self._mysql_user or not self._mysql_pwd:
            log.error(f"Get env failed. pid:{self._p_id} jobId{self._job_id}")
            return False
        return True

    def parse_restore_log_parameter(self, job_json):
        extend_info_json = job_json.get(MySQLJsonConstant.EXTENDINFO, {})
        if not extend_info_json:
            log.error(f"Get extend info failed. pid:{self._p_id} jobId{self._job_id}")
            return False, ""
        restore_time_stamp = extend_info_json.get(MySQLJsonConstant.RESTORETIMESTAMP, "")
        if not restore_time_stamp:
            log.error(f"Get restore time stamp failed. pid:{self._p_id} jobId{self._job_id}")
            return False, ""
        log.debug(f"Get restore time stamp： {restore_time_stamp}")
        return True, restore_time_stamp

    def check_restore_type(self):
        restore_type = RestoreType.EMPTY
        ret, restore_parse_param = self.parse_restore_parameters()
        if not ret:
            return False, restore_type, restore_parse_param.restore_log

        if restore_parse_param.sub_type == MySQLParamType.INSTANCE:
            restore_type = RestoreType.SINGLE_INSTANCE
        elif restore_parse_param.sub_type == MySQLParamType.DATABASE:
            if not restore_parse_param.cluster_type:
                restore_type = RestoreType.SINGLE_DB
            else:
                ret, restore_type = \
                    self.parse_db_restore_type(restore_parse_param.cluster_type, restore_parse_param.role_type)
                if not ret:
                    return False, restore_type, restore_parse_param.restore_log
        elif restore_parse_param.sub_type == MySQLParamType.CLUSTER:
            ret, restore_type = \
                self.parse_instance_restore_type(restore_parse_param.cluster_type, restore_parse_param.role_type)
            if not ret:
                return False, restore_type, restore_parse_param.restore_log
        else:
            log.error(f"Get sub type: {restore_parse_param.sub_type} is wrong.")
            return False, restore_type, restore_parse_param.restore_log
        log.debug(f"Get restore type:{restore_type}  log restore:{restore_parse_param.restore_log}")
        return True, restore_type, restore_parse_param.restore_log

    def check_database_with_the_same_name_exist(self):
        ret, copy_path = self.get_restore_copy_path()
        if not ret:
            log.error(f"Get copy path failed. pid:{self._p_id} jobId:{self._job_id}")
            return True
        # 获取备份副本中数据库名
        ret, copy_database_name = MysqlRestoreCommon.get_copy_database_name(copy_path)
        if not ret:
            log.error(f"Get copy database name failed. pid:{self._p_id} jobId:{self._job_id}")
            return True
        # 获取下发参数的数据库名，原位置和备份副本同名，新位置即为选择的数据库名
        ret, choose_database_name = self.get_restore_database()
        if not ret:
            log.error(f"Get choose database name failed. pid:{self._p_id} jobId:{self._job_id}")
            return True
        # 获取当前数据库目录
        ret, mysql_data_dir = self.get_mysql_storage_path()
        if not ret:
            log.error(f"Get mysql storage path failed. pid:{self._p_id} jobId:{self._job_id}")
            return True
        # 获取数据库目录下的数据库名
        dir_names = []
        for sub_dir in os.listdir(mysql_data_dir):
            sub_dir_path = os.path.join(mysql_data_dir, sub_dir)
            if os.path.isfile(sub_dir_path):
                continue
            dir_names.append(sub_dir)
        mysql_restore_common = MysqlRestoreCommon(self._p_id, self._job_id, self._sub_job_id, self._json_param)
        # 获取重命名数据库名
        rename_database_name = mysql_restore_common.parse_new_database_name()
        # 未重命名
        if not rename_database_name:
            if mysql_restore_common.original_location_and_recoverable(copy_database_name, choose_database_name,
                                                                      dir_names):
                return False
            self._error_code = MySQLErrorCode.CHECK_NEW_POS_DATABASE_WITH_THE_SAME_NAME_EXIST
        # 重命名
        else:
            if mysql_restore_common.rename_restore(choose_database_name, copy_database_name, rename_database_name,
                                                   dir_names):
                return False
            self._error_code = MySQLErrorCode.CHECK_DATABASE_WITH_THE_SAME_NAME_EXIST
        log.error(f"Database with the same name exist. pid:{self._p_id} jobId{self._job_id}")
        return True

    def check_mysql_cluster_is_stop(self):
        ret, restore_type, log_restore = self.check_restore_type()
        if not ret:
            log.error(f"Mysql get restore type failed. pid:{self._p_id} jobId{self._job_id}")
            return False
        if restore_type == RestoreType.PXC_DB_CLUSTER_NODE or restore_type == RestoreType.PXC_INSTANCE_CLUSTER_NODE:
            if self.check_mysql_bootstrap_service_run_status():
                self._error_code = MySQLErrorCode.CHECK_MYSQL_NOT_CLOSE
                log.error(f"Mysql bootstrap service running. pid:{self._p_id} jobId{self._job_id}")
                return False
        elif restore_type in [RestoreType.EAPP_INSTANCE, RestoreType.PXC_INSTANCE_COMMON_NODE,
                              RestoreType.PXC_DB_COMMON_NODE]:
            if MysqlRestoreCommon.check_mysql_can_not_restore():
                self._error_code = MySQLErrorCode.CHECK_MYSQL_NOT_CLOSE
                log.error(f"Mysql running. pid:{self._p_id} jobId{self._job_id}")
                return False
        else:
            job_json = self._json_param.get(MySQLJsonConstant.JOB, {})
            _, port, _ = self.get_port_and_ip_from_target_env(job_json)
            log.info(f"port:{port}")
            if is_port_in_use(port):
                self._error_code = MySQLErrorCode.CHECK_MYSQL_NOT_CLOSE
                log.error(f"Mysql running. pid:{self._p_id} jobId{self._job_id}")
                return False
        return True

    def exec_restore_pre_job(self):
        if self._sub_job_id:
            log.debug(f"Get self._sub_job_id: {self._sub_job_id}")
            progress_type = self._sub_job_id
        else:
            log.debug(f"Get self._job_id: {self._job_id}")
            progress_type = self._job_id
        self.write_progress_file(SubJobStatusEnum.RUNNING.value, 5, progress_type)
        ret = self.set_mysql_restore_param()
        if not ret:
            return False
        # 检查数据库是否停用
        if not self.check_mysql_cluster_is_stop():
            return False
        # 检查备份工具与数据库版本是否匹配
        ret = self.check_mysql_and_backtools_version()
        if not ret:
            return False
        # 检查是否安装日志备份工具
        cmd_str = "mysqlbinlog --version"
        ret, read, output = execute_cmd(cmd_str)
        if ret == ExecCmdResult.UNKNOWN_CMD:
            log.error(f"No mysqlbinlog found. pid:{self._p_id} jobId{self._job_id}")
            return False
        mysql_restore_common = MysqlRestoreCommon(self._p_id, self._job_id, self._sub_job_id, self._json_param)
        if mysql_restore_common.get_restore_type_is_database() and self.check_database_with_the_same_name_exist():
            return False
        return True

    def operate_chown(self, mysql_data_dir, main_process=True):
        if self._restore_step >= MySQLRestoreStep.CHOWN:
            log.debug(f"Step {MySQLRestoreStep.CHOWN} has done. pid:{self._p_id} jobId:{self._job_id}")
            return True
        user_name = MysqlUtils.get_data_path_user_name(self.my_cnf_path)
        log.info(f"user_name:{user_name}")
        user_group, user_info = get_user_info(user_name)
        ret = exec_lchown_dir_recursively(mysql_data_dir, user_name, user_group)
        if not ret:
            log.error(f"Exec chown mysql failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        if main_process:
            self.write_restore_step_info(MySQLRestoreStep.CHOWN)
        return True

    def get_restore_copy_path(self):
        copy_path = ""
        ret, data_path = self.set_restore_all_param()
        if not ret:
            log.error(f"Exec get cache path failed. pid:{self._p_id} jobId:{self._job_id}")
            return False, copy_path
        for path in os.listdir(self._data_path):
            new_path = os.path.join(self._data_path, path)
            if os.path.isdir(new_path) and MySQLStrConstant.MYSQL in new_path \
                    and new_path.endswith(f"_{BackupTypeEnum.FULL_BACKUP.value}"):
                copy_path = f"{new_path}"
                break
        log.debug(f"get restore copy path success,copy_path:{copy_path}")
        return True, copy_path

    def roll_back_restore_data(self, mysql_storage_dir, mysql_storage_old_dir):
        if os.path.exists(mysql_storage_old_dir) and mysql_storage_old_dir.is_dir() and not os.path.islink(
                mysql_storage_old_dir):
            self.check_and_del_target_dir(mysql_storage_dir)
            os.rename(mysql_storage_old_dir, mysql_storage_dir)
            log.debug(f"Mysql storage old dir roll back to origin success. pid:{self._p_id} jobId:{self._job_id}")
        else:
            if os.path.islink(mysql_storage_dir):
                log.warning(f"Path has link.")
                return
            for dirs_files in os.listdir(mysql_storage_dir):
                temp_dirs_files = os.path.join(mysql_storage_dir, dirs_files)
                if os.path.exists(temp_dirs_files) and ".old" in temp_dirs_files:
                    os.rename(temp_dirs_files, temp_dirs_files.strip(".old"))
            log.debug(f"Mysql storage old files roll back to origin success. pid:{self._p_id} jobId:{self._job_id}")

    @exter_attack
    def exec_restore_post(self):
        if self._sub_job_id:
            progress_type = self._sub_job_id
        else:
            progress_type = self._job_id
        self.write_progress_file(SubJobStatusEnum.RUNNING.value, 5, progress_type)
        ret, mysql_storage_dir = self.get_mysql_storage_path()
        mysql_storage_old_dir = mysql_storage_dir + RestorePath.DOTOLD
        restore_post_ret = int(self._json_param.get(MySQLJsonConstant.RESTOREJOBRESULT, 0))
        log.debug(f"Restore job ret:{restore_post_ret}")
        # 若恢复任务执行不成功，需要回滚数据
        if restore_post_ret != 0 and mysql_storage_dir:
            self.roll_back_restore_data(mysql_storage_dir, mysql_storage_old_dir)
        else:
            # 删除原数据库改名后的mysql.old目录，以及mysql中.old结尾的文件
            if ret:
                self.check_and_del_target_dir(mysql_storage_old_dir)
                self.del_dot_old_dir_or_file(mysql_storage_dir)

        self.write_progress_file(SubJobStatusEnum.COMPLETED.value, 100, progress_type)
        return True

    def del_restore_cache_dir(self):
        # 清除cache目录
        self.set_cache_path()
        if self._cache_path:
            self.clean_dir(self._cache_path)
        if not is_clone_file_system(self.get_json_param()):
            self.clean_dir(os.path.join("/tmp", self._job_id))

    def kill_restore_progress(self):
        ret, output = self.query_restore_process_alive()
        if not ret:
            return
        for line in output.splitlines():
            if len(line.split()) > 1:
                pid = int(line.split()[1])
                os.kill(pid, signal.SIGKILL)
                log.info(f"Kill {pid} pid:{self._p_id} jobId{self._job_id}")

    def abort_restore_job(self):
        if self._sub_job_id:
            progress_type = self._sub_job_id
        else:
            progress_type = self._job_id
        self.write_progress_file(SubJobStatusEnum.ABORTING.value, 5, progress_type)
        self.kill_restore_progress()
        ret, _ = self.query_restore_process_alive()
        if ret:
            self.write_progress_file(SubJobStatusEnum.ABORTED_FAILED.value, 100, progress_type)
        else:
            self.write_progress_file(SubJobStatusEnum.ABORTED.value, 100, progress_type)

    def set_restore_all_param(self):
        if not self._json_param:
            return False, ""
        self.set_data_path()
        if not self._data_path:
            return False, ""
        ret = self.set_mysql_restore_param()
        if not ret:
            return False, ""
        return True, self._data_path

    def get_restore_database(self):
        if not self._json_param:
            log.error("Get json param failed.")
            return False, ""
        job_json = self._json_param.get(MySQLJsonConstant.JOB, {})
        if not job_json:
            log.error("Get job json failed.")
            return False, ""
        target_object_json = job_json.get(MySQLJsonConstant.TARGETOBJECT, {})
        if not target_object_json:
            log.error("Get target object json failed.")
            return False, ""
        restore_param = target_object_json.get(MySQLJsonConstant.TYPE, "")
        if not restore_param:
            log.error("Get restore param json failed.")
            return False, ""
        if restore_param == MySQLType.TYPE:
            database_name = target_object_json.get(MySQLJsonConstant.NAME, "")
            log.info(f"Get database name: {database_name}")
            return True, database_name
        log.error("Get restore param json not equal Database.")
        return False, ""

    def get_target_env_extend_value_by_key(self, key):
        """
        根据指定的key从nodes中找到对应节点的值
        :return:
        """
        value = ""
        nodes_json = self._json_param.get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.TARGETENV, {}).get(
            MySQLJsonConstant.NODES, [])
        if not nodes_json:
            log.error(f"Get nodes json failed. pid:{self._p_id} jobId:{self._job_id}")
            return False, ""
        local_ips = self.get_local_ips()
        if not local_ips:
            log.error(f"Get local ips failed. pid:{self._p_id} jobId:{self._job_id}")
            return False, ""
        for element in nodes_json:
            ip_record = element.get(MySQLJsonConstant.ENDPOINT, "")
            if not ip_record:
                log.error(f"Get target env nodes ip record failed. pid:{self._p_id} jobId:{self._job_id}")
                return False, ""
            if ip_record not in local_ips:
                continue
            nodes_extend_info = element.get(MySQLJsonConstant.EXTENDINFO, {})
            if not nodes_extend_info:
                continue
            value = nodes_extend_info.get(key, "")
            if not value:
                continue
            break
        if not value:
            log.error(f"Get {key} value failed. pid:{self._p_id} jobId:{self._job_id}")
            return False, value
        return True, value

    def get_mysql_log_index_path(self):
        """
        获取当前节点mysql实例的所在路径
        :return:
        """
        ret, log_bin_index_path = self.get_target_env_extend_value_by_key(MySQLJsonConstant.LOG_BIN_INDEX_PATH)
        if not ret:
            log.error(f"Get mysql log bin index path failed. pid:{self._p_id} jobId:{self._job_id}")
            return False, ""
        log_bin_index_path_dir = log_bin_index_path.rstrip("/")
        log.debug(f"Get mysql log bin index path from cluster. pid:{self._p_id} jobId:{self._job_id}")
        return True, log_bin_index_path_dir

    def get_mysql_service_name(self):
        """
        获取mysql服务的名称，区分是mariadb、mysql、mysqld等
        :return:
        """
        ret, service_name = self.get_target_env_extend_value_by_key(MySQLJsonConstant.SERVICE_NAME)
        if not ret:
            log.error(f"Get mysql service name failed. pid:{self._p_id} jobId:{self._job_id}")
            return False, ""
        log.debug(f"Get mysql service name({service_name}) success. pid:{self._p_id} jobId:{self._job_id}")
        return True, service_name

    def get_mysql_system_service_type(self):
        """
        获取mysql系统服务注册的方式，区分是service还是systemctl。
        如果没有找到，则考虑升级场景，默认为systemctl
        :return:
        """
        ret, register_type = self.get_target_env_extend_value_by_key(MySQLJsonConstant.SYSTEM_SERVICE_TYPE_KEY)
        if not ret:
            log.error(f"Get mysql service register type failed. pid:{self._p_id} jobId:{self._job_id}")
            return True, SystemServiceType.SYSTEMCTL
        return True, register_type

    def get_mysql_storage_path(self):
        """
        获取当前节点mysql实例的所在路径
        :return:
        """
        ret, mysql_storage_dir = self.get_target_env_extend_value_by_key(MySQLJsonConstant.DATA_DIR)
        if not ret:
            log.error(f"Get mysql storage failed. pid:{self._p_id} jobId:{self._job_id}")
            return False, ""
        storage_dir = mysql_storage_dir.rstrip("/")
        log.debug(f"Get mysql storage dir from cluster. pid:{self._p_id} jobId:{self._job_id}")
        return True, storage_dir

    def get_full_copy_path(self):
        """
        获取依赖的全量副本路径
        :return 
        """
        full_copy_path = ""
        for path in os.listdir(self._data_path):
            new_path = os.path.join(self._data_path, path)
            if os.path.isdir(new_path) and MySQLStrConstant.MYSQL in new_path \
                    and new_path.endswith(f"_{BackupTypeEnum.FULL_BACKUP.value}"):
                full_copy_path = new_path
                break
        return full_copy_path

    def is_live_mount_job(self):
        job_type = self._json_param.get(MySQLJsonConstant.APPLICTION, {}). \
            get(MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.JOBTYPE, "")
        if job_type == MySQLJsonConstant.LIVEMOUNT:
            return True
        return False

    def get_port_and_ip_by_restore(self):
        job_json = self._json_param.get(MySQLJsonConstant.JOB, {})
        if not job_json:
            log.error("Get job json failed.")
            return False, "", ""
        ret, port, instance_ip = self.get_port_and_ip_from_target_env(job_json)
        if ret:
            return True, port, instance_ip
        ret, port, instance_ip = self.get_port_and_ip_from_target_object(job_json)
        if ret:
            return True, port, instance_ip
        log.error("Get port failed.")
        return False, "", ""

    def get_user_pwd_by_restore(self):
        job_json = self._json_param.get(MySQLJsonConstant.JOB, {})
        if not job_json:
            log.error("Get job json failed.")
            return "", ""
        ret, restore_parse_param = self.parse_restore_parameters()
        if not ret:
            return "", ""

        if restore_parse_param.sub_type == MySQLParamType.INSTANCE:
            mysql_user, mysql_pwd = self.get_single_user_pwd_from_restore(job_json)
            return mysql_user, mysql_pwd
        elif restore_parse_param.sub_type == MySQLParamType.DATABASE:
            if not restore_parse_param.cluster_type:
                mysql_user, mysql_pwd = self.get_single_user_pwd_from_restore(job_json)
                return mysql_user, mysql_pwd
            else:
                mysql_user, mysql_pwd = self.get_cluster_user_pwd_from_restore(job_json)
                return mysql_user, mysql_pwd
        elif restore_parse_param.sub_type == MySQLParamType.CLUSTER:
            mysql_user, mysql_pwd = self.get_cluster_user_pwd_from_restore(job_json)
            return mysql_user, mysql_pwd
        else:
            log.error(f"sub type: {restore_parse_param.sub_type} is wrong.")
            return "", ""

    def get_cluster_user_pwd_from_restore(self, job_json):
        target_env_json = job_json.get(MySQLJsonConstant.TARGETENV, {})
        if not target_env_json:
            log.error("Get target env json failed.")
            return "", ""
        nodes_json = target_env_json.get(MySQLJsonConstant.NODES, [])
        if not nodes_json:
            log.error("Get nodes json failed.")
            return "", ""
        local_ips = self.get_local_ips()
        if not local_ips:
            log.error("Get local ips failed.")
            return "", ""
        node_list_num = len(nodes_json)
        for i in range(node_list_num):
            ip_record = nodes_json[i].get(MySQLJsonConstant.ENDPOINT, "")
            if not ip_record:
                log.error("Get target env nodes ip record failed.")
                continue
            if ip_record not in local_ips:
                log.debug(f"{ip_record} not in local ips.")
                continue
            mysql_user = f"job_targetEnv_nodes_{i}_auth_authKey_{self._p_id}"
            mysql_pwd = f"job_targetEnv_nodes_{i}_auth_authPwd_{self._p_id}"
            log.debug(f"Get cluster user and pwd.")
            return mysql_user, mysql_pwd
        return "", ""

    def get_single_user_pwd_from_restore(self, job_json):
        mysql_user = ""
        mysql_pwd = ""
        try:
            if job_json:
                mysql_user = f"job_targetObject_auth_authKey_{self._p_id}"
                mysql_pwd = f"job_targetObject_auth_authPwd_{self._p_id}"
        except Exception as exception_str:
            log.error(f"Set mysql restore param failed:{str(exception_str)} pid:{self._p_id} jobId{self._job_id}")
            return "", ""
        log.debug(f"Get single user and pwd.")
        return mysql_user, mysql_pwd

    def parse_restore_role_type(self, job_json):
        target_env_json = job_json.get(MySQLJsonConstant.TARGETENV, {})
        if not target_env_json:
            log.error(f"Get target env json failed. pid:{self._p_id} jobId{self._job_id}")
            return False, ""
        nodes_json = target_env_json.get(MySQLJsonConstant.NODES, [])
        if not nodes_json:
            log.info(f"Get nodes json failed. pid:{self._p_id} jobId{self._job_id}")
            return True, ""
        local_ips = self.get_local_ips()
        if not local_ips:
            log.error(f"Get local ips failed. pid:{self._p_id} jobId{self._job_id}")
            return False, ""
        role_type = ""
        for element in nodes_json:
            ip_record = element.get(MySQLJsonConstant.ENDPOINT, "")
            if not ip_record:
                log.error("Get target env nodes ip record failed.")
                return False, role_type
            nodes_extend_info = element.get(MySQLJsonConstant.EXTENDINFO, {})
            if not nodes_extend_info:
                continue
            role = nodes_extend_info.get(MySQLJsonConstant.ROLE, "")
            if not role:
                continue
            if int(role) != 1:
                continue
            if ip_record in local_ips:
                role_type = RoleType.ACTIVE_NODE
            else:
                role_type = RoleType.STANDBY_NODE
            break
        log.debug(f"Get role type: {role_type}")
        return True, role_type

    def parse_restore_parameters(self):
        job_json = self._json_param.get(MySQLJsonConstant.JOB, {})
        if not job_json:
            log.error("Get job json failed.")
            restore_parse_param = RestoreParseParam(False, "", "", "")
            return False, restore_parse_param

        restore_log, restore_time_stamp = self.parse_restore_log_parameter(job_json)

        ret, sub_type, cluster_type = self.parse_restore_subtype_and_cluster_type(job_json)
        if not ret:
            restore_parse_param = RestoreParseParam(restore_log, sub_type, cluster_type, "")
            return False, restore_parse_param

        ret, role_type = self.parse_restore_role_type(job_json)
        restore_parse_param = RestoreParseParam(restore_log, sub_type, cluster_type, role_type)
        if not ret:
            return False, restore_parse_param
        return True, restore_parse_param

    def report_restore_progress_comm(self):
        """
        上报恢复进度
        :return:
        """
        self.set_cache_path()
        if not self._cache_path:
            log.error("Get cache path by restore filed.")
            return False
        if self._sub_job_id:
            progress_type = self._sub_job_id
        else:
            progress_type = self._job_id
        file_path_comm = os.path.join(self._cache_path, progress_type)
        if os.access(file_path_comm, os.F_OK):
            json_str = ReadFile.read_param_file(file_path_comm)
            json_str[MySQLJsonConstant.LOGDETAIL] = self.read_log_detail_from_file()
            self.output_other_result(json_str)
            log.info(f"Report restore progress comm success. pid:{self._p_id} jobId{self._job_id}")
            return True
        else:
            log.error(f"Report restore progress comm failed. pid:{self._p_id} jobId{self._job_id}")
            return False

    def get_sub_job_type(self):
        sub_job_json = self._json_param.get(MySQLJsonConstant.SUBJOB, {})
        if not sub_job_json:
            log.error(f"Get sub job json failed. pid:{self._p_id} jobId{self._job_id}")
            return ""

        job_name = sub_job_json.get(MySQLJsonConstant.JOBNAME, "")
        if not job_name:
            log.error(f"Get job name failed. pid:{self._p_id} jobId{self._job_id}")
            return ""
        log.debug(f"Job name: {job_name} pid:{self._p_id} jobId{self._job_id}")
        return job_name

    def cmd_execution_exception(self, cmd, job_id="", sub_job_id=""):
        backup_cmd_with_progress = [
            MySQLCmdStr.BACKUP_PER, MySQLCmdStr.BACKUP, MySQLCmdStr.BACKUP_POST,
            MySQLCmdStr.LIVE_MOUNT, MySQLCmdStr.CANCEL_LIVE_MOUNT
        ]
        restore_cmd_with_progress = [MySQLCmdStr.RESTORE_PRE, MySQLCmdStr.RESTORE, MySQLCmdStr.RESTORE_POST]
        cmd_not_process = [MySQLCmdStr.PROGRESS_COMM, MySQLCmdStr.RESTORE_GEN_SUB]
        log.info(f"Deal exception cmd： {cmd} pid:{self._p_id} jobId{self._job_id}")
        if cmd in backup_cmd_with_progress:
            self.write_progress_file(SubJobStatusEnum.FAILED.value, 100, MySQLProgressFileType.COMMON)
            return
        if cmd in restore_cmd_with_progress:
            if sub_job_id:
                self.write_progress_file(SubJobStatusEnum.FAILED.value, 100, sub_job_id)
            else:
                self.write_progress_file(SubJobStatusEnum.FAILED.value, 100, job_id)
            return
        if cmd not in cmd_not_process:
            self.output_action_result(MySQLCode.FAILED.value, BodyErr.ERR_PLUGIN_CANNOT_BACKUP.value, "")
            return

    def read_restore_step_info(self):
        """
        从cache仓读取恢复步骤
        :return:
        """
        self.set_cache_path()
        if not self._cache_path:
            return 0
        host_sn = self.get_host_sn()
        if not host_sn:
            log.error(f"Get host sn failed. pid:{self._p_id} jobId:{self._job_id}")
            return 0
        copy_info_path = os.path.join(self._cache_path, f"restore_step{host_sn}")
        if not os.path.exists(copy_info_path):
            log.error(f"File not exists. pid:{self._p_id} jobId:{self._job_id}")
            return 0
        json_dict = ReadFile.read_param_file(copy_info_path)
        if not json_dict:
            log.error(f"Read file failed. pid:{self._p_id} jobId:{self._job_id}")
            return 0
        if "restore_step" not in json_dict.keys():
            return 0
        return json_dict["restore_step"]

    def write_restore_step_info(self, restore_step_value):
        """
        将恢复步骤写到cache仓中
        :return:
        """
        self.set_cache_path()
        if not self._cache_path:
            log.error(f"Write restore step failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        host_sn = self.get_host_sn()
        if not host_sn:
            log.error(f"Get host sn failed, write restore step failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        copy_info_path = os.path.join(self._cache_path, f"restore_step{host_sn}")
        restore_step = {"restore_step": restore_step_value}
        log.debug(f"Write restore step: {restore_step}")
        exec_overwrite_file(copy_info_path, restore_step)
        return True

    def get_backup_tool_name(self):
        tool_name = MysqlBackupToolName.XTRBACKUP2
        object_json = self._json_param.get(MySQLJsonConstant.JOB, {}). \
            get(MySQLJsonConstant.PROTECTOBJECT, {})
        if not object_json:
            object_json = self._json_param.get(MySQLJsonConstant.JOB, {}). \
                get(MySQLJsonConstant.TARGETOBJECT, {})
        if not object_json:
            log.error(f"Get object json failed. pid:{self._p_id} jobId:{self._job_id}")
            return tool_name
        version = object_json.get(MySQLJsonConstant.EXTENDINFO, {}). \
            get(MySQLJsonConstant.VERSION, "")
        log.info(f"Get version({version}) success. pid:{self._p_id} jobId:{self._job_id}")
        if version and MySQLStrConstant.MARIADB in version:
            # 针对mariadb 5.x系列，没有自带mariabackup，备份工具使用xtrabackup2
            if version.startswith("5."):
                return MysqlBackupToolName.XTRBACKUP2
            # 其它mariadb 系列，则使用mariadb自带的mariabackup
            return MysqlBackupToolName.MARIADBBACKUP
        if version and version.startswith("8.0"):
            return MysqlBackupToolName.XTRBACKUP8
        return tool_name

    def backup_table_struct(self, database_name, copy_path):
        sql_file = os.path.join(copy_path, f"{database_name}.sql")

        set_gtid_purged = "--set-gtid-purged=OFF"
        # mariadb里的mysqldump命令，没有set-gtid-purged这个参数
        version = MysqlJobParamUtil.get_mysql_version(self._json_param, self._job_id, self._p_id)
        if not version:
            raise Exception(f"get mysql version from job param is null. pid:{self._p_id} jobId:{self._job_id}")
        if MySQLStrConstant.MARIADB in version:
            set_gtid_purged = ""
        if not support_parameter("mysqldump", "--set-gtid-purged"):
            set_gtid_purged = ""
        cmd = cmd_format("--user={} -h{} -P{} \
                --password --no-data --compact --add-drop-table --skip_add_locks --skip-lock-tables \
                --routines --triggers --events {}\
                {} --result-file={}",
                         self._mysql_user, self._mysql_ip, self._mysql_port, set_gtid_purged, database_name, sql_file)

        ret, _ = self.exec_xtrabackup_cmd(cmd, MysqlBackupToolName.MYSQLDUMP, self._mysql_pwd)
        if not ret:
            log.error(f"Backup table struct failed. pid:{self._p_id} jobId:{self._job_id}")
            return False
        log.info(f"Backup table struct success. pid:{self._p_id} jobId:{self._job_id}")
        return True

    def write_tmp_file(self, context, file_name):
        """
        写临时文件
        :param context:
        :param file_name:
        :return:
        """
        file_path = os.path.join(self._cache_path, file_name)
        if os.path.exists(file_path) and not su_exec_rm_cmd(file_path):
            log.error(f"Rm file failed. path:[{file_path}]. pid:{self._p_id} jobId:{self._job_id}")
        if not exec_overwrite_file(file_path, context):
            raise Exception(f"write file[{file_path}] failed. pid:{self._p_id} jobId:{self._job_id}")

    def get_last_copy_info(self, copy_type_array):
        log.info("Start to get data copy host_sn")
        param = dict()
        param[ParamKeyConst.JOB_ID] = self._job_id
        param[MySQLJsonConstant.APPLICTION] = self._json_param.get(MySQLJsonConstant.JOB, {}). \
            get(MySQLJsonConstant.PROTECTOBJECT)
        param[MySQLJsonConstant.TYPES] = copy_type_array
        param[MySQLJsonConstant.COPYID] = self._job_id
        copy_info_in_file_name = f"copy_info_hostsn_in_{self._job_id}"
        copy_info_out_file_name = f"copy_info_hostsn_out_{self._job_id}"
        self.write_tmp_file(param, copy_info_in_file_name)
        param_file = os.path.join(self._cache_path, copy_info_in_file_name)
        out_file = os.path.join(self._cache_path, copy_info_out_file_name)

        ret = exec_rc_tool_cmd("QueryPreviousCopy", param_file, out_file)
        if not ret:
            log.error(f"Failed to QueryPreviousCopy. pid:{self._p_id} jobId{self._job_id}")
            return False, ""
        out_info = ReadFile.read_param_file(out_file)
        if not out_info:
            log.error(f"Get copy info failed. pid:{self._p_id} jobId{self._job_id}")
            return False, ""
        return True, out_info

    def get_copy_info_host_sn(self):
        ret, out_info = self.get_last_copy_info([CopyDataTypeEnum.FULL_COPY.value])
        if not ret or not out_info:
            log.error(f"Failed to get last copy info. pid:{self._p_id} jobId{self._job_id}")
            return False, ""
        if not out_info.get(MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.BACKUPHOSTSN, ""):
            log.error("Failed to get data copy host_sn")
            return False, ""
        host_sn = out_info.get(MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.BACKUPHOSTSN)
        log.info(f"Succeed to get data copy info host_sn({host_sn}). \
            pid:{self._p_id} jobId{self._job_id}")
        return True, host_sn

    def get_copy_info_last_full_backup_time(self):
        ret, out_info = self.get_last_copy_info([CopyDataTypeEnum.FULL_COPY.value])
        if not ret or not out_info:
            log.error(f"Failed to get last copy info. pid:{self._p_id} jobId{self._job_id}")
            return False, ""
        if not out_info.get(MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.FIRSTFULLBACKUPTIME, ""):
            log.error("Failed to get data copy last backup time")
            return False, ""
        last_backup_time = out_info.get(MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.FIRSTFULLBACKUPTIME)
        log.info(f"Succeed to get data copy info last backup time({last_backup_time}). \
            pid:{self._p_id} jobId{self._job_id}")
        return True, int(last_backup_time)

    def get_first_full_backup_time(self, current_backup_time):
        # 返回恢复后第一次全量备份的时间，将此时间填入副本信息中
        first_full_backup_time = 0
        # 非全量备份直接返回0
        backup_type = int(self._json_param.get(MySQLJsonConstant.JOB, {}). \
                          get(MySQLJsonConstant.JOBPARAM, {}).get(MySQLJsonConstant.BACKUPTYPE, "0"))
        if backup_type not in [BackupTypeEnum.FULL_BACKUP.value, BackupTypeEnum.INCRE_BACKUP.value,
                               BackupTypeEnum.DIFF_BACKUP.value]:
            log.info(f"Get backup type is {backup_type}. pid:{self._p_id} jobId:{self._job_id}")
            return first_full_backup_time
        ret = True
        try:
            next_cause_key = self._json_param.get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.EXTENDINFO, {}). \
                get(MySQLJsonConstant.NEXTCAUSEPARAM, 0)
        except Exception as exception_str:
            log.error(f"Parse param failed. pid:{self._p_id} jobId:{self._job_id},error:{exception_str}")
            return first_full_backup_time
        # 0代表非恢复后的首次全量
        if int(next_cause_key) == 0:
            ret, first_full_backup_time = self.get_copy_info_last_full_backup_time()
        else:
            first_full_backup_time = current_backup_time
        if not ret:
            # 获取失败不影响当次备份任务结果
            log.warning(f"Get first full backup time failed. pid:{self._p_id} jobId:{self._job_id}")
            first_full_backup_time = 0
        log.info(f"Success get first full backup time{first_full_backup_time}.\
            pid:{self._p_id} jobId:{self._job_id}")
        return first_full_backup_time

    def flush_log(self):
        # 主动归档还未归档的日志
        exec_sql_param = self.generate_exec_sql_param()
        exec_sql_param.sql_str = "flush logs"
        ret, output = exec_sql(exec_sql_param)
        if not ret:
            log.error(f"Exec sql failed. sql:flush logs \
                ret:{ret} pid:{self._p_id} jobId{self._job_id}")
            return False
        self._flush_time = int(time.time())
        log.info(f"Flush logs success. pid:{self._p_id} jobId{self._job_id}")
        return True

    def get_log_file_parent_path(self):
        """
        获取日志文件的目录
        首要通过variables来获取，如果没有的话，则通过读取mysql的配置文件，取log_bin，然后找到父目录
        :return:
        """
        exec_sql_param = self.generate_exec_sql_param()
        exec_sql_param.sql_str = "show variables like 'log_%'"
        ret, output = exec_sql(exec_sql_param)
        if not ret:
            log.error(f"Exec sql failed. sql:show variables like 'log_%' \
                ret:{ret} pid:{self._p_id} jobId{self._job_id}")
            return False, ""
        log_bin_basename = ""
        for i in output:
            if i[0] == 'log_bin_basename':
                log_bin_basename = i[1]
                break
        if not log_bin_basename:
            log.error(f"Get log bin basename failed. start get by config. pid:{self._p_id} jobId: {self._job_id}")
            log_bin_basename = MysqlUtils.get_log_bin_by_config_file(self.my_cnf_path)
            if log_bin_basename:
                # 两种场景，是一个mysql-bin这种标识；
                # 或者是/data/mysql/binlog/mysql-bin这种目录，mysql-bin这只是一个标识，不是路径
                if log_bin_basename.find("/") != -1:
                    log.info(f"Mysql log bin is full file.")
                else:
                    log_bin_basename = MysqlUtils.get_data_dir_by_config_file(self.my_cnf_path) + "/" + log_bin_basename
                    log.info(f"Mysql log bin is {log_bin_basename}.")
        if not log_bin_basename:
            log.error(f"Get log bin basename failed. pid:{self._p_id} jobId{self._job_id}")
            return False, ""
        index = log_bin_basename.rfind('/')
        path = log_bin_basename[:index]
        return True, path

    def get_log_time_by_pos(self, log_file, pos):
        # 通过制定biglog位点，获取对应时间
        # 该命令要使用管道，log_file是用户的本地目录，无法做特殊字符校验，加''进行防护
        # 已经跟安全对齐此策略
        cmd_str = [
            cmd_format("mysqlbinlog --no-defaults --base64-output=decode-rows -v '{}'", log_file),
            cmd_format("grep \"end_log_pos {}\"", pos)
        ]
        ret, output, _ = execute_cmd_list_communicate(cmd_str)
        if ret != ExecCmdResult.SUCCESS or not output or len(output) < 17:
            log.error(f"Get binlog time by pos failed. ret:{ret}\
                pid:{self._p_id} jobId:{self._job_id}")
            return False, 0
        # 第2到第16个字符表示的是时间
        log_time = output[1:16]
        try:
            time_array = time.strptime(f"20{log_time}", "%Y%m%d %H:%M:%S")
        except Exception as exception_str:
            log.error(f"Convert time failed. pid:{self._p_id} jobId:{self._job_id},error:{exception_str}")
            return False, 0
        timestamp = int(time.mktime(time_array))
        log.info(f"Success get biglog pos time {timestamp}. pid:{self._p_id} jobId:{self._job_id}")
        return True, timestamp

    def get_backup_log_time(self, copy_path):
        # 获取备份之后的biglog位点，把这个时间作为副本时间
        xbackup_big_log = os.path.join(copy_path, "xtrabackup_binlog_info")
        ret, out = exec_cat_cmd(xbackup_big_log)
        if not ret:
            log.error(f"Cat xbackup binlog info failed. pid:{self._p_id} jobId{self._job_id}")
            return False, 0
        line_array = out.strip().split("\n")
        if not line_array:
            log.error(f"Get binlog pos failed. pid:{self._p_id} jobId{self._job_id}")
            return False, 0
        # 先取文件中最后一行,高版本中会记录所有的日志 低版本机会记录最后一个
        tool_name = self.get_backup_tool_name()
        if tool_name == MysqlBackupToolName.XTRBACKUP2:
            binlog_line = line_array[0]
        else:
            binlog_line = line_array[-1]
        text_array = binlog_line.split()
        if len(text_array) < 2:
            log.error(f"Get binlog pos failed. pid:{self._p_id} jobId{self._job_id}")
            return False, 0
        # 文件内容示例：mysql-bin.000006	385	0-2-22，这里需要取文件名合pos
        binlog_file = text_array[0]
        binlog_pos = text_array[1]
        ret, log_parent_path = self.get_log_file_parent_path()
        if not ret:
            log.error(f"Get log paraent path failed. pid:{self._p_id} jobId{self._job_id}")
            return False, 0
        ret, log_time = self.get_log_time_by_pos(os.path.join(log_parent_path, binlog_file), binlog_pos)
        return ret, log_time

    def parse_backup_lock_detail(self, output):
        # output为空，1、备份失败 2、不是执行的备份命令
        if not output:
            log.info("Backup failed or not backup cmd")
            return False
        # 是执行的全局锁还是非innoDB表锁
        lock_name = "database"
        if output.find('Executing FLUSH TABLES WITH READ LOCK') == -1:
            lock_name = "non-InnoDB tables"
        # 拿到mysql的版本
        mysql_version = self.get_json_param().get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.PROTECTOBJECT, {}) \
            .get(MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.VERSION)
        # 获取MySQL的锁的时间
        try:
            lock_time = self.get_lock_time(mysql_version, output)
        except Exception as except_str:
            log.error(f"Parse backup lock time failed.error:{except_str}")
            lock_time = "00:00:00"
        self._lock_name = lock_name
        self._lock_time = lock_time
        return True

    def convert_xtrbackup_error(self, output, tool_name):
        if XtrbackupErrStr.CORRUPT_DATABASE_PAGE in output and \
                tool_name in [MysqlBackupToolName.XTRBACKUP2, MysqlBackupToolName.XTRBACKUP8]:
            self._error_code = MySQLErrorCode.EXEC_BACKUP_RECOVER_LIVEMOUNT_CMD_FAIL
            self._log_detail_params = ["xtrbackup", XtrbackupErrStr.CORRUPT_DATABASE_PAGE]
        # 记录mariadb 可能出现的已准备报错
        if XtrbackupErrStr.DATABASE_COPY_IS_PREPARED in output:
            self._internal_error = XtrbackupErrStr.DATABASE_COPY_IS_PREPARED

    def backup_log(self, copy_path):
        with open(self.get_last_bin_log_file()) as f:
            last_bin_log = f.read()
        if not last_bin_log:
            log.error("Failed to get last bin log")
            return False
        log.info("Start backup bin log")
        exec_sql_param = self.generate_exec_sql_param()
        ret, all_bin_logs = MysqlUtils.get_all_log_file(exec_sql_param)
        if not ret or not all_bin_logs:
            log.error("Failed to get all bin log")
            return False
        need_backup_logs = list()
        ret, log_dir = self.get_log_file_parent_path()
        for bin_log in all_bin_logs:
            if bin_log >= last_bin_log:
                bin_log_path = os.path.join(log_dir, bin_log)
                if not os.path.exists(bin_log_path):
                    self._error_code = MySQLErrorCode.ERR_BIN_LOG_NOT_EXIST
                    return False
                need_backup_logs.append(bin_log_path)
        ret = mysql_backup_files(self._job_id, need_backup_logs, copy_path)
        if not ret:
            log.error(f"Backup log file failed. pid:{self._p_id} jobId{self._job_id}")
            return False
        log.info(f"Backup log file success")
        return True

    def get_mysql_ip(self):
        return self._mysql_ip

    def get_mysql_port(self):
        return self._mysql_port

    def get_last_bin_log_file(self):
        return os.path.join(self._cache_path, self.get_host_sn() + '_bin_log')

    def flush_log_for_eapp(self):
        ret = self.flush_log()
        if not ret:
            log.error("Failed to flush log")
            return False
        log.info("Flush log success")
        exec_sql_param = self.generate_exec_sql_param()
        ret, last_bin_log = MysqlUtils.get_last_log_file(exec_sql_param)
        if not ret or not last_bin_log:
            log.error("Failed to get last bin log")
            return False
        MysqlUtils.save_last_bin_log(self.get_last_bin_log_file(), last_bin_log)
        log.info("Save last bin log success")
        return True

    def calc_progress(self):
        return True, "5"

    def backup_connect_param(self):
        connect_param_path = os.path.join(self._data_path, 'connect_param.json')
        connect_param = self.generate_exec_sql_param()
        passwd = get_passwd(connect_param)
        connect_param.passwd = Kmc().encrypt(passwd)
        exec_overwrite_file(connect_param_path, connect_param.__dict__)
        clear(passwd)

    def clear_password_environ_for_maria(self):
        if self.get_backup_tool_name() == MysqlBackupToolName.MARIADBBACKUP:
            del os.environ["MYSQL_PWD"]

