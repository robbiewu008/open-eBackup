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

import configparser
import os
import time

from mysql import log
from common.util.cmd_utils import cmd_format
from common.util.exec_utils import su_exec_rm_cmd, exec_overwrite_file, check_path_valid
from common.common import execute_cmd, execute_cmd_list, execute_cmd_list_communicate
from common.const import ParamConstant, IPConstant
from mysql.src.common.constant import MySQLStrConstant, MySQLConfigPath, MySQLPreTableLockStatus, \
    ExecCmdResult, RestorePath, MySQLJsonConstant, MySQLClusterType, RestoreType, MysqlStatusStr, RoleType


def get_lock_ddl_param():
    if not os.path.exists(MySQLConfigPath.CONFIGPATH):
        return MySQLPreTableLockStatus.OFF
    # 创建ConfigParser对象
    conf = configparser.ConfigParser()
    # 读取文件内容
    conf.read(MySQLConfigPath.CONFIGPATH)
    try:
        config = conf.get('mysql', 'lock-ddl-per-table')
    except Exception as except_str:
        log.error(f"Get mysql lock from config failed.{except_str}")
        return MySQLPreTableLockStatus.OFF
    if config not in (MySQLPreTableLockStatus.ON, MySQLPreTableLockStatus.OFF):
        log.error("Mysql per-table lock parameter settings are incorrect.")
        return MySQLPreTableLockStatus.OFF
    return config


def delete_files_in_meta_dir(meta_path):
    for path in os.listdir(meta_path):
        new_path = os.path.join(meta_path, path)
        if not su_exec_rm_cmd(new_path):
            log.error(f"Rm file failed. path:[{new_path}]")
        return


def check_tool_exist(tool_name):
    cmd_exec_err = False
    ret = int(ExecCmdResult.SUCCESS)
    cmd_str = cmd_format("{} --version", tool_name)
    try:
        ret, read, output = execute_cmd(cmd_str)
    except Exception as exception_str:
        log.error(f"check_tool_exists:{exception_str}")
        cmd_exec_err = True
    if ret == ExecCmdResult.UNKNOWN_CMD or cmd_exec_err:
        log.error(f"No {tool_name} found.")
        return False
    # 后续将工具与数据库的匹配表测出来后再处理
    return True


class MysqlBaseUtils(object):

    @staticmethod
    def parse_instance_restore_type(cluster_type, role_type):
        if cluster_type == MySQLClusterType.PXC:
            if role_type == RoleType.ACTIVE_NODE:
                return True, RestoreType.PXC_INSTANCE_CLUSTER_NODE
            elif role_type == RoleType.STANDBY_NODE:
                return True, RestoreType.PXC_INSTANCE_COMMON_NODE
            else:
                log.error(f"Instance PXC's role type: {role_type} is wrong.")
                return False, RestoreType.EMPTY
        elif cluster_type == MySQLClusterType.AP:
            if role_type == RoleType.ACTIVE_NODE:
                return True, RestoreType.AP_INSTANCE_CLUSTER_MASTER_NODE
            elif role_type == RoleType.STANDBY_NODE:
                return True, RestoreType.AP_INSTANCE_CLUSTER_SLAVE_NODE
            else:
                log.error(f"Instance AP's role type: {cluster_type} is wrong.")
                return False, RestoreType.EMPTY
        elif cluster_type == MySQLClusterType.EAPP:
            return True, RestoreType.EAPP_INSTANCE
        else:
            log.error(f"Instance cluster type: {role_type} is wrong.")
            return False, RestoreType.EMPTY

    @staticmethod
    def read_file(file_path):
        if not file_path or not os.path.exists(file_path):
            return ""
        with open(file_path, "r", encoding="utf-8") as f:
            line = f.read()
        return line

    @staticmethod
    def deal_with_mysqlbinlog_output(out_str):
        if "Failed on connect:" in out_str:
            log.error("Failed to connect to MySQL server: Access denied")
            return True
        if "user" not in out_str:
            log.error(f"Mysqlbinlog err:{out_str}")
        else:
            log.error("Mysqlbinlog exec error.")
        return True

    @staticmethod
    def clean_dir(dir_path):
        for path in os.listdir(dir_path):
            new_path = os.path.join(dir_path, path)
            if not su_exec_rm_cmd(new_path, check_white_black_list_flag=False):
                log.error(f"Rm file failed. path:[{new_path}]")

    @staticmethod
    def check_and_del_target_dir(target_dir_path):
        """
        查询目标目录是否存在，存在就删除
        :return:
        """
        if os.path.exists(target_dir_path):
            if os.path.isdir(target_dir_path):
                MysqlBaseUtils.clean_dir(target_dir_path)
                log.info(f"rmtree target_dir_path success.")

    @staticmethod
    def find_mycnf_path(my_cnf_path):
        """
        查找my.cnf文件路径
        :return:
        """

        def parse_file():
            for line in output.splitlines(False):
                if "my.cnf" not in line:
                    continue
                for i in line.split(" "):
                    if os.access(i, os.F_OK):
                        return True, i
            return False, ""

        if my_cnf_path:
            if check_path_valid(my_cnf_path, False):
                return True, my_cnf_path
            return False, ""
        # 命令硬编码，使用subprocess.getstatusoutput接口无安全风险
        ret, output, _ = execute_cmd_list_communicate(["mysql --help", "grep -A 1 'Default options'", "grep my.cnf"])
        if ret == ExecCmdResult.SUCCESS:
            # 过滤一些警告信息
            result, file_path = parse_file()
            if result:
                return True, file_path
        return False, ""

    @staticmethod
    def get_dir_size(dir_path):
        """
        求文件夹的总大小
        :return:
        """
        size = 0
        lst = os.listdir(dir_path)
        for i in lst:
            new_path = os.path.join(dir_path, i)
            if os.path.isfile(new_path):
                size += os.path.getsize(new_path)
            elif os.path.isdir(new_path):
                # 使用递归(副本目录不会有很多层，无溢出风险)
                size += MysqlBaseUtils.get_dir_size(new_path)
            else:
                continue
        return size

    @staticmethod
    def query_restore_process_alive():
        cmd_str_xtrabackup = ["ps -ef", "grep {self._job_id}", "grep xtrabackup", "grep -v grep"]
        cmd_str_mysqlbinlog = ["ps -ef", "grep {self._job_id}", "grep mysqlbinlog", "grep -v grep"]
        ret_xtrabackup, output_xtrabackup, _ = execute_cmd_list(cmd_str_xtrabackup)
        ret_mysqlbinlog, output_mysqlbinlog, _ = execute_cmd_list(cmd_str_mysqlbinlog)
        if ret_xtrabackup == ExecCmdResult.SUCCESS or ret_mysqlbinlog == ExecCmdResult.SUCCESS:
            return True, f"{output_xtrabackup}{output_mysqlbinlog}"
        return False, ""

    @staticmethod
    def check_mysql_bootstrap_service_run_status():
        ret, output, _ = execute_cmd(f"systemctl status {MySQLStrConstant.MYSQLPXCSERVICES}")
        if not output:
            return False
        if MysqlStatusStr.STOP not in output and MysqlStatusStr.FAILED not in output:
            log.error(f"Mysql service {MySQLStrConstant.MYSQLPXCSERVICES} status error.")
            return True
        return False

    @staticmethod
    def parse_db_restore_type(cluster_type, role_type):
        if cluster_type == MySQLClusterType.PXC:
            if role_type == RoleType.ACTIVE_NODE:
                return True, RestoreType.PXC_DB_CLUSTER_NODE
            elif role_type == RoleType.STANDBY_NODE:
                return True, RestoreType.PXC_DB_COMMON_NODE
            else:
                log.error(f"DB PXC's role type: {role_type} is wrong.")
                return False, RestoreType.EMPTY
        elif cluster_type == MySQLClusterType.AP:
            if role_type == RoleType.ACTIVE_NODE:
                return True, RestoreType.AP_DB_CLUSTER_MASTER_NODE
            elif role_type == RoleType.STANDBY_NODE:
                return True, RestoreType.AP_DB_CLUSTER_SLAVE_NODE
            else:
                log.error(f"DB AP's role type: {cluster_type} is wrong.")
                return False, RestoreType.EMPTY
        else:
            log.error(f"DB cluster type: {role_type} is wrong.")
            return False, RestoreType.EMPTY

    @staticmethod
    def get_local_ips():
        """
        获取本机的IP地址
        :return:
        """
        ret, output, _ = execute_cmd_list(["ip addr", "grep 'inet '", "awk '{print $2}'"], True)
        if ret == ExecCmdResult.SUCCESS:
            ips = [ip.split('/')[0] for ip in output.split('\n')]
            return ips
        return []

    @staticmethod
    def read_mysql_data_dir(cnf_file_path):
        mysql_storage_dir = ""
        with open(cnf_file_path, 'r', encoding='utf-8') as file_read:
            for line in file_read.readlines():
                line_tmp = line.rstrip()
                if not line_tmp:
                    continue
                list_tmp = line_tmp.split("=")
                if list_tmp and list_tmp[0] == RestorePath.DATADIR:
                    mysql_storage_dir = list_tmp[1]
                    break
        return mysql_storage_dir

    @staticmethod
    def parse_restore_subtype_and_cluster_type(job_json):
        target_object_json = job_json.get(MySQLJsonConstant.TARGETOBJECT, {})
        if not target_object_json:
            log.error("Get target object json failed.")
            return False, "", ""
        sub_type = target_object_json.get(MySQLJsonConstant.SUBTYPE, "")
        if not sub_type:
            log.error("Get sub type json failed.")
            return False, "", ""
        log.debug(f"Get sub type: {sub_type}")
        extend_info = target_object_json.get(MySQLJsonConstant.EXTENDINFO, {})
        if not extend_info:
            log.error("Get target object's extend info json failed.")
            return False, sub_type, ""
        cluster_type = extend_info.get(MySQLJsonConstant.CLUSTERTYPE, "")
        log.debug(f"Get cluster type: {cluster_type}")
        return True, sub_type, cluster_type

    @staticmethod
    def del_dot_old_dir_or_file(mysql_storage_dir):
        for file in os.listdir(mysql_storage_dir):
            if file.endswith(RestorePath.DOTOLD):
                path_file = os.path.join(mysql_storage_dir, file)
                if os.path.islink(path_file):
                    log.warning(f"Path has link.")
                    continue
                if not su_exec_rm_cmd(path_file, check_white_black_list_flag=False):
                    log.error(f"Rm file failed. path:[{path_file}]")

    @staticmethod
    def get_port_and_ip_from_target_object(job_json):
        target_object_json = job_json.get(MySQLJsonConstant.TARGETOBJECT, {})
        if not target_object_json:
            log.error("Get target object json failed.")
            return False, "", ""
        mysql_ip = target_object_json.get(MySQLJsonConstant.EXTENDINFO, {}). \
            get(MySQLJsonConstant.INSTANCEIP, "")
        if not mysql_ip:
            mysql_ip = IPConstant.LOCAL_HOST
        auth_json = target_object_json.get(MySQLJsonConstant.AUTH, {})
        if not auth_json:
            log.error("Get auth json failed.")
            return False, "", ""
        auth_extend_json = auth_json.get(MySQLJsonConstant.EXTENDINFO, {})
        if not auth_extend_json:
            log.error("Get auth extend json failed.")
            return False, "", ""
        port = auth_extend_json.get(MySQLJsonConstant.INSTANCEPORT, "")
        if not port:
            return False, "", ""
        log.debug(f"Get port: {port}")
        return True, port, mysql_ip

    @staticmethod
    def get_lock_time(mysql_version, output):
        # 锁定秒数 默认设置为0
        second = 0
        # mariaDB
        if MySQLStrConstant.MARIADB in mysql_version:
            # 根据Mariadb备份输出结果进行解析
            temp_start_str = output[0:output.find("Starting to backup non-InnoDB tables and files")]
            # 截取字符串的结果为 (\n[00] 2022-12-27 23:36:32 )故下方下标加6 和 -1
            start = temp_start_str[temp_start_str.rfind("\n") + 6:-1]

            temp_end_str = output[0:output.find("All tables unlocked")]
            end = temp_end_str[temp_end_str.rfind("\n") + 6:temp_end_str.rfind("ALl")]
            second = time.mktime(time.strptime(end, '%Y-%m-%d %H:%M:%S')) - \
                     time.mktime(time.strptime(start, '%Y-%m-%d %H:%M:%S'))
            total = time.strftime("%H:%M:%S", time.gmtime(second))
        # MySQL8.x
        elif mysql_version[:1] == '8' and MySQLStrConstant.MARIADB not in mysql_version:
            # 根据MySQL8.x版本备份输出结果进行解析
            temp_start_str = output[0:output.find("Starting to backup non-InnoDB tables and files")]
            # 截取字符串结果为\\r\\n2022-12-22T17:11:30.440603+08:00
            start = temp_start_str[temp_start_str.rfind("\\r\\n") + 4:temp_start_str.rfind(".")]

            temp_end_str = output[0:output.find("All tables unlocked")]
            end = temp_end_str[temp_end_str.rfind("\\r\\n") + 4:temp_end_str.rfind(".")]
            second = time.mktime(time.strptime(end, '%Y-%m-%dT%H:%M:%S')) - \
                     time.mktime(time.strptime(start, '%Y-%m-%dT%H:%M:%S'))
            total = time.strftime("%H:%M:%S", time.gmtime(second))
        # MySQL5.x
        elif mysql_version[:1] == '5' and MySQLStrConstant.MARIADB not in mysql_version:
            # 根据MySQL5.x版本备份输出结果进行解析
            temp_start_str = output[0:output.find("Starting to backup non-InnoDB tables and files")]
            start = temp_start_str[temp_start_str.rfind("\\r\\n") + 4:-1]

            temp_end_str = output[0:output.find("All tables unlocked")]
            end = temp_end_str[temp_end_str.rfind("\\r\\n") + 4:-1]
            second = time.mktime(time.strptime("20" + end, '%Y%m%d %H:%M:%S')) - \
                     time.mktime(time.strptime("20" + start, '%Y%m%d %H:%M:%S'))
            total = time.strftime("%H:%M:%S", time.gmtime(second))

        # 如果数据量很小的情况下，锁表时间设置为1秒
        if second == 0:
            return "00:00:01"
        return str(total)

    @staticmethod
    def output_action_result_ex(pid, code, body_err, message):
        """
        将actionResult写入到结果文件,供框架读取
        :return:
        """
        json_result = {
            "code": code,
            "bodyErr": body_err,
            "message": message
        }
        file_path = os.path.join(ParamConstant.RESULT_PATH, f"result{pid}")
        log.info(f"Write file. result:{code} {body_err} {message}")
        exec_overwrite_file(file_path, json_result)

    def get_host_sn(self):
        host_sn = self.read_file(RestorePath.HOST_SN_FILE_PATH)
        host_sn = host_sn.strip("\n")
        return host_sn

    def get_port_and_ip_from_target_env(self, job_json):
        mysql_ip = IPConstant.LOCAL_HOST
        target_env_json = job_json.get(MySQLJsonConstant.TARGETENV, {})
        if not target_env_json:
            log.error("Get target env json failed.")
            return False, "", ""
        nodes_json = target_env_json.get(MySQLJsonConstant.NODES, [])
        if not nodes_json:
            log.error("Get nodes json failed.")
            return False, "", ""
        local_ips = self.get_local_ips()
        if not local_ips:
            log.error("Get local ips failed.")
            return False, "", ""
        for element in nodes_json:
            ip_record = element.get(MySQLJsonConstant.ENDPOINT, "")
            if not ip_record:
                log.error("Get target env nodes ip record failed.")
                return False, "", ""
            if ip_record not in local_ips:
                continue
            instance_ip = element.get(MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.INSTANCEIP, "")
            if instance_ip:
                mysql_ip = instance_ip
            auth_json = element.get(MySQLJsonConstant.AUTH, {})
            if not auth_json:
                log.error("Get auth json failed.")
                return False, "", ""
            auth_extend_json = auth_json.get(MySQLJsonConstant.EXTENDINFO, {})
            if not auth_extend_json:
                log.error("Get auth extend json failed.")
                return False, "", ""
            port = auth_extend_json.get(MySQLJsonConstant.INSTANCEPORT, "")
            if port:
                log.debug(f"Get port: {port} ip: {instance_ip}")
                return True, port, mysql_ip
            log.error("Get port json failed.")
            return False, "", ""
        log.error("Get port failed.")
        return False, "", ""
