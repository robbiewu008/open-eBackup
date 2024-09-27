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

import datetime
import fcntl
import json
import os
import re
import signal
import sqlite3
import stat
import subprocess
import time
from dataclasses import asdict

import pexpect
import pymysql
import requests

from common.cleaner import clear
from common.common_models import LogDetail, SubJobDetails
from common.common import execute_cmd_with_expect, execute_cmd, execute_cmd_list
from common.const import SubJobStatusEnum, CMDResultInt, CMDResult, DBLogLevel
from common.env_common import get_install_head_path
from common.util.cmd_utils import cmd_format
from tdsql.common.const import TdsqlBackTypeConstant, BackupParam, ErrorCode, EnvNameValue, HostParam, \
    MySQLVersion, ConnectParam, JobInfo
from tdsql.common.tdsql_common import report_job_details, get_std_in_variable
from tdsql.handle.common.const import TDSQLReportLabel
from tdsql.logger import log
from mysql.src.common.constant import MysqlExecSqlError


def exec_backupcmd(backup_cmd, backup_step):
    log.info(f"Begin to execute {backup_step} at time {time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())}")
    return_code, std_out, std_err = execute_cmd_with_expect(backup_cmd, "", None)
    ret = (return_code == 0)
    output = std_out if ret else std_err
    if not ret:
        log.error(f"Exec {backup_step} cmd failed at time {time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())}, the "
                  f"output detail is {output}.")
        return False
    log.info(f"End to execute {backup_step} success at time {time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())}, "
             f"the output detail is {output}.")
    return True


def backup_log(job_info: JobInfo, cnf_path, target_dir, start_time):
    # 记录日志备份的当前时间和UTC时间
    copy_time = datetime.datetime.now(datetime.timezone.utc).astimezone()
    copy_time_str = copy_time.astimezone(datetime.timezone.utc)
    log.info(f"backup_log : end_time {copy_time}, end_time_str {copy_time_str}")
    # 计算时间差，用于找到时间区间内的binlog文件
    min_between = round((copy_time_str - start_time).seconds / 60) + 10

    # 获取 MySQL 配置文件中 log-bin 参数指定的 binlog 目录
    cmd_str = [f"cat {cnf_path}", "grep log-bin", "grep -v '^#'", "awk -F '=' '{print $2}'", "xargs dirname"]
    ret, binlogpath, err = execute_cmd_list(cmd_str)
    # 此时获取的binlogpath是binlog目录的路径，该路径下存放所有的binlog文件
    if ret != CMDResult.SUCCESS.value:
        log.error(f"Exec to get binlog path from database, the output detail is {binlogpath}.Err is {err}")
        return False, copy_time

    # 使用扩展正则获取时间区间内的待拷贝文件完整路径，binlog开头，后缀是至少6个数字
    cmd_str = [
        f"find {binlogpath.strip()}/ -regextype posix-extended -regex '.*/binlog.[0-9]{{6,}}' \
        -type f -mmin -{str(min_between)}",
        f"xargs -i cp -r {{}} {target_dir}"
    ]
    ret, output, err = execute_cmd_list(cmd_str)
    if ret != CMDResult.SUCCESS.value:
        log.error(f"Exec to cp binlog fail, the output detail is {output}.Err is {err}")
        return False, copy_time

    exec_cmd = cmd_format("find {}/ -type f", target_dir.strip())
    ret, cp_log_paths, err = execute_cmd(exec_cmd)
    expression = ".*binlog.[0-9]{6,}.*"
    if not re.search(expression, cp_log_paths):
        # 若查找的日志文件为空，日志备份失败，上报label提示
        log.error(
            f"Current directory not exist archive log. pid:{job_info.pid} \
            job_id:{job_info.job_id} sub_job_id:{job_info.sub_job_id}")
        report_label_prompt(job_info, TDSQLReportLabel.TDSQL_GROUP_LOG_BACKUP_FAIL_LABEL)
        return False, copy_time

    # 拷贝binlog到目标路径
    cmd_str = [f"find {binlogpath.strip()} -name '*gtidlist' -type f", f"xargs -i cp -r {{}} {target_dir}"]
    ret, output, err = execute_cmd_list(cmd_str)
    if ret != CMDResult.SUCCESS.value:
        log.error(f"Exec to copy binlog gtidlist to target path, the output detail is {output}.Err is {err}")
        return False, copy_time

    cmd_str = [f"find {binlogpath.strip()} -name '*index' -type f", f"xargs -i cp -r {{}} {target_dir}"]
    ret, output, err = execute_cmd_list(cmd_str)
    if ret != CMDResult.SUCCESS.value:
        log.error(f"Exec to copy binlog index to target path, the output detail is {output}.Err is {err}")
        return False, copy_time

    # 记录start_time到xtrabackup_info文件
    exec_cmd = cmd_format("echo start_time = `date +'%Y-%m-%d %H:%M:%S'` > {}",
                          os.path.join(target_dir, "xtrabackup_info"))
    ret, xtrabackup_info, err = execute_cmd(exec_cmd)
    if ret != CMDResult.SUCCESS.value:
        log.error(
            f"Exec to record startime to file xtrabackup_info, the output detail is {xtrabackup_info}.Err is {err}")
        return False, copy_time
    return True, copy_time


def backup_tdsql(backup_param: BackupParam, backup_pwd, req_id, job_id, sub_id):
    file_name = backup_param.file_name
    ip = backup_param.host
    port = backup_param.port
    backup_type = backup_param.backup_type
    backup_tools = backup_param.backup_tools
    target_dir = backup_param.target_dir
    base_dir = backup_param.base_dir
    node_info = ip + ":" + port

    # 清空target_dir文件夹、保存mysqld版本进而last_node文件
    cmd = f"rm -rf {target_dir}/* ; \
           `dirname {backup_tools}`/../bin/mysqld --version >> {target_dir}/mysql_version; \
           echo {ip}/{port}/`date +'%Y-%m-%d %H:%M:%S'` > {target_dir}/last_node"
    log.info(f"begin to execute command {cmd}")
    ret, output = subprocess.getstatusoutput(cmd)
    if ret:
        log.error(f"Failed to exec {cmd}, the return value is {ret} and the output detail is {output} .")
        set_failed_and_choose_node(file_name, node_info, req_id, job_id, sub_id)
        return False

    # 数据备份
    cmd = generate_backup_cmd(backup_type, backup_param, base_dir)
    if backup_param.mysql_version.startswith(MySQLVersion.MARIADB):
        # mariaDB 需要在命令中拼接密码
        cmd = cmd + cmd_format(" --password={}", backup_pwd)
        log.info(f"begin to execute data backup, mariaDB cmd {cmd}")
        return_code, std_out, out_str = execute_cmd_with_expect(cmd, "", None)
    else:
        log.info(f"begin to execute data backup, mysql cmd {cmd}")
        return_code, out_str = exec_cmd_spawn(cmd, backup_pwd)
    if return_code != CMDResultInt.SUCCESS:
        set_failed_and_choose_node(file_name, node_info, req_id, job_id, sub_id)
        log.error(f"Execute command failed: {out_str}.")
        return False

    # 执行prepare
    if not prepare_backup(backup_type, base_dir, target_dir, backup_tools):
        set_failed_and_choose_node(file_name, node_info, req_id, job_id, sub_id)
        log.error(f"Exec to prepare mysql database with apply log only option.")
        return False
    after_backup(file_name, node_info)
    return True


def exec_cmd_spawn(cmd, passwd):
    try:
        child = pexpect.spawn(cmd, timeout=None)
    except Exception as exception_str:
        log.error(f"Spawn except an exception {exception_str}.")
        return 1, "cmd error"
    try:
        ret_code = child.expect(["Enter password:", "Failed to connect to MySQL server", \
                                 pexpect.TIMEOUT, pexpect.EOF])
    except Exception as exception_str:
        # 此处不打印异常，异常会显示用户名
        log.error(f"Spawn except an exception {exception_str}.")
        child.close()
        return 1, "cmd error"
    if ret_code != 0:
        log.error(f"Exec cmd failed.")
        child.close()
        return 1, "cmd error"
    if not passwd:
        child.close()
        return 1, "get passwd failed"
    child.sendline(passwd)
    try:
        out_str = child.read()
    except Exception as exception_str:
        log.error(f"Exec cmd except an exception.")
        child.close()
        return 1, "cmd error"
    child.close()
    return child.exitstatus, str(out_str)


def prepare_backup(backup_type, base_dir, target_dir, backup_tools):
    full_clause = ""
    inc_clause = ""
    if backup_type == TdsqlBackTypeConstant.INCREMENTAL \
            or backup_type == TdsqlBackTypeConstant.INCREMENTAL_FOREVER:
        inc_clause = f"--target-dir={base_dir} --incremental-dir={target_dir}"
    else:
        full_clause = f"--target-dir={target_dir}"
    cmd = f"{backup_tools} --prepare --apply-log-only {full_clause} {inc_clause}"

    log.info(f"begin to execute command {cmd}")
    return exec_backupcmd(cmd, "prepare with apply log only option")


def after_backup(file_name, node_info):
    timeout = 100
    log.info(f"set isCompleted=1 on node {node_info} and write it to shared file {file_name}")
    sql_lists = [f"update nodeinfo set is_completed=1 where node_host='{node_info}'"]
    while os.path.exists(file_name):
        ret, output = exec_sqlite_sql(file_name, timeout, sql_lists)
        if ret:
            break


def report_message(req_id, job_id, sub_id, node_list, level):
    if level == "info":
        message = f"{node_list[1]}"
        log_detail = LogDetail(logInfo="dme_schedule_exec_job_label", logInfoParam=[message],
                               logLevel=DBLogLevel.INFO)
    elif level == "warn":
        message = f"node switch to {node_list[1]}"
        log_detail = LogDetail(logInfo="dme_databases_backup_failed_label", logInfoParam=[f"{node_list[0]}"],
                               logLevel=DBLogLevel.WARN, logDetail=ErrorCode.EXEC_BACKUP_RECOVER_CMD_FAIL,
                               logDetailParam=["Backup", message])
    sub_dict = SubJobDetails(taskId=job_id, subTaskId=sub_id, progress=100, logDetail=[log_detail],
                             taskStatus=SubJobStatusEnum.RUNNING.value)
    report_job_details(req_id, sub_dict.dict(by_alias=True))


def get_failed_count(file_name):
    cnt = 0
    timeout = 100
    sql_lists = ["select count(*) from nodeinfo where is_completed= 2 or is_alive != 0"]
    ret, output = exec_sqlite_sql(file_name, timeout, sql_lists)
    if ret:
        cnt = list(output)[0][0]
    return cnt


def set_failed_node_list(file_name, curr_node, req_id, job_id, sub_id):
    timeout = 100
    sql = [f"update nodeinfo set is_exec_node=0, is_completed=2 where node_host='{curr_node}'"]
    exec_sqlite_sql(file_name, timeout, sql)
    next_node = choose_next_node(file_name)
    if next_node:
        sql = [f"update nodeinfo set is_exec_node=1 where node_host='{next_node}'"]
        exec_sqlite_sql(file_name, timeout, sql)
        nodes = [curr_node, next_node]
        report_message(req_id, job_id, sub_id, nodes, "warn")


def monitor(backup_thread, backup_param: BackupParam, req_id, job_id, sub_id):
    file_name = backup_param.file_name
    ip = backup_param.host
    port = backup_param.port
    set_id = backup_param.set_id
    url = backup_param.url
    timeout = 100

    while True:
        log.info("begin to execute monitor job")
        log.info(f"monitor job url:{url} ip:{ip} port:{port} set_id:{set_id}")
        node_list = get_nodelist(file_name, timeout)
        if len(node_list) == 1:
            break
        curr_node = ip + ":" + port
        cnt = get_failed_count(file_name)
        if len(node_list) - 1 == cnt:
            break
        host_param = HostParam(url=url, ip=ip, port=port)
        if not is_slave(host_param, set_id, "backup", req_id) and len(node_list) - 1 != cnt:
            log.info(f"{ip}:{port} is not slave")
            try:
                log.error(f"database {curr_node} is master, terminate backup job")
                log.error(f"backup thead is stilling running, try to stop it")
                backup_thread.terminate()
            except Exception as ex:
                log.error(f"ex {ex}")
            set_failed_node_list(file_name, curr_node, req_id, job_id, sub_id)
            break
        if is_job_finished(file_name):
            log.warn(f"the backup job has finished")
            break
        time.sleep(15)


def check_cnt_update_file(file_name):
    cnt = 0
    timeout = 100
    node_list = get_nodelist(file_name, 100)
    if not node_list:
        log.warn("node_list is empty")
        return
    for j in node_list:
        if j[2] == 0:
            cnt = cnt + 1
    if cnt == len(node_list):
        next_node = choose_next_node(file_name)
        if next_node:
            log.info(f"the current executing node is null, choosing {next_node} as the next executing node")
            sql = [f"update nodeinfo set is_exec_node=1 where node_host='{next_node}'"]
            exec_sqlite_sql(file_name, timeout, sql)


def can_exec_backup(backup_param: BackupParam, file_name, set_id, pid):
    # 获取获共享文件
    url = backup_param.url
    ip = backup_param.host
    port = backup_param.port
    timeout = 100
    check_cnt_update_file(file_name)
    node_list = get_nodelist(file_name, timeout)
    log.info(f"the current node_list is {node_list}")
    host_node = ip + ":" + port
    for j in node_list:
        # 确保pre生成的节点能够执行备份
        if j[0] == host_node and j[2] == 1 and j[3] == 0:
            sql = [f"update nodeinfo set ever_backup=1 where node_host='{host_node}'"]
            exec_sqlite_sql(file_name, timeout, sql)
            log.info(f"the current executing node is {ip}:{port}")
            return True

        # 判断节点是否已down，更新共享文件
        if j[2] == 1 and \
                not is_node_living(url, j[0].split(":")[0], j[0].split(":")[1], set_id, pid):
            sql = [f"update nodeinfo set is_exec_node=0, is_completed=2 where node_host='{j[0]}'"]
            exec_sqlite_sql(file_name, timeout, sql)
            next_node = choose_next_node(file_name)
            if next_node:
                sql = [f"update nodeinfo set is_exec_node=1 where node_host='{next_node}'"]
                exec_sqlite_sql(file_name, timeout, sql)
            return False
    return False


def choose_next_node(file_name):
    timeout = 100
    node_list = get_nodelist(file_name, timeout)
    if not node_list or len(node_list) == 0:
        return ""
    master = ""
    nodes_selected = []
    for value in node_list:
        if value[6] != 0:
            continue
        if value[5] == 1 and value[3] != 1:
            master = value[0]
            continue
        if value[3] == 1:
            continue
        nodes_selected.append([value[0], value[1]])
    if not nodes_selected:
        if not master:
            log.info("the next executing node is null")
            return ""
        else:
            log.info(f"the next executing node is {master}")
            return master
    nodes_selected = sorted(nodes_selected, key=lambda s: s[1])
    log.info(f"the next executing node is {nodes_selected[0][0]}")
    return nodes_selected[0][0]


def is_job_finished(file_name):
    log.info("begin to check job finished")
    timeout = 100
    node_list = get_nodelist(file_name, timeout)

    log.info("the nodelist is node_host, priority, is_exec_node, ever_backup, is_completed, is_master, is_alive")
    log.info(f"check job finished, {node_list}")
    cnt = 0
    for value in node_list:
        if value[4] == 1:
            log.info("job is finished, one node has finished backup job")
            return True
        if value[4] == 2 or value[6] != 0:
            cnt = cnt + 1
    if len(node_list) == cnt:
        log.info("job is finished, all nodes have failed")
        return True
    log.info("job is still running")
    return False


def write_shared_files(file_name, data):
    log.info(f"writing data {data} to file {file_name}")
    flags = os.O_WRONLY | os.O_CREAT
    modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
    with os.fdopen(os.open(file_name, flags, modes), 'w') as file_stream:
        fcntl.flock(file_stream.fileno(), fcntl.LOCK_EX)
        file_stream.truncate(0)
        file_stream.write(json.dumps(data))


def get_tdsql_status(url, set_id, task_type, pid):
    request_header = {'Content-Type': 'application/json'}
    if task_type == "backup":
        caller = get_std_in_variable(f"{EnvNameValue.IAM_USERNAME_BACKUP}_{pid}")
        password = get_std_in_variable(f"{EnvNameValue.IAM_PASSWORD_BACKUP}_{pid}")
    else:
        caller = get_std_in_variable(f"{EnvNameValue.IAM_USERNAME_RESTORE}_{pid}")
        password = get_std_in_variable(f"{EnvNameValue.IAM_PASSWORD_RESTORE}_{pid}")
    request_body = {
        "callee": "TDSQL",
        "caller": caller,
        "password": password,
        "eventId": 101,
        "interface": {"interfaceName": "TDSQL.GetInstance", "para": {"instance": [{"id": f"{set_id}"}]}},
        "version": "1.0"
    }
    ret, body, header = request_post(url, request_body, request_header)
    try:
        if not (ret and body.get("returnData").get("err_code") == 0):
            log.error(f"get tdsql status error happened")
            return {}
        else:
            return body.get("returnData").get("instance")
    except Exception as ex:
        log.error(f"Exception2 {ex}")
        return {}
    finally:
        clear(password)


def is_slave(host_param: HostParam, set_id, task_type, pid):
    url = host_param.url
    ip = host_param.ip
    port = host_param.port
    body = get_tdsql_status(url, set_id, task_type, pid)
    for i in body:
        for j in dict(i).get("db"):
            if j.get('port') == int(port) and j.get('ip') == ip and j.get('master') == 1:
                log.info(f"{ip}：{port} is master")
                return False
    log.info(f"{ip}：{port} is slave")
    return True


def is_node_living(url, ip, port, set_id, pid):
    body = get_tdsql_status(url, set_id, "backup", pid)
    for i in body:
        for j in dict(i).get("db"):
            if j.get('port') == int(port) and j.get('ip') == ip and j.get('alive') == 0:
                log.info(f"{ip}：{port} is alive")
                return True
    log.info(f"{ip}：{port} is dead")
    return False


def check_status(mysql_port):
    max_failed_count = 0
    while max_failed_count < 3:
        ret, output = subprocess.getstatusoutput(f"ps -ef | grep mysqld | grep -v grep | grep {mysql_port} | wc -l")
        if ret == 0 and int(output) > 0:
            break
        else:
            max_failed_count = max_failed_count + 1
            time.sleep(10)
    if max_failed_count > 2:
        log.error("mysqld is not running, can not start backup job")
        return False
    else:
        log.info("mysqld is running, can execute next task")
        return True


def get_progress():
    log.info('Query backup progress!')
    return True, "5"


def get_last_node(path):
    last_node_path = os.path.join(path, "last_node")
    with open(last_node_path, "r", encoding='UTF-8') as f_content:
        fcntl.flock(f_content.fileno(), fcntl.LOCK_SH)
        data = f_content.read()
    nodes = data.split("/")
    # ip/port/start_time
    return nodes[0], nodes[1], nodes[2].split("\n")[0]


def cleanup_files(path, set_id):
    backup_path = os.path.join(path, f"TDSQL_{set_id}/BACKUP")
    backup_dirs = []
    for file in os.listdir(backup_path):
        abs_path = os.path.join(backup_path, file)
        if os.path.isdir(abs_path):
            backup_dirs.append(file)
    backup_dirs.sort()
    backup_dir = backup_dirs[-1]
    os.remove(backup_dir)


def request_post(url, body, headers):
    result = requests.post(url, json=body, headers=headers, verify=False)
    try:
        ret_body = json.loads(result.content.decode())
    except Exception as err:
        log.err(f"Err at POST: {err}")
        return False, {}, {}
    ret_headers = dict(result.headers)
    return True, ret_body, ret_headers


def exec_mysql_sql(user, pwd, socket, sql_str):
    try:
        mysql_conn = pymysql.connect(user=user, password=pwd, unix_socket=socket)
        cursor = mysql_conn.cursor()
        cursor.execute(sql_str)
        results = cursor.fetchall()
        mysql_conn.close()
    except Exception:
        log.error(f"execute sql {sql_str} failed!")
        return False, {}
    return True, results


def kill_backup_job(job_id, file_name, backup_tools, backup_type):
    # 杀进程
    exec_cmd = f"ps -ef|grep '{job_id}' | grep '{backup_tools}' |grep -v grep"
    ret, output = subprocess.getstatusoutput(exec_cmd)
    if ret:
        for line in output.splitlines():
            pid = int(line.split()[1])
            os.kill(pid, signal.SIGKILL)
            log.info(f"os.kill {pid} jobId{job_id}")

    if backup_type != TdsqlBackTypeConstant.LOG:
        # 更新共享文件
        with open(file_name, "r", encoding='UTF-8') as f_content:
            fcntl.flock(f_content.fileno(), fcntl.LOCK_SH)
            node_list = json.loads(f_content.read())
        log.warn(f"update isCompleted to 2 and write to file {file_name} to abort job {job_id}")
        for j in node_list.values():
            j["isCompleted"] = 2
        write_shared_files(file_name, node_list)


def get_cpu_number():
    exec_cmd = "cat /proc/cpuinfo | grep processor | wc -l"
    ret, output = subprocess.getstatusoutput(exec_cmd)
    if not ret:
        return int(output)
    else:
        return 8


def set_failed_and_choose_node(file_name, node_info, req_id, job_id, sub_id):
    timeout = 100
    update_sql = [f"update nodeinfo set is_exec_node=0, is_completed = 2 where node_host='{node_info}'"]
    exec_sqlite_sql(file_name, timeout, update_sql)
    next_node = choose_next_node(file_name)
    if next_node:
        sql = [f"update nodeinfo set is_exec_node=1 where node_host='{next_node}'"]
        exec_sqlite_sql(file_name, timeout, sql)
        nodes = [node_info, next_node]
        report_message(req_id, job_id, sub_id, nodes, "warn")


def check_priv_is_ok(user, pwd, socket, mysql_version):
    log.info(f"check_priv_is_ok mysql_version:{mysql_version}")
    tdsql_config = get_tdsql_config()
    need_privs = tdsql_config.get('needPrivs').get(mysql_version)
    priv_ips = ["%", "localhost", "127.0.0.1"]
    for priv_ip in priv_ips:
        cnt = 0
        priv_cmd = f"show grants for '{user}'@'{priv_ip}';"
        ret, output = exec_mysql_sql(user, pwd, socket, priv_cmd)
        if not ret:
            continue
        for iter_priv in need_privs:
            if iter_priv in str(output):
                cnt = cnt + 1
        log.info(f"cnt:{cnt}")
        log.info(f"len(need_privs):{len(need_privs)}")
        if cnt == len(need_privs):
            return True
    return False


def exec_sqlite_sql(file_name, timeout, sql_lists):
    try:
        conn = sqlite3.connect(file_name, timeout=timeout)
        cursor = conn.cursor()
        cursor.execute("begin exclusive transaction")
        for sql_list in sql_lists:
            cursor = conn.execute(sql_list)
        conn.commit()
        results = cursor.fetchall()
        conn.close()
    except Exception as ex:
        log.error(f"exception {ex}, execute sql {sql_lists} failed!")
        conn.rollback()
        conn.close()
        return False, {}
    return True, results


def init_create_info(file_name, timeout):
    sql_list = "create table nodeinfo(node_host varchar(200) primary key, set_id varchar(200), \
                    agent_uuid varchar(200), priority int, is_exec_node int, last_modified_time varchar(100), \
                    ever_backup int, is_completed int, is_master int, is_alive int)"

    ret, output = exec_sqlite_sql(file_name, timeout, [sql_list])
    if ret:
        log.info("init table success")
        return True, output
    log.error(f"init table failed, the output is {output}")
    return False, []


def get_nodelist(file_name, timeout):
    sql_lists = "select node_host, priority, is_exec_node, ever_backup, is_completed, is_master, \
                 is_alive from nodeinfo"
    ret, output = exec_sqlite_sql(file_name, timeout, [sql_lists])
    if ret:
        return output
    log.info("nodelist is null")
    return {}


def init_node_data(file_name, nodes_info, timeout):
    ret, output = init_create_info(file_name, timeout)
    if not ret:
        log.error(f"create table nodeinfo error , ret is {ret}, output is {output}")
        return False
    sql_lists = []
    for _, value in nodes_info.items():
        node_host = value.get("nodeHost")
        set_id = value.get("setId")
        agent_uuid = value.get("agentUuid")
        priority = value.get("priority")
        is_exec = value.get("isExecNode")
        ever_backup = value.get("everBackup")
        is_completed = value.get("isCompleted")
        is_master = value.get("isMaster")
        is_alive = value.get("isAlive")
        sql = f"insert into nodeinfo(node_host, set_id, agent_uuid, priority, is_exec_node, ever_backup, " \
              f"is_completed, is_master, is_alive) values " \
              f"('{node_host}', '{set_id}', '{agent_uuid}', {priority}, " \
              f"{is_exec}, {ever_backup}, {is_completed}, {is_master}, {is_alive})"
        sql_lists.append(sql)
    ret, output = exec_sqlite_sql(file_name, timeout, sql_lists)
    if not ret:
        log.error(f"insert into nodeinfo error , ret is {ret}, output is {output}")
        return False
    return True


def get_tdsql_config():
    tdsql_conf = f"{get_install_head_path()}" \
                 f"/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin/applications/tdsql/tdsql.conf"
    try:
        with open(tdsql_conf, "r") as temp_f:
            tdsql_config = json.loads(temp_f.read())
    except Exception as err:
        log.error("Fail to read tdsql conf for %s", err)
        return ""
    return tdsql_config


def get_version_path(db_version):
    default_version_path = ""
    log.info(f"db_version:{db_version}")
    if '-' in db_version:
        mysql_version = db_version.split('-')[0]
        tdsql_config = get_tdsql_config()
        version_path = tdsql_config.get('versionPath').get(mysql_version, '')
    else:
        version_path = default_version_path
    log.info(f"version_path is {version_path}")
    return version_path


def get_xtrabackup_tool_path(mysql_version):
    tdsql_config = get_tdsql_config()
    return tdsql_config.get('backupTools').get(mysql_version, '')


def get_backup_param_value(backup_param_dict, backup_config_key, backup_config_value):
    backup_key = backup_config_key.replace('-', '_')
    if backup_key == 'compress_threads':
        backup_key = 'parallel'
    backup_param_value = ''
    if backup_param_dict.get(backup_key, ''):
        backup_param_value = backup_param_dict[backup_key]
    elif backup_config_value:
        backup_param_value = backup_config_value
    if backup_key == 'use_memory' and backup_param_value:
        backup_param_value = str(backup_param_value) + 'M'
    return backup_param_value


def generate_backup_cmd(backup_type, backup_param, base_dir):
    log.info(f"generate_backup_cmd, {backup_param.mysql_version}")
    incremental_clause = ""
    if backup_type == TdsqlBackTypeConstant.INCREMENTAL \
            or backup_type == TdsqlBackTypeConstant.INCREMENTAL_FOREVER:
        incremental_clause = f"--incremental --incremental-basedir={base_dir}"
    backup_param_dict = asdict(backup_param)
    tdsql_config = get_tdsql_config()
    common_params = tdsql_config.get("commonBackupParams")
    backup_cmd = ''
    for param_key in common_params:
        param_value = get_backup_param_value(backup_param_dict, param_key, common_params[param_key])
        backup_cmd += f"--{param_key}={param_value} "
    version_params = tdsql_config.get('versionBackupParams').get(backup_param.mysql_version)
    for param_key in version_params:
        param_value = get_backup_param_value(backup_param_dict, param_key, version_params[param_key])
        backup_cmd += f"--{param_key}={param_value} "
    no_value_params = tdsql_config.get("noValueBackupParams").get(backup_param.mysql_version)
    for param in no_value_params:
        backup_cmd += f"{param} "
    cmd = f"{backup_param.backup_tools} {backup_cmd} {incremental_clause}"
    return cmd


def exec_sql(user, passwd, connect_param: ConnectParam, sql_str):
    if not passwd:
        return False, "get passwd failed"
    try:
        log.info(f"into exec_sql mysql_connection, the user is {user}")
        if connect_param.socket:
            mysql_connection = pymysql.connect(user=user, password=passwd, unix_socket=connect_param.socket)
        else:
            mysql_connection = pymysql.connect(user=user, password=passwd, host=connect_param.ip,
                                               port=int(connect_param.port))
    except pymysql.Error as except_str:
        log.error(f"Connect MySQL service failed!")
        if MysqlExecSqlError.ERROR_ACCESS_DENINED in str(except_str):
            return False, MysqlExecSqlError.ERROR_ACCESS_DENINED
        return False, "exec_sql raise except"
    cursor = mysql_connection.cursor()
    cursor.execute(sql_str)
    results = cursor.fetchall()
    log.info(f"exec_sql success results {results}")
    mysql_connection.close()
    return True, results


def report_label_prompt(job_info: JobInfo, log_info):
    """
    用于上报label提示
    :param job_info: JobInfo类
    :param log_info:str类型的提示词
    :return:
    """
    log_detail = LogDetail(logInfo=log_info,
                           logInfoParam=[job_info.sub_job_id],
                           logLevel=DBLogLevel.ERROR.value)
    return report_job_details(job_info.pid,
                              SubJobDetails(taskId=job_info.job_id, subTaskId=job_info.sub_job_id, progress=100,
                                            logDetail=[log_detail],
                                            taskStatus=SubJobStatusEnum.RUNNING.value).dict(
                                  by_alias=True))
