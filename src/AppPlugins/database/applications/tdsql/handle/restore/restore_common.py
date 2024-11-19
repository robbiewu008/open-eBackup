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

import glob
import os
import pwd
import shutil
import stat
import subprocess
from datetime import datetime

import pexpect
import pymysql

from common.common import execute_cmd, clean_dir, touch_file
from common.common_models import LogDetail, SubJobDetails
from common.const import CMDResult, SubJobStatusEnum
from common.util.check_utils import is_ip_address
from tdsql.common.const import TdsqlRestoreSubJobName, TdsqlClusterGroupRestoreSubJobName, MySQLVersion
from tdsql.common.tdsql_common import extract_ip, write_file, execute_cmd_list, report_job_details
from tdsql.common.util import get_tdsql_status, get_xtrabackup_tool_path, is_valid_port, \
    get_backup_pre_from_defaults_file_path
from tdsql.handle.common.const import TDSQLReportLabel
from tdsql.logger import log


def get_mysql_install_path(mysql_conf_path):
    mysql_path = mysql_conf_path.split("/etc")[0]
    log.info(f"mysql_path is {mysql_path}")
    return mysql_path


def get_zk_install_path():
    return '/data/application/scheduler/bin'


def get_oc_agent_path():
    return '/data/oc_agent/bin'


def check_mysql_version():
    return True


def check_mysql_status(port):
    cmd_list = ["ps -ef", "grep mysqld", f"grep {port}", "grep -v grep", "wc -l"]
    return_code, out_info, err_info = execute_cmd_list(cmd_list)
    if return_code != CMDResult.SUCCESS:
        log.error(f"The execute check_mysql_status cmd failed!")
        log.info(f"out_info: {out_info} err_info: {err_info}")
        return False
    if int(out_info) > 0:
        log.info("mysqld service running")
        return True
    else:
        log.warn("mysqld service stopped")
        return False


def get_sub_job_name(param):
    """
    获取sub job执行函数名称
    :param param:
    :return:
    """
    sub_name = param.get("subJob", {}).get("jobName", "")
    log.info("get_sub_job_name", sub_name)
    if sub_name not in (TdsqlRestoreSubJobName.SUB_EXEC_PREPARE, TdsqlRestoreSubJobName.SUB_CHECK_MYSQL_VERSION,
                        TdsqlRestoreSubJobName.SUB_EXEC_RESTORE, TdsqlRestoreSubJobName.SUB_EXEC_CREATE_INSTANCE):
        log.error(f"{sub_name} not found in sub jobs!")
        return ""
    return sub_name


def get_cluster_group_sub_job_name(param):
    """
    获取sub job执行函数名称
    :param param:
    :return:
    """
    sub_name = param.get("subJob", {}).get("jobName", "")
    log.info("get_sub_job_name", sub_name)
    if sub_name not in (
            TdsqlClusterGroupRestoreSubJobName.SUB_EXEC_MOUNT,
            TdsqlClusterGroupRestoreSubJobName.SUB_CHECK_MYSQL_VERSION,
            TdsqlClusterGroupRestoreSubJobName.SUB_CHECK_HOST_AGENT,
            TdsqlClusterGroupRestoreSubJobName.SUB_EXEC_RESTORE,
            TdsqlClusterGroupRestoreSubJobName.SUB_EXEC_UMOUNT):
        log.error(f"{sub_name} not found in sub jobs!")
        return ""
    return sub_name


def need_log_restore(pid, job_id, json_param):
    job_json = json_param.get("job", {})
    if not job_json:
        log.error("Get job json failed.")
        return False
    extend_info_json = job_json.get("extendInfo", {})
    if not extend_info_json:
        log.error(f"Get extend info failed. pid:{pid} jobId:{job_id}")
        return False
    restore_time_stamp = extend_info_json.get("restoreTimestamp", "")
    if not restore_time_stamp:
        log.warn(f"Get restore time stamp failed. pid:{pid} jobId:{job_id}")
        return False
    log.debug(f"Get restore time stamp： {restore_time_stamp}")
    return True


def write_progress_file(message: str, file_name: str):
    log.info(f'Write message into progress file: {message}')
    flags = os.O_WRONLY | os.O_CREAT
    modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
    with os.fdopen(os.open(file_name, flags, modes), 'a+') as out_file:
        out_file.write(message)
        out_file.write('\n')


def rename_dir_or_file(src_name, dst_name):
    if os.path.islink(src_name):
        log.warning(f"Path has link.")
        return
    if os.path.exists(src_name):
        os.rename(src_name, dst_name)


def execute_command(cmd, timeout):
    try:
        process = pexpect.spawn(f'bash -c "{cmd}"', timeout=timeout)
        ret = process.expect(pexpect.EOF)
        log.warn(process.before)
    except Exception as ex:
        log.error(f"Spawn except an exception {ex}.")
        return False
    finally:
        if process:
            process.close()
    if ret != 0:
        log.info(f"Failed to execute_command: {cmd}.")
        return False
    return True


def check_permission(data_dir, os_user):
    cmd = f'chown -R tdsql:users {data_dir}'
    return_code, out_info, err_info = execute_cmd(cmd)
    if return_code != CMDResult.SUCCESS:
        log.error(f"The execute check_permission cmd failed!")
        log.info(f"out_info: {out_info} err_info: {err_info}")
        return False
    pw_uid = pwd.getpwnam(os_user).pw_uid
    data_uid = os.stat(data_dir).st_uid
    if pw_uid != data_uid:
        log.error("data file permission not correct")
        return False
    return True


def start_mysql_service(port, mysql_conf_path):
    mysql_path = get_mysql_install_path(mysql_conf_path)
    cmd = f'cd {mysql_path}/install && sh {mysql_path}/install/startmysql_cgroup.sh  {port}'
    try:
        execute_command(cmd, 300)
    except Exception as ex:
        log.debug(f"start_mysql_service failed ex:{ex}")
        return False
    return True


def start_oc_agent():
    oc_agent_path = get_oc_agent_path()
    cmd = f'cd {oc_agent_path} && sh {oc_agent_path}/start_agent.sh'
    try:
        execute_command(cmd, 300)
    except Exception as ex:
        log.debug(f"start_oc_agent failed ex:{ex}")
        return False
    return True


def start_crontab():
    cmd = 'service crond restart'
    return_code, out_info, err_info = execute_cmd(cmd)
    # 执行命令停止服务失败
    if return_code != CMDResult.SUCCESS:
        log.error(f"The execute start crontab cmd failed!")
        log.info(f"out_info: {out_info} err_info: {err_info}")
        return False
    return True


def stop_mysql_service(port, mysql_conf_path):
    mysql_path = get_mysql_install_path(mysql_conf_path)
    cmd = f'cd {mysql_path}/install && sh stopmysql.sh {port}'
    try:
        execute_command(cmd, 300)
    except Exception as ex:
        log.debug(f"stop_mysql_service failed ex:{ex}")
        return False
    return True


def stop_crontab():
    cmd = 'service crond stop'
    return_code, out_info, err_info = execute_cmd(cmd)
    # 执行命令停止服务失败
    if return_code != CMDResult.SUCCESS:
        log.error(f"The execute stop crontab service cmd failed!")
        log.info(f"out_info: {out_info} err_info: {err_info}")
        return False
    return True


def stop_oc_agent():
    oc_agent_path = get_oc_agent_path()
    cmd = f'cd {oc_agent_path} && sh {oc_agent_path}/stop_agent.sh'
    try:
        execute_command(cmd, 300)
    except Exception as ex:
        log.debug(f"stop_oc_agent failed ex:{ex}")
        return False
    cmd_list = ["ps -ef", "grep ewp", "grep -v grep", "wc -l"]
    return_code, out_info, err_info = execute_cmd_list(cmd_list)
    if return_code != CMDResult.SUCCESS:
        log.error(f"The execute stop_oc_agent cmd failed!")
        log.info(f"out_info: {out_info} err_info: {err_info}")
        return False
    return True


def stop_mysqlagent(mysql_port, mysql_conf_path):
    backup_pre = get_backup_pre_from_defaults_file_path(mysql_conf_path)
    mysqlagent_path = os.path.join(backup_pre, mysql_port, "mysqlagent/bin")
    log.info(f"stop_mysqlagent mysqlagent_path {mysqlagent_path}")
    cmd = f'cd {mysqlagent_path} && sh stopreport.sh ../conf/mysqlagent_{mysql_port}.xml'
    try:
        execute_command(cmd, 300)
    except Exception as ex:
        log.debug(f"stop_mysqlagent failed ex:{ex}")
        return False
    return True


def xtrabackup_prepare(port, target_dir, mysql_conf_path, mysql_version):
    mysql_path = get_mysql_install_path(mysql_conf_path)
    channel_number = get_cpu_number()
    xtrabackup_tool_path = get_xtrabackup_tool_path(mysql_version)
    log.info(f"xtrabackup_tool_path {xtrabackup_tool_path}")
    cmd = f'{mysql_path}/{xtrabackup_tool_path} --prepare --parallel={channel_number}  --target-dir={target_dir}'
    try:
        log.info(f'target_dir:{target_dir}')
        log.info(f'cmd: {cmd}')
        execute_command(cmd, None)
    except Exception as ex:
        log.debug(f"xtrabackup_prepare {ex}")
        return False
    return True


def xtrabackup_restore(port, target_dir, mysql_conf_path, mysql_version):
    mysql_path = get_mysql_install_path(mysql_conf_path)
    channel_number = get_cpu_number()
    xtrabackup_tool_path = get_xtrabackup_tool_path(mysql_version)
    log.info(f"xtrabackup_tool_path {xtrabackup_tool_path}")
    cmd = f"{mysql_path}/{xtrabackup_tool_path} \
          --defaults-file={mysql_conf_path} \
          --parallel={channel_number} --copy-back --target-dir={target_dir}"
    try:
        log.info(f'target_dir:{target_dir}')
        log.info(f'cmd: {cmd}')
        execute_command(cmd, None)
    except Exception as ex:
        log.debug(f"xtrabackup_restore {ex}")
        return False
    return True


def get_data_dir(socket):
    data_dir_list = socket.split('/')[0:-2]
    data_dir = "/".join(data_dir_list)
    log.info(f'data_dir: {data_dir}')
    return data_dir.strip()


def get_dblogs_path(myconf_file):
    cmd_list = [f"cat {myconf_file}", "grep 'log dir is'", "head -n 1", "awk -F '=' '{print $2}'"]
    return_code, out_info, err_info = execute_cmd_list(cmd_list)
    if return_code != CMDResult.SUCCESS:
        log.error(f"The get_dblogs_path failed!")
        log.info(f"out_info: {out_info} err_info: {err_info}")
        return False, ""
    dblogs_path = out_info.strip()
    if not os.path.exists(f'{dblogs_path}'):
        log.error(f"The dblogs_path {dblogs_path} not exists")
        return False, ""
    return True, dblogs_path


def mv_data_and_log_dir(data_dir, dblogs_path, mysql_conf_path):
    suffix = '.old'
    innodb_log_arch_dir = get_innodb_log_arch_dir(mysql_conf_path)
    path_list = (
        f'{data_dir}/logs', f'{data_dir}/dbdata_raw/data',
        f'{data_dir}/dbdata_raw/dbdata', f'{dblogs_path}/bin', f'{innodb_log_arch_dir}'
    )
    for path in path_list:
        log.info(f"path {path}")
        if not os.path.exists(path + suffix) and os.path.exists(path):
            try:
                rename_dir_or_file(path, path + suffix)
            except Exception as ex:
                log.warn(f"rename {path} failed ex:{ex}")
        # 防止path存在的情况，做一次clean动作
        if os.path.exists(path):
            clean_dir(path)
    return True


def get_innodb_log_arch_dir(mysql_conf_path):
    with open(mysql_conf_path, "r", encoding='utf-8') as tmp:
        lines = tmp.readlines()

    innodb_log_group_home_dir_line = ''
    for line in lines:
        if line and '#' not in line and "innodb_log_group_home_dir" in line:
            innodb_log_group_home_dir_line = line.strip()
            break

    innodb_log_group_home_dir = ''
    if len(innodb_log_group_home_dir_line.split("=")) > 1:
        innodb_log_group_home_dir = innodb_log_group_home_dir_line.split("=")[1].strip()
    log.info(f"clear_innodb_log_arch_dir innodb_log_group_home_dir {innodb_log_group_home_dir}")
    return innodb_log_group_home_dir


def get_master(url, set_id, task_type, pid):
    body = get_tdsql_status(url, set_id, task_type, pid)
    for i in body:
        for j in dict(i).get("db"):
            if j.get('master') == 1:
                log.info("url", j.get('ip', ""))
                return True, j.get('ip', "")
    return False, ""


def get_url(oss_nodes):
    for node in oss_nodes:
        ip = node.get("ip", "")
        port = node.get("port")
        if not is_ip_address(ip):
            log.error(f'ip_address is Illegal {ip}')
            raise Exception(f'ip_address is Illegal {ip}')
        if not is_valid_port(port):
            raise Exception(f"port is Illegal {port}")
        if ip and port:
            return True, f'http://{ip}:{port}/tdsql'
    log.error("get oss url fail")
    return False, ""


def fetch_user_and_pwd(param):
    info = param.get("subJob", "").get("jobInfo", "")
    user_pwd = info.split(" ")
    user = user_pwd[2]
    password = user_pwd[3]
    return user, password


def get_port_and_ip(nodes):
    mysql_ip = "127.0.0.1"
    nodes_json = nodes
    if not nodes_json:
        log.error("Get nodes json failed.")
        return False, "", ""
    local_ips = extract_ip()
    if not local_ips:
        log.error("Get local ips failed.")
        return False, "", ""
    for element in nodes_json:
        ip_record = element.get("ip", "")
        if not ip_record:
            log.error("Get nodes ip record failed.")
            return False, "", ""
        if ip_record not in local_ips:
            continue
        if ip_record:
            mysql_ip = ip_record
        port = element.get("port", "")
        if port:
            return True, port, mysql_ip
        log.error("Get port and socket json failed.")
        return False, "", ""
    log.error("Get port failed.")
    return False, "", ""


def clean_backup_dir(data_dir, dblogs_path, mysql_conf_path):
    suffix = '.old'
    innodb_log_arch_dir = get_innodb_log_arch_dir(mysql_conf_path)
    path_list = (
        f'{data_dir}/logs', f'{data_dir}/dbdata_raw/data', f'{data_dir}/dbdata_raw/dbdata', f'{dblogs_path}/bin',
        f'{innodb_log_arch_dir}'
    )
    for path in path_list:
        log.info(f"path {path}{suffix}")
        if os.path.exists(path + suffix):
            try:
                clean_dir(path + suffix)
                os.rmdir(path + suffix)
            except Exception as ex:
                log.warn(f"clean {path}{suffix} failed ex:{ex}")
    log.info(f"clean dir success")
    return True


def copy_back_cnf_and_pem_file(data_dir):
    suffix = '.old'
    data_dir_data = os.path.join(data_dir, 'dbdata_raw', 'data')
    data_back_path = f'{data_dir_data}{suffix}'
    log.info(f"copy back data_dir_data {data_dir_data} data_back_path {data_back_path}")
    if os.path.exists(f'{data_back_path}/auto.cnf'):
        db_files = glob.glob(f"{data_back_path}/*.pem")
        for file in db_files:
            cmd = f'cp -rf {file} {data_dir_data}'
            return_code, out_info, err_info = execute_cmd(cmd)
            if return_code != CMDResult.SUCCESS:
                log.error(f"The execute copy back cnf and pem cmd failed!")
                log.info(f"out_info: {out_info} err_info: {err_info}")
                return False

        cmd = f'cp -rf {data_back_path}/auto.cnf {data_dir_data}'
        return_code, out_info, err_info = execute_cmd(cmd)
        # 执行命令停止服务失败
        if return_code != CMDResult.SUCCESS:
            log.error(f"The execute copy back cnf and pem cmd failed!")
            log.info(f"out_info: {out_info} err_info: {err_info}")
            return False
        log.info("copy_back_cnf_and_pem_file success")
    return True


def deal_xtrabackup_error(output):
    """
    处理命令的错误输出
    """
    try:
        for err_flag in ["[FATAL]", "Error:"]:
            index = output.find(err_flag)
            if index != -1:
                log.error(f"CMD error:{output[index:]}")
                return
    except Exception as ex:
        log.error(f"Deal cmd error failed.{ex}")


def get_register_ip(host_ip):
    """
    获取当前节点注册ip
    :return:
    """
    cur_host_ips = set(extract_ip())
    remote_ips = set(host_ip)
    result_set = cur_host_ips & remote_ips
    log.debug(f"ip result_set: {result_set}")
    if not result_set or len(result_set) != 1:
        log.error(f"Get register ip failed, result_set count: {len(result_set)}.")
        return ""
    return result_set.pop()


def get_nodes_info(param):
    """
    获取要恢复的目标集群使用的代理主机
    """
    target_env = param.get("job", "").get("targetEnv", "")
    if not target_env:
        log.error(f"Fail to get target env")
        return []
    nodes = target_env.get("nodes", "")
    if not nodes or len(nodes) == 0:
        log.error(f"Fail to get nodes info")
        return []
    nodes_list = []
    for node in nodes:
        if node.get("type", "") == "Host":
            nodes_list.append(node.get("endpoint", ""))
        else:
            continue
    return nodes_list


def get_cpu_number():
    exec_cmd = "cat /proc/cpuinfo | grep processor | wc -l"
    ret, output = subprocess.getstatusoutput(exec_cmd)
    if not ret:
        return int(output)
    else:
        return 8


def get_deploy_conf_path(port):
    oc_agent_bin_path = get_oc_agent_path()
    parent_dir = os.path.abspath(os.path.join(oc_agent_bin_path, os.pardir))
    oc_agent_log_path = os.path.join(parent_dir, "log")
    conf_file = f"{port}_deploy.conf"
    oc_agent_deploy_conf_path = os.path.join(oc_agent_log_path, conf_file)
    return oc_agent_deploy_conf_path


def create_deploy_conf(port):
    oc_agent_deploy_conf_path = get_deploy_conf_path(port)
    log.info(f"create oc_agent_deploy_conf_path is {oc_agent_deploy_conf_path}")
    content = "1|" + datetime.max.strftime('%Y-%m-%d %H:%M:%S')
    try:
        write_file(oc_agent_deploy_conf_path, content)
    except Exception as ex:
        exception_message = str(ex)
        if "no space left on device" in exception_message.lower():
            raise ex
        log.error(f"create oc_agent {port}_deploy.conf failed ex:{exception_message}")
        return False
    log.info(f"create oc_agent {port}_deploy.conf end")
    return True


def remove_deploy_conf(port):
    oc_agent_deploy_conf_path = get_deploy_conf_path(port)
    log.info(f"delete oc_agent_deploy_conf_path is {oc_agent_deploy_conf_path}")
    if not os.path.exists(oc_agent_deploy_conf_path):
        return True
    if os.path.isfile(oc_agent_deploy_conf_path):
        try:
            os.remove(oc_agent_deploy_conf_path)
        except Exception as e:
            log.error(f"Fail to delete file {oc_agent_deploy_conf_path} for {e}.")
            return False
    log.info(f"delete oc_agent_deploy_conf_path {oc_agent_deploy_conf_path} success")
    return True


def create_binlog_index(dblogs_path, mysql_version):
    """
    适配mariadb, 恢复时需要手动创建binlog.index文件
    :return:
    """
    if not need_create_binlog_index(mysql_version):
        log.warn("no need to create index file")
        return True
    binlog_bin_dir = os.path.join(dblogs_path, "bin")
    if not os.path.exists(binlog_bin_dir):
        log.info("create binlog bin dir")
        os.mkdir(binlog_bin_dir)
    binlog_index_file = os.path.join(binlog_bin_dir, "binlog.index")
    if not os.path.exists(binlog_index_file):
        log.info("create binlog.index file")
        touch_file(binlog_index_file)
    if not os.path.exists(binlog_index_file):
        log.error(f"create binlog index file: {binlog_index_file} failed")
        return False
    return True


def need_create_binlog_index(mysql_version):
    if MySQLVersion.MARIADB in mysql_version:
        log.info("mariadb need to create index file")
        return True
    version = mysql_version.split('-')[-1]
    if version[:1] == '8':
        log.warn("8.x version no need to create index file")
        return False
    return True


