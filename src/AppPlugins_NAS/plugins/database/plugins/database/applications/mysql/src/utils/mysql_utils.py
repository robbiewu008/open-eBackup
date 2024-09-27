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

import fcntl
import os.path
import pwd
import stat

from mysql import log
from common.common import execute_cmd, execute_cmd_list, touch_file
from common.const import EnumPathType
from common.err_code import CommErrCode
from common.exception.common_exception import ErrCodeException
from common.file_common import check_file_or_dir
from common.util.exec_utils import check_path_valid
from mysql.src.common.constant import MySQLJsonConstant, MySQLStrConstant, RestorePath, ExecCmdResult, \
    MysqlConfigFileKey
from mysql.src.common.execute_cmd import exec_sql, match_greatsql
from mysql.src.common.parse_parafile import ExecSQLParseParam
from mysql.src.protect_mysql_base_utils import MysqlBaseUtils


class MysqlUtils:
    @staticmethod
    def get_cluster_type(json_param):
        return json_param.get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.PROTECTOBJECT, {}) \
            .get(MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.CLUSTERTYPE, "")

    @staticmethod
    def get_all_log_file(exec_sql_param: ExecSQLParseParam):
        exec_sql_param.sql_str = "show binary logs"
        ret, output = exec_sql(exec_sql_param)
        if not ret or not output:
            log.error(f"Exec sql failed. sql:show binary logs")
            return False, []
        all_log_file = list()
        for i in output:
            all_log_file.append(i[0])
        return True, all_log_file

    @staticmethod
    def get_last_log_file(exec_sql_param: ExecSQLParseParam):
        ret, logs = MysqlUtils.get_all_log_file(exec_sql_param)
        if not ret or not logs:
            return False, ''
        if len(logs) >= 3:
            return True, logs[-3]
        return True, logs[-2]

    @staticmethod
    def get_master_info_by_host(master_info_list, host):
        for master_info in master_info_list:
            if host == master_info.master_host:
                return master_info
        return ''

    @staticmethod
    def find_bin_log_pattern(my_cnf_path):
        ret, mycnf_file = MysqlBaseUtils.find_mycnf_path(my_cnf_path)
        if not ret:
            log.warning("Failed to find my.cnf file")
            return MySQLStrConstant.DEFAULT_BIN_LOG_PATTERN
        log_bin_line = ''
        for line in open(mycnf_file):
            if line.startswith("log-bin="):
                log_bin_line = line
                break
        if not log_bin_line:
            return MySQLStrConstant.DEFAULT_BIN_LOG_PATTERN
        return log_bin_line.split("=")[1].strip()

    @staticmethod
    def get_log_bin_index_by_config_file(my_cnf_path):
        """
        从mysql配置文件中，获取mysql的log_bin_index文件的路径，如果没有则返回空字符串
        :return: 如果没有，则返回空字符串
        """
        ret, config_file = MysqlBaseUtils.find_mycnf_path(my_cnf_path)
        if not ret:
            raise Exception("Failed to find my.cnf file")
        for line in open(config_file):
            for log_bin_index in MysqlConfigFileKey.LOG_BIN_INDEX_ARR:
                if line.startswith(log_bin_index):
                    return line.split("=")[1].strip()
        return ""

    @staticmethod
    def get_log_bin_by_config_file(my_cnf_path):
        """
        从mysql配置文件中，读取mysql的log_bin文件的路径
        :return: 如果没有，则返回空字符串
        """
        ret, config_file = MysqlBaseUtils.find_mycnf_path(my_cnf_path)
        if not ret:
            raise Exception("Failed to find my.cnf file")
        for line in open(config_file):
            for log_bin_index in MysqlConfigFileKey.LOG_BIN_ARR:
                if line.startswith(log_bin_index):
                    return line.split("=")[1].strip()
        return ""

    @staticmethod
    def get_relay_log_by_config_file(my_cnf_path):
        """
        从mysql配置文件中，读取mysql的relay-log文件的路径
        :return: 如果没有，则返回空字符串
        """
        ret, config_file = MysqlBaseUtils.find_mycnf_path(my_cnf_path)
        if not ret:
            raise Exception("Failed to find my.cnf file")
        for line in open(config_file):
            for log_bin_index in MysqlConfigFileKey.RELAY_LOG_ARR:
                if line.startswith(log_bin_index):
                    return line.split("=")[1].strip()
        return ""

    @staticmethod
    def get_relay_log_recovery_by_config_file(my_cnf_path):
        """
        从mysql配置文件中，读取mysql的relay-log文件的路径
        :return: 如果没有，则返回空字符串
        """
        ret, config_file = MysqlBaseUtils.find_mycnf_path(my_cnf_path)
        if not ret:
            raise Exception("Failed to find my.cnf file")
        for line in open(config_file):
            for log_bin_index in MysqlConfigFileKey.RELAY_LOG_DIRECTORY:
                if line.startswith(log_bin_index):
                    return line.split("=")[1].strip()
        return ""

    @staticmethod
    def get_innodb_undo_directory_by_config_file(my_cnf_path):
        """
        从mysql配置文件中，读取mysql的inno undo配置
        :return: 如果没有，则返回空字符串
        """
        ret, config_file = MysqlBaseUtils.find_mycnf_path(my_cnf_path)
        if not ret:
            raise Exception("Failed to find my.cnf file")
        with open(config_file, "r", encoding='UTF-8') as f:
            for line in f.readlines():
                value = MysqlUtils.get_value_by_keys(line, MysqlConfigFileKey.INNODB_UNDO_DIRECTORY)
                if value != "":
                    return value
                else:
                    continue
        return ""

    @staticmethod
    def get_innodb_undo_tablespace_by_config_file(my_cnf_path):
        """
        从mysql配置文件中，读取mysql的inno undo table space配置
        :return: 如果没有，则返回空字符串
        """
        ret, config_file = MysqlBaseUtils.find_mycnf_path(my_cnf_path)
        if not ret:
            raise Exception("Failed to find my.cnf file")
        with open(config_file, "r", encoding='UTF-8') as f:
            for line in f.readlines():
                line_strip = line.strip()
                if line_strip.startswith("#"):
                    continue
                value = MysqlUtils.get_value_by_keys(line_strip, MysqlConfigFileKey.INNODB_TABLE_SPACE)
                if value != "":
                    return value
                else:
                    continue
        return "0"

    @staticmethod
    def innodb_undo_tablespaces_deprecated(version):
        if MySQLStrConstant.MARIADB in version or version.startswith("5."):
            return False
        if version.startswith("8.0"):
            if match_greatsql(version):
                version = version.split('-')[0]
            version_suffix = version[4::]
            try:
                return int(version_suffix) >= 19
            except Exception as parse_error:
                log.error(f"parse error:{parse_error}")
                return False
        return True

    @staticmethod
    def get_data_dir_by_config_file(my_cnf_path):
        """
        从mysql配置文件中，读取mysql的数据目录
        :return: 如果没有，则返回空字符串
        """
        ret, config_file = MysqlBaseUtils.find_mycnf_path(my_cnf_path)
        if not ret:
            raise Exception("Failed to find my.cnf file")
        with open(config_file, "r", encoding='UTF-8') as f:
            for line in f.readlines():
                value = MysqlUtils.get_value_by_keys(line, MysqlConfigFileKey.DATA_DIR)
                if value != "":
                    return value
                else:
                    continue
        return ""

    @staticmethod
    def get_value_by_keys(line, keys):
        for key in keys:
            if not line.startswith(key):
                continue
            return line.split("=")[1].strip()
        return ""

    @staticmethod
    def get_kv_by_keys(line, keys):
        for key in keys:
            if not line.startswith(key):
                continue
            return line
        return ""

    @staticmethod
    def get_current_server_id(my_cnf_path):
        ret, mycnf_file = MysqlBaseUtils.find_mycnf_path(my_cnf_path)
        if not ret:
            log.error("Failed to get my.cnf")
            return False, 0
        f = open(mycnf_file)
        server_id = 0
        lines = f.readlines()
        for line in lines:
            if line.startswith(MySQLStrConstant.SERVER_ID):
                server_id = int(line.split("=")[1].strip())
        f.close()
        if not server_id:
            log.error("Failed to find server-id in my.cnf")
            return False, 0
        log.info("Get server-id success:%s", server_id)
        return True, server_id

    @staticmethod
    def generate_server_id(cache_path, len_nodes, my_cnf_path):
        ret, server_id = MysqlUtils.get_current_server_id(my_cnf_path)
        if not ret:
            return False, 0
        new_server_id = server_id + len_nodes
        server_id_file = os.path.join(cache_path, RestorePath.SERVER_ID_FILE)
        file_desc = os.open(server_id_file, os.O_CREAT | os.O_RDWR, stat.S_IWUSR | stat.S_IRUSR)
        try:
            fcntl.lockf(file_desc, fcntl.LOCK_EX | fcntl.LOCK_NB)
            file = open(server_id_file, 'r+')
            lines = file.readlines()
            # 未写入过server-id，直接写入
            if not lines or str(new_server_id) not in lines:
                lines.append(str(new_server_id))
                file.writelines(lines)
                file.close()
                return True, new_server_id
            while str(new_server_id) in lines:
                new_server_id = new_server_id + 1
            lines.append(str(new_server_id))
            file.writelines(lines)
        except IOError as ex:
            log.exception(f"exclusive operate error:{ex}")
        finally:
            os.close(file_desc)
        log.info("Generate server-id success:%s", new_server_id)
        return True, new_server_id

    @staticmethod
    def create_server_id_file(cache_path):
        server_id_file = os.path.join(cache_path, RestorePath.SERVER_ID_FILE)
        if os.path.exists(server_id_file):
            log.warning("Server-id file exists")
            return
        f = open(server_id_file, 'x')
        f.close()
        log.info("Create server-id file success")

    @staticmethod
    def save_server_id(server_id, my_cnf_path):
        ret, mycnf_file = MysqlBaseUtils.find_mycnf_path(my_cnf_path)
        if not ret:
            log.error("Failed to get my.cnf")
            return False
        f = open(mycnf_file)
        lines = f.readlines()
        f.close()
        for index, line in enumerate(lines):
            if line.startswith("server-id"):
                lines[index] = "server-id=" + str(server_id) + "\n"
        MysqlUtils.save_file(mycnf_file, lines)
        log.info("Save server_id(%s) success.", server_id)
        return True

    @staticmethod
    def save_file(file_path, lines):
        if not check_path_valid(file_path, False):
            log.error("The mkdir path param is invalid: %s.", file_path)
            raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message=f"The file[{file_path}] is invalid.")
        if os.path.exists(file_path):
            path_type = check_file_or_dir(file_path)
            if path_type != EnumPathType.FILE_TYPE:
                log.error(f"Check des path not file type: {path_type}")
                raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message=f"The file[{file_path}] is not file.")

        with os.fdopen(os.open(file_path, os.O_WRONLY, stat.S_IWUSR | stat.S_IRUSR), 'w') as out:
            out.truncate()
            out.writelines(lines)

    @staticmethod
    def read_file(file_path):
        with open(file_path) as f:
            return f.readlines()

    @staticmethod
    def eapp_is_running():
        ret, _, _ = execute_cmd_list([f"systemctl status {MySQLStrConstant.EAPPMYSQLSERVICES}",
                                      "grep \"Active: active (running)\""],
                                     True)
        if ret == ExecCmdResult.SUCCESS:
            log.info(f"EAppMySQL is running.")
            return True
        ret, _, _ = execute_cmd_list(
            [f"systemctl status {MySQLStrConstant.EAPPMYSQLSERVICES}", "grep \"Active: active (exited)\""],
            True)
        if ret == ExecCmdResult.SUCCESS:
            log.info(f"EAppMySQL is running.")
            return True
        return False

    @staticmethod
    def enable_skip_slave_start(is_new_location: bool, my_cnf_path):
        if not is_new_location:
            return True
        ret, mycnf_file = MysqlBaseUtils.find_mycnf_path(my_cnf_path)
        if not ret:
            log.error("Failed to get my.cnf")
            return False
        with open(mycnf_file) as cnf_file:
            lines = cnf_file.readlines()
        mysqld_idx = -1
        for idx, line in enumerate(lines):
            if line.strip(os.linesep) == '[mysqld]':
                mysqld_idx = idx
                break
        if mysqld_idx == -1:
            log.error("Failed to find mysqld in conf file")
            return False
        lines.insert(mysqld_idx + 1, MySQLStrConstant.SKIP_SLAVE_START + os.linesep)
        MysqlUtils.save_file(mycnf_file, lines)
        return True

    @staticmethod
    def disable_skip_slave_start(is_new_location: bool, my_cnf_path: str):
        if not is_new_location:
            return True
        ret, mycnf_file = MysqlBaseUtils.find_mycnf_path(my_cnf_path)
        if not ret:
            log.error("Failed to get my.cnf")
            return False
        with open(mycnf_file) as cnf_file:
            lines = cnf_file.readlines()
        new_lines = []
        for line in lines:
            if not line.startswith(MySQLStrConstant.SKIP_SLAVE_START):
                new_lines.append(line)
        MysqlUtils.save_file(mycnf_file, new_lines)
        log.info("Success to disable skip-slave-start")
        return True

    @staticmethod
    def save_last_bin_log(path, last_bin_log):
        if not check_path_valid(path, False):
            log.error("The mkdir path param is invalid: %s.", path)
            raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message=f"The file[{path}] is invalid.")
        if os.path.exists(path):
            path_type = check_file_or_dir(path)
            if path_type != EnumPathType.FILE_TYPE:
                log.error(f"Check des path not file type: {path_type}")
                raise ErrCodeException(CommErrCode.PARAMS_IS_INVALID, message=f"The file[{path}] is not file.")

        with os.fdopen(os.open(path, os.O_WRONLY | os.O_CREAT, stat.S_IWUSR | stat.S_IRUSR), 'w') as f:
            f.flush()
            f.write(last_bin_log)

    @staticmethod
    def kill_mysql_process():
        ret, out, err = execute_cmd("pidof mysqld")
        if not ret or not out:
            log.info("No mysql process")
            return True
        log.error("Found mysql process:%s", out)
        execute_cmd(f"kill -9 {out}")
        return True

    @staticmethod
    def filter_gap_gtid(gtid_purged, line):
        """
        :param gtid_purged 已回收的gtid集合
                    eg. f9dc1d93-577b-11ee-9f56-005056bb51a8:1-5133
        :param line 待处理的空洞gtid
                    eg. f9dc1d93-577b-11ee-9f56-005056bb51a8:1,5,9
        """
        line = line.strip()
        if not line:
            return ''
        idx = line.index(":")
        server_uuid = line[:idx]
        gap_tids = line[idx + 1:].split(',')
        real_gap_tids = []
        for gtid in gtid_purged.split(','):
            if not gtid.startswith(server_uuid):
                continue
            all_gtids = list()
            tids = gtid[len(server_uuid):].split(':')
            for tid_str in tids:
                if "-" not in tid_str:
                    all_gtids.append(tid_str)
                    continue
                cur_start, end = tid_str.split("-")
                all_gtids.extend([str(x) for x in range(int(cur_start), int(end) + 1)])
            for tid in gap_tids:
                if tid in all_gtids:
                    real_gap_tids.append(tid)
        if not real_gap_tids:
            return ''
        return f"{server_uuid}:{':'.join(real_gap_tids)}"

    @staticmethod
    def save_gap_gtid(file_path, lines):
        if os.path.exists(file_path):
            log.warning("Server-id file exists")
            return
        touch_file(file_path)
        log.info("Create file success")
        MysqlUtils.save_file(file_path, lines)

    @staticmethod
    def get_need_purge_gtid(master_dir, node_ids):
        need_purge_gtids = list()
        for gap_file_name in os.listdir(master_dir):
            if gap_file_name not in node_ids:
                continue
            gap_file_path = os.path.join(master_dir, gap_file_name)
            lines = MysqlUtils.read_file(gap_file_path)
            for line in lines:
                line = line.strip()
                if line:
                    need_purge_gtids.append(line.replace(',', ':'))
        return need_purge_gtids

    @staticmethod
    def get_gap_gtids(gtids):
        """
        :param gtids 已经执行的gtid集合
                eg. f9dc1d93-577b-11ee-9f56-005056bb51a8:1-5:8-5133
        """
        start = 1
        seq_len = len(gtids)
        gap_gtids = list()
        for idx in range(0, seq_len - 1):
            gtid = gtids[idx]
            uuid_idx = gtid.index(":")
            server_uuid = gtid[:uuid_idx]
            tids = gtid[uuid_idx + 1:].split(':')
            gap_tids = list()
            for tid_str in tids:
                if "-" not in tid_str:
                    cur_start, end = tid_str, tid_str
                else:
                    cur_start, end = tid_str.split("-")
                if int(cur_start) > start:
                    gap_tids.extend([str(x) for x in range(start, int(cur_start))])
                start = int(end) + 1
            if not gap_tids:
                continue
            gap_gtids.append(f"{server_uuid}:{','.join(gap_tids)}")
        return gap_gtids

    @staticmethod
    def get_data_path_user_name(my_cnf_path):
        """
        获取mysql basedir目录的属主
        """
        path = MysqlUtils.get_data_dir_by_config_file(my_cnf_path)
        if not path:
            log.error(f"Get basedir path failed.")
            return MySQLStrConstant.MYSQL
        path_old = path + RestorePath.DOTOLD
        if os.path.exists(path_old):
            stat_info = os.stat(path_old)
        else:
            stat_info = os.stat(path)
        uid = stat_info.st_uid
        user = pwd.getpwuid(uid).pw_name
        log.info(f"Get basedir user name: {user}")
        return user
