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
import re
import time
from datetime import datetime

from mysql.src.utils.common_func import SQLParam, exec_mysql_sql_cmd


def parse_time_stamp(time_stamp_str):
    time_stamp = time.localtime(int(time_stamp_str))
    return time.strftime("%Y-%m-%d %H:%M:%S", time_stamp)


def parse_xtrabackup_info(file_path):
    info = {}
    with open(file_path, 'r') as file:
        for line in file:
            if '=' in line:
                key, value = line.strip().split('=', 1)
                info[key.strip()] = value.strip()
    if info.get("binlog_pos"):
        binlog_pos = info.get('binlog_pos')
        match = re.search(r"filename '([^']+)', position '(\d+)'", binlog_pos)
        if match:
            filename = match.group(1)
            position = match.group(2)
            info.update({"binlog_filename": filename, "binlog_position": position})
    return info


def parse_log_meta(meta_file):
    timestamp_id_dict = {}
    with open(meta_file, 'r', encoding='utf-8') as file_read:
        for line in file_read.readlines():
            key_value = line.strip('\n').split(";")
            key = key_value[0].strip()
            value = key_value[1].strip()
            start_stamp = value.split("~")[0]
            end_stamp = value.split("~")[1]
            time_range = {
                "start_stamp": start_stamp,
                "end_stamp": end_stamp
            }
            timestamp_id_dict.update({key: time_range})
    return timestamp_id_dict


def convert_to_timestamp(time_str):
    datetime_obj = datetime.strptime(time_str, '%Y-%m-%d %H:%M:%S')
    return int(datetime_obj.timestamp())


def parse_num(file_name):
    return int(''.join(char for char in file_name if char.isdigit()))


def get_bin_log_names(bin_log_copy_dir):
    binlog_file_names = list()
    for file_name in os.listdir(bin_log_copy_dir):
        if file_name.endswith(".index"):
            continue
        new_path = os.path.join(bin_log_copy_dir, file_name)
        if not os.path.isfile(new_path):
            continue
        binlog_file_names.append(file_name)
    binlog_file_names.sort(key=lambda file: parse_num(file), reverse=True)
    return binlog_file_names


def convert_restore_binlog_files_str(restore_files: [str]):
    restore_files.sort(key=lambda f: parse_num(os.path.basename(f)))
    return " ".join(map(lambda x: f"\"{x}\"", restore_files))


def stop_slave(sql_param: SQLParam):
    sql_param.sql = "stop slave"
    ret, output = exec_mysql_sql_cmd(sql_param)
    if not ret:
        return False
    return True


def reset_slave_all(sql_param: SQLParam):
    sql_param.sql = "reset slave all"
    ret, output = exec_mysql_sql_cmd(sql_param)
    if not ret:
        return False
    return True
