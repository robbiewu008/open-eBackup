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
import sys

from common.common import exter_attack, execute_cmd_list, execute_cmd
from common.const import ParamConstant
from common.util.check_utils import is_valid_id
from common.util.exec_utils import exec_write_new_file, exec_overwrite_file
from mysql import log
from mysql.src.common.constant import MySQLStrConstant, ExecCmdResult, MySQLClusterType, IPConstant, MySQLJsonConstant
from mysql.src.common.error_code import MySQLErrorCode
from mysql.src.common.execute_cmd import get_operating_system, get_charset_from_instance, \
    get_config_value_from_instance, validate_my_cnf, match_greatsql
from mysql.src.common.execute_cmd import safe_get_environ
from mysql.src.common.parse_parafile import ParseParaFile
from mysql.src.protect_mysql_base import MysqlBase
from mysql.src.protect_mysql_base_utils import MysqlBaseUtils
from mysql.src.resource.eapp_cluster_verify import EAppClusterVerify
from mysql.src.utils.mysql_service_info_utils import MysqlServiceInfoUtil
from mysql.src.utils.mysql_utils import MysqlUtils


@exter_attack
def query_cluster(job_pid):
    application = get_application_param(job_pid)
    result_path = os.path.join(ParamConstant.RESULT_PATH, f"result{job_pid}")
    cluster_type = application.get("extendInfo", {}).get("clusterType")
    log.info(f"Start query mysql information. mysql type: {cluster_type}")

    # 判断这个节点mysql服务是否ok，如果不ok，则直接返回
    if cluster_type == MySQLClusterType.EAPP and not MysqlUtils.eapp_is_running():
        write_error_param(application, MySQLErrorCode.ERROR_INSTANCE_IS_NOT_RUNNING, result_path)
        return
    my_cnf_path = get_config_value_from_instance(application, "myCnfPath", "")
    if my_cnf_path and not validate_my_cnf(my_cnf_path):
        write_error_param(application, MySQLErrorCode.INPUT_MY_CNF_NOT_EXIST, result_path)
        return
    mysql_ip = application.get("extendInfo", {}).get("instanceIp", "")
    mysql_base = build_mysql_instance(application, mysql_ip, job_pid)
    master_ips = []
    master_info = []
    if cluster_type == MySQLClusterType.PXC:
        is_master = check_local_is_bootstrap()
    elif cluster_type == MySQLClusterType.AP:
        is_master = not mysql_base.check_local_is_standby()
    elif cluster_type == MySQLClusterType.EAPP:
        ret, master_ips, master_info = check_eapp(mysql_base, result_path)
        if not ret:
            return
        is_master = True
    else:
        log.warning(f"Mysql cluster type not identify {cluster_type}")
        is_master = False
    # 如果是集群节点，则设置role为1，否则设置为2
    master_node_role = 1 if is_master else 2

    # 获取version
    version = mysql_base.get_mysql_version()
    if not version:
        write_error_param(application, MySQLErrorCode.ERROR_INSTANCE_IS_NOT_RUNNING, result_path)
        return
    code, out, err = execute_cmd("mysql --version")
    if code == ExecCmdResult.SUCCESS:
        if match_greatsql(out):
            version = version + "-" + MySQLStrConstant.GREATSQLAPPLICTATION
    is_running, system_service_type, mysql_service_type = MysqlServiceInfoUtil.get_mysql_service_info(cluster_type)
    if not is_running:
        system_service_type = "manual"
        mysql_service_type = "mysqld"
    # 构建返回JSON
    result_param = {
        "id": application.get("id"), "type": application.get("type"), "subType": application.get("subType"),
        "extendInfo": {
            "is_master": master_node_role, "status": "0", "dataDir": mysql_base.find_data_dir(),
            "version": version, "deployOperatingSystem": get_operating_system(),
            MySQLJsonConstant.SERVICE_NAME: mysql_service_type,
            MySQLJsonConstant.SYSTEM_SERVICE_TYPE_KEY: system_service_type,
            MySQLJsonConstant.LOG_BIN_INDEX_PATH: mysql_base.find_log_bin_path_dir(version),
            "master_list": ",".join(master_ips), "master_info": json.dumps(master_info),
            "current_ip_list": ",".join(MysqlBaseUtils.get_local_ips())
        }
    }
    log.info(f"Success to execute query cluster command. pid:{job_pid}, cluster:{result_param.get('id')}")
    exec_write_new_file(result_path, result_param)


def write_error_param(application: dict, error_code, result_path):
    """
    写入失败的返回信息到结果文件里
    :param application: 应用信息
    :param error_code: 错误返回值
    :param result_path: 结果文件
    """
    result_param = {
        "id": application.get("id"),
        "type": application.get("type"),
        "subType": application.get("subType"),
        "extendInfo": {
            "status": "1",
            "error_code": error_code
        }
    }
    log.info(f"result_param:{result_param}")
    exec_overwrite_file(result_path, result_param)


def check_eapp(mysql_base, result_path):
    error_code = EAppClusterVerify(mysql_base).check()
    if not error_code:
        master_ips = mysql_base.get_master_ips()
        master_info = get_master_info(mysql_base)
        return True, master_ips, master_info
    else:
        exec_write_new_file(result_path, {"extendInfo": {"error_code": error_code}})
        return False, [], []


def get_master_info(mysql_base):
    ret, master_info_list = mysql_base.get_master_info()
    if not ret:
        log.error("Failed to get master info")
        return []
    master_info = []
    for item in master_info_list:
        master_info.append({"host": item.master_host, "port": item.master_port})
    return master_info


def get_application_param(job_pid):
    context = ParseParaFile.parse_para_file(job_pid)
    log.info(f"Begin to query cluster pid: {job_pid}")
    application = context.get("application", {})
    return application


def build_mysql_instance(application, mysql_ip, job_pid):
    mysql_base = MysqlBase("", "", "", {})
    mysql_base.set_mysql_port(application.get("auth", {}).get("extendInfo", {}).get("instancePort"))
    mysql_base.set_mysql_charset(get_charset_from_instance(application))
    mysql_base.set_mysql_ip(mysql_ip if mysql_ip else IPConstant.LOCALHOST)
    mysql_base.set_mysql_user(safe_get_environ(f"application_auth_authKey_{job_pid}"))
    mysql_base.set_mysql_pwd(f"application_auth_authPwd_{job_pid}")
    return mysql_base


def check_local_is_bootstrap():
    """
    PXC集群的节点，区分是集群节点，还是普通节点。前提是节点状态必须是启动起来了的
    """
    ret, _, _ = execute_cmd_list([f"systemctl status {MySQLStrConstant.MYSQLPXCSERVICES}", "grep \"Active: active\""])
    if ret == ExecCmdResult.SUCCESS:
        return True
    else:
        return False


if __name__ == '__main__':
    pid = sys.argv[1]
    if not is_valid_id(pid):
        log.warn(f'req_id[{pid}] is invalid')
        sys.exit(1)
    query_cluster(pid)
