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
from threading import Thread

from common.cleaner import clear_repository_dir
from common.common import execute_cmd_list_communicate
from common.common import exter_attack
from common.const import SubJobStatusEnum, RpcParamKey
from common.util.cmd_utils import cmd_format
from common.util.exec_utils import exec_mkdir_cmd, exec_overwrite_file
from common.util.scanner_utils import scan_dir_size
from mysql import log
from mysql.src.common.constant import ExecCmdResult, MySQLJsonConstant, MySQLProgressFileType
from mysql.src.common.error_code import MySQLErrorCode
from mysql.src.common.execute_cmd import mysql_backup_files
from mysql.src.common.parse_parafile import ReadFile
from mysql.src.protect_mysql_base import MysqlBase
from mysql.src.service.backup.backup_func import get_last_copy_info
from mysql.src.utils.common_func import find_log_bin_path_dir, get_binlog_filenames


class MysqlBackupLog(MysqlBase):

    def __init__(self, p_id, job_id, sub_job_id, json_param):
        super().__init__(p_id, job_id, sub_job_id, json_param)
        self._report_progress_thread_start = True
        self._data_size = 0
        self._error_code = 0
        self.last_backup_binlog = ""
        self.start_binlog_time = ""

    @staticmethod
    def time_str_convert_timestamp(time_str):
        time_array = time.strptime(time_str, "%Y%m%d %H:%M:%S")
        timestamp = int(time.mktime(time_array))
        return timestamp

    def get_log_backup_extend_info(self, start_timestamp, end_timestamp):
        log.info("start_timestamp: %s, end_timestamp: %s" % (start_timestamp, end_timestamp))
        # 构造日志副本信息中需要的字段
        # 判断如果起始时间超过首次全量备份时间，就截断
        param_application = self._json_param.get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.PROTECTOBJECT)
        out_info = get_last_copy_info(None, self._job_id, [RpcParamKey.LOG_COPY], application=param_application)
        if out_info:
            begin_time = int(out_info.get("extendInfo", {}).get("endTime", ""))
            log.info(f"begin_time: {begin_time}, start_timestamp:{start_timestamp}")
            if begin_time < start_timestamp:
                begin_time = self.get_start_full_time(param_application)
        else:
            begin_time = self.get_start_full_time(param_application)
        json_dict = {
            "logDirName": "",
            "associatedCopies": [],
            "beginTime": begin_time,
            "endTime": end_timestamp
        }
        return json_dict

    def get_start_full_time(self, param_application):
        out_info = get_last_copy_info(None, self._job_id, [RpcParamKey.FULL_COPY, RpcParamKey.INCREMENT_COPY,
                                                           RpcParamKey.DIFF_COPY], application=param_application)
        log.info(f"full_copy_out_info:{out_info}")
        return int(out_info.get("extendInfo", {}).get("backupTime", ""))

    def get_log_start_time(self, bin_log_file):
        ret, output = self.get_bin_log_sql(bin_log_file)
        if not ret:
            return False, 0
        time_lines = output.split("\n")
        # 取日志中的起始时间，开始时间是在第一行，第2到第16个字符表示的是时间
        try:
            start_time = time_lines[0][1:16]
        except Exception as exception_str:
            log.error(f"Get start time format err. pid:{self._p_id} jobId:{self._job_id},error:{exception_str}")
            return False, 0
        return True, self.time_str_convert_timestamp(f"20{start_time}")

    def get_log_end_time(self, bin_log_file):
        ret, output = self.get_bin_log_sql(bin_log_file)
        if not ret:
            return False, 0
        time_lines = output.split("\n")
        # 取日志中的结束时间，结束时间是在倒数第二行，第2到第16个字符表示的是时间
        try:
            end_time = time_lines[-2][1:16]
        except Exception as exception_str:
            log.error(f"Get end time format err. pid:{self._p_id} jobId:{self._job_id},error:{exception_str}")
            return False, 0
        return True, self.time_str_convert_timestamp(f"20{end_time}")

    def get_bin_log_sql(self, binlog_file_path):
        # 该命令要使用管道，file_path是用户的本地目录，无法做特殊字符校验，加''进行防护
        cmd_str = [
            cmd_format("mysqlbinlog --no-defaults --base64-output=decode-rows -v '{}'", binlog_file_path),
            "grep \"server id\""
        ]
        ret, output, _ = execute_cmd_list_communicate(cmd_str)
        log.info(f"cmd_format:{cmd_str}, ret:{ret}")
        if ret != ExecCmdResult.SUCCESS or not output:
            log.error(f"Get binlog sql file failed. ret:{ret}\
                pid:{self._p_id} jobId:{self._job_id}")
            return False, ""
        return True, output

    def prepare_backup_log(self, target_path):
        self.flush_log()
        self.kill_backup_progress()
        if os.path.exists(target_path):
            clear_repository_dir(target_path)
        else:
            return exec_mkdir_cmd(target_path)
        return True

    def get_copy_path(self):
        copy_path = os.path.join(self._log_path, f"{self.get_host_sn()}")
        return True, copy_path

    def set_backup_all_param_log(self):
        self.set_log_path()
        self.set_cache_path()
        self.set_data_path()
        self.set_meta_path()
        ret, copy_path = self.get_copy_path()
        log.info(f"Get copy path success. pid:{self._p_id} jobId{self._job_id}")
        self.set_mysql_param()
        return True, copy_path

    @exter_attack
    def exec_sub_job(self):
        log.info(f"Log backup begin. pid:{self._p_id} jobId:{self._job_id}")
        ret, copy_path = self.set_backup_all_param_log()
        log.info(f"ret:{ret},copy_path:{copy_path}")
        self.write_progress_file(SubJobStatusEnum.RUNNING.value, 1,
                                 MySQLProgressFileType.COMMON)
        progress_thread = Thread(target=self.report_backup_progress_thread)
        progress_thread.daemon = True
        progress_thread.start()
        ret, start_timestamp, end_timestamp = self.exec_backup(copy_path)
        self._report_progress_thread_start = False
        progress_thread.join()
        if not ret:
            return False
        log.info(f"Log time:{start_timestamp}--{end_timestamp}.{self.get_log_comm()}")
        # 获取上一个（全量、增量、差异、日志）副本的时间，与上一个副本时间做比较，校验日志是否连续
        json_extend_info = self.get_log_backup_extend_info(start_timestamp, end_timestamp)
        self.write_copy_info_log_backup(end_timestamp, json_extend_info)
        ret = self.write_log_flag_file()
        if not ret:
            return False
        ret, size = scan_dir_size(self._job_id, copy_path)
        if ret and size != 0:
            self._data_size = size
        log.info(f"Success to get log copy size {self._data_size}.")
        log.info(f"Log backup success. pid:{self._p_id} jobId:{self._job_id}")
        return True

    def write_copy_info_log_backup(self, date_time, extend_info):
        copy_info_path = os.path.join(self._cache_path, f"copy_info_{self._job_id}")
        copy_json = self._json_param.get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.COPY, [{}])[0]
        copy_json[MySQLJsonConstant.TIMESTAMP] = date_time
        copy_json[MySQLJsonConstant.EXTENDINFO] = extend_info
        exec_overwrite_file(copy_info_path, copy_json)
        return True

    def exec_backup(self, copy_path):
        ret = self.prepare_backup_log(copy_path)
        if not ret:
            log.error(f"Exec prepare_backup_log failed. {self.get_log_comm()}")
            return False, 0, 0
        sql_param = self.generate_sql_param()
        binlog_dir = find_log_bin_path_dir(sql_param, self.my_cnf_path)
        binlog_filenames = get_binlog_filenames(sql_param)
        # 最后一个文件是flush log 生成的，不备份
        binlog_filenames = binlog_filenames[:-1]
        if not binlog_dir or not binlog_filenames:
            log.info(f"dir_path or binlog_filenames query empty,{self.get_log_comm()}")
            return False, 0, 0
        filter_binlog_names = self.filter_backup_binlog_names(binlog_dir, binlog_filenames)
        backup_files = [os.path.join(binlog_dir, file) for file in filter_binlog_names]
        log.info(f"all file array size: {len(backup_files)}")
        log.info(f"target_path:{copy_path}")
        # 备份工具限制为1000 插件可以指定更大的数字 覆盖掉1000 避免在binlog日志文件数量大于1000时备份失败
        ret = mysql_backup_files(self._job_id, backup_files, copy_path, len(backup_files))
        if not ret:
            self._error_code = MySQLErrorCode.ERR_BACKUP_TOOL_FAILED
            log.error(f"Backup log file failed. pid:{self._p_id} jobId{self._job_id}")
            return False, 0, 0
        start_ret, start_timestamp = self.get_log_start_time(os.path.join(binlog_dir, filter_binlog_names[0]))
        self.last_backup_binlog = filter_binlog_names[-1]
        end_ret, end_timestamp = self.get_log_end_time(os.path.join(binlog_dir, filter_binlog_names[-1]))
        if not start_ret or not end_ret:
            log.error(f"Get logCopy info failed. pid:{self._p_id} jobId:{self._job_id}")
            return False, 0, 0
        _, start_binlog_time = self.get_log_start_time(os.path.join(binlog_dir, binlog_filenames[0]))
        self.start_binlog_time = str(start_binlog_time)
        return True, start_timestamp, end_timestamp

    def get_meta_host_sn_file_path(self):
        host_sn = self.get_host_sn()
        path = os.path.join(self._meta_path, host_sn)
        return True, path

    def write_log_flag_file(self):
        ret, sn_file_path = self.get_meta_host_sn_file_path()
        json_str = {
            MySQLJsonConstant.LOG_FLAG: self.last_backup_binlog,
            MySQLJsonConstant.LOG_FLAG_START_TIME: self.start_binlog_time
        }
        try:
            exec_overwrite_file(sn_file_path, json_str)
        except Exception as exception_str:
            log.error(f"Write log flag file failed.{self.get_log_comm()},error:{exception_str}")
            return False
        log.info(f"write log flag success. file:{self.last_backup_binlog},{self.get_log_comm()}")
        return True

    def filter_backup_binlog_names(self, binlog_dir, binlog_filenames):
        ret, sn_file_path = self.get_meta_host_sn_file_path()
        try:
            read_json = ReadFile.read_param_file(sn_file_path)
        except Exception as err_exception:
            log.error(f"err_exception:{err_exception}")
            return binlog_filenames
        last_start_time = read_json.get(MySQLJsonConstant.LOG_FLAG_START_TIME, "")
        last_binlog_name = read_json.get(MySQLJsonConstant.LOG_FLAG, "")
        if not last_start_time or not last_binlog_name:
            log.info(f"last_start_time or last_binlog_name is empty:{self.get_log_comm()}")
            return binlog_filenames
        log.info(f"last_start_time:{last_start_time},last_binlog_name:{last_binlog_name}")
        ret, current_start_timestamp = self.get_log_start_time(os.path.join(binlog_dir, binlog_filenames[0]))
        log.info(f"ret:{ret}, current_start_timestamp:{current_start_timestamp}")
        if str(last_start_time) != str(current_start_timestamp):
            return binlog_filenames
        if last_binlog_name not in binlog_filenames:
            return binlog_filenames
        index = binlog_filenames.index(last_binlog_name)
        filter_binlog_names = binlog_filenames[index + 1:]
        return filter_binlog_names
