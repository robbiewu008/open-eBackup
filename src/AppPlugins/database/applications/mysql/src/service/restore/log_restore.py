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

from common.common import execute_cmd
from common.exception.common_exception import ErrCodeException
from mysql import log
from mysql.src.common.constant import SystemConstant, MysqlPrivilege
from mysql.src.common.error_code import MySQLErrorCode
from mysql.src.service.base_service import BaseService
from mysql.src.service.restore.restore_param import RestoreParam
from mysql.src.utils.common_func import exec_mysql_sql_cmd, get_version_from_sql
from mysql.src.utils.restore_func import parse_time_stamp, parse_xtrabackup_info, \
    convert_to_timestamp, convert_restore_binlog_files_str, get_bin_log_names, parse_log_meta


class LogRestore(BaseService):
    def __init__(self, job_id, sub_job_id, restore_param: RestoreParam, database=""):
        super().__init__(job_id, sub_job_id, restore_param.pid)
        self.param: RestoreParam = restore_param
        self.database = database

    def pre_log_restore(self):
        self.param.reset_sql_param()
        max_allowed_packet = SystemConstant.MAX_ALLOWED_PACKET
        # 设置最大允许的数据包数量
        self.param.sql_param.sql = f"set global max_allowed_packet={max_allowed_packet};"
        ret, output = exec_mysql_sql_cmd(self.param.sql_param)
        # 设置最大等待超时时间
        log.info(f"ret:{ret},output:{output},{self.get_log_comm()}")
        self.param.sql_param.sql = f"set global wait_timeout=288000;"
        ret, output = exec_mysql_sql_cmd(self.param.sql_param)
        log.info(f"ret:{ret},output:{output},{self.get_log_comm()}")
        # 设置最大命令等待时间
        self.param.sql_param.sql = f"set global interactive_timeout=288000;"
        ret, output = exec_mysql_sql_cmd(self.param.sql_param)
        log.info(f"ret:{ret},output:{output},{self.get_log_comm()}")

    def exec_restore(self):
        self.pre_log_restore()
        stop_datetime = parse_time_stamp(self.param.restore_time_stamp)
        xtrabackup_info_path = os.path.join(self.param.restore_copy_path, 'xtrabackup_info')
        if not os.path.exists(xtrabackup_info_path):
            log.error(f"xtrabackup_info_path:{xtrabackup_info_path} not exits")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)
        xtrabackup_info = parse_xtrabackup_info(xtrabackup_info_path)
        start_position = xtrabackup_info.get("binlog_position")
        log_start_time = xtrabackup_info.get("end_time")
        binlog_file = xtrabackup_info.get("binlog_filename")
        log.info(f"start_position:{start_position},log_start_time:{log_start_time},binlog_filename:{binlog_file}")
        if not start_position:
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)
        ret, restore_files = self.get_log_restore_files(convert_to_timestamp(log_start_time), binlog_file)
        log.info(f"ret:{ret}, restore_files:{restore_files}")
        if not ret:
            log.error(f"get restore_files error,{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)
        database_cmd = f"--database={self.database}" if self.database else ""
        skip_gtids_cmd = self.get_skip_gtids_cmd()
        restore_cmd = f"mysqlbinlog --no-defaults {database_cmd} {skip_gtids_cmd} {restore_files} " \
                      f"--disable-log-bin " \
                      f"--start-position={start_position} " \
                      f"--stop-datetime='{stop_datetime}' | mysql -u{self.param.user} -P{self.param.port} " \
                      f"-h{self.param.mysql_ip} -p'{self.param.pass_wd}'"
        restore_exec_cmd = f"/bin/bash -c \"{restore_cmd}\""
        ret, out_str, err_str = execute_cmd(restore_exec_cmd)
        log.info(f"ret:{ret}, out_str:{out_str}, err_str:{err_str}")
        if not ret:
            log.error(f"restore log failed,output:{out_str}, err_str:{err_str},{self.get_log_comm()}")
            raise ErrCodeException(MySQLErrorCode.SYSTEM_ERROR)
        log.info(f"restore log success,output:{out_str},err_str:{err_str},{self.get_log_comm()}")

    def get_log_restore_files(self, copy_end_timestamp, start_binlog_name):
        meta_file = os.path.join(self.param.origin_log_path, f"{self.param.restore_copy_id}.meta")
        log.info(f"meta_file:{meta_file}")
        if not os.path.exists(meta_file):
            log.error(f"Get timestamp id dict is empty. {self.get_log_comm()}")
            return False, []
        log_meta_dict = parse_log_meta(meta_file)
        log.info(f"Get timestamp id dict is {log_meta_dict}.{self.get_log_comm()}")
        restore_end_time = int(self.param.restore_time_stamp)
        sub_dir_list = self.filter_related_binlog_dirs(log_meta_dict, restore_end_time, copy_end_timestamp)
        sub_dir_list.sort(key=lambda x: list(x.values())[0])
        log.info(f"sub_dir_list:{sub_dir_list}")
        sub_dir_list = sub_dir_list[::-1]
        restore_file_list = []
        restore_binlog_names = []
        for dir_dict in sub_dir_list:
            key, value = dir_dict.popitem()
            log.info(f"dir_name:{key},end_time:{value}")
            bin_log_copy_dir = self.get_bin_log_copy_dir(key)
            bin_log_names = get_bin_log_names(bin_log_copy_dir)
            log.info(f"bin_log_names:{bin_log_names}")
            for bin_log_name in bin_log_names:
                if bin_log_name in restore_binlog_names:
                    continue
                restore_binlog_names.append(bin_log_name)
                restore_file = os.path.join(bin_log_copy_dir, bin_log_name)
                restore_file_list.append(restore_file)
                if bin_log_name != start_binlog_name:
                    continue
                log.info(f"restore_files:{restore_file_list}")
                return True, convert_restore_binlog_files_str(restore_file_list)
        return True, convert_restore_binlog_files_str(restore_file_list)

    # 兼容老版本，多出 mysql_{copy_id}_{host_sn} 格式子文件
    def get_bin_log_copy_dir(self, log_copy_id):
        bin_log_copy_dir = os.path.join(self.param.origin_log_path, log_copy_id)
        for sub_file in os.listdir(bin_log_copy_dir):
            sub_file_path = os.path.join(bin_log_copy_dir, sub_file)
            if os.path.isdir(sub_file_path):
                return sub_file_path
        return bin_log_copy_dir

    def filter_related_binlog_dirs(self, log_meta_dict: dict, restore_end_time: int, copy_end_timestamp: int):
        log.info(f"restore_end_time:{restore_end_time},copy_end_timestamp:{copy_end_timestamp}")
        sub_dir_list = []
        for sub_dir in os.listdir(self.param.origin_log_path):
            log.info(f"sub_dir:{sub_dir}")
            sub_dir_path = os.path.join(self.param.origin_log_path, sub_dir)
            log.info(f"sub_dir_path:{sub_dir_path}")
            # 非文件、时间点恢复的开始时间大于等于时间戳结束，时间点恢复的结束时间小于等于时间戳开始，这三种情况不满足条件
            if not os.path.isdir(sub_dir_path) or sub_dir not in log_meta_dict:
                continue
            time_range = log_meta_dict[sub_dir]
            start_stamp = int(time_range.get("start_stamp"))
            end_stamp = int(time_range.get("end_stamp"))
            if start_stamp >= restore_end_time or end_stamp <= copy_end_timestamp:
                continue
            sub_dir_list.append({sub_dir: start_stamp})
        return sub_dir_list

    def get_skip_gtids_cmd(self):
        version = get_version_from_sql(self.param.sql_param)
        if MysqlPrivilege.is_mariadb(version):
            return ""
        return "--skip-gtids"
