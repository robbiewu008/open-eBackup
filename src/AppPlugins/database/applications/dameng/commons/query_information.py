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

import xmltodict

from common.common import execute_cmd, execute_cmd_list, check_command_injection
from common.const import JobData, SubJobStatusEnum
from common.logger import Logger
from dameng.commons.common import del_space, del_space_in_list, del_file, cmd_grep
from dameng.commons.const import DamengStrConstant, DM_FILE_PATH, RestoreProgress, ErrCode
from dameng.commons.const import DamengStrFormat, ArrayIndex, ExecCmdResult
from dameng.commons.dameng_tool import DmSqlTool
from dameng.commons.dm_rman_tool import DmRmanTool
from dameng.resource.damengsource import DamengSource

LOGGER = Logger().get_logger("dameng.log")


def query_process_info(pid_rules='', ppid_rules=(), isppid=False):
    """
    查询进程信息（disql）
    :return: 进程ID列表
    """
    result_msg = []
    pid_result = []
    if pid_rules == ():
        return False, [], ''
    cmd_list = ["ps -ef", f"grep {pid_rules}", "grep -v grep"]
    return_code, pid_info, err_info = execute_cmd_list(cmd_list)
    if return_code == "0":
        pid_info = pid_info.strip("\n").split("\n")
        for pid in pid_info:
            info_list = del_space(pid)
            pid_result.append([info_list[1], info_list[2]])
    else:
        return False, pid_result, err_info
    if not isppid:
        return True, pid_result, err_info
    if not ppid_rules:
        return True, pid_result, err_info
    for pid_info in pid_result:
        cmd_list = ["ps -ax", f'grep {pid_info[1]} pts']
        return_code, ppid_info, err_info = execute_cmd_list(cmd_list)
        flag = 0
        for ppid_rule in ppid_rules:
            if ppid_rule not in ppid_info:
                flag = 1
                break
        if flag:
            continue
        result_msg.append(pid_info)
    return True, result_msg, ''


def get_progress(num, progress):
    try:
        progress = num + int(progress) / DamengStrConstant.DIVIDED_BY_2
    except (ZeroDivisionError, ValueError) as exception_str:
        LOGGER.error("Get progress fail.")
        return False, int(progress), SubJobStatusEnum.FAILED.value
    if progress == DamengStrConstant.ONE_HUNDRED:
        return True, RestoreProgress.completed_progress, SubJobStatusEnum.RUNNING.value
    else:
        return True, int(progress), SubJobStatusEnum.RUNNING.value


def query_db_status(db_info):
    """
    查询数据库状态
    :return: '状态','模式'
    """
    mode = ''
    status = ''
    single_or_cluser = db_info.get("single_or_cluser", '')
    if single_or_cluser == "single":
        tool = DmSqlTool(db_info)
        disql_cmd = f"select STATUS$,MODE$ from v$instance;"
        sql_status, result_info = tool.run_disql_tool([disql_cmd])
        if not sql_status:
            return status, mode
        result = result_info[0].strip('\n').split('\n')[-3].split(' ')
        result = del_space_in_list(result)
        status, mode = result[1], result[2]
        return status, mode
    elif single_or_cluser == "cluser":
        tool = DmSqlTool(db_info)
        cmd = []
        disql_cmd = "select STATUS$,MODE$ from v$instance;"
        cmd.append(disql_cmd)
        sql_status, result_info = tool.run_disql_tool(cmd, mpp_type='local')
        if not sql_status:
            LOGGER.error('Get status and mode fail.')
            return status, mode
        result = result_info[0].strip('\n').split('\n')[-3].split(' ')
        result = del_space_in_list(result)
        status, mode = result[1], result[2]
        return status, mode
    else:
        LOGGER.error("Unknown database type.")
        return status, mode


def query_db_arch_status(db_info):
    """
    查询数据库状态
    :return: '状态'
    """
    single_or_cluser = db_info.get("single_or_cluser", '')
    if single_or_cluser == "single":
        tool = DmSqlTool(db_info)
        disql_cmd = f"select arch_mode from v$database;"
        sql_status, result_info = tool.run_disql_tool([disql_cmd])
        if not sql_status:
            return ''
        result = result_info[0].strip('\n').split('\n')[-3].split(' ')
        result = del_space_in_list(result)
        return result[-1]
    elif single_or_cluser == "cluser":
        tool = DmSqlTool(db_info)
        disql_cmd = "select arch_mode from v$database;"
        cmd = [disql_cmd]
        sql_status, result_info = tool.run_disql_tool(cmd, mpp_type='local')
        if not sql_status:
            return ''
        result = result_info[0].strip('\n').split('\n')[-3].split(' ')
        result = del_space_in_list(result)
        return result[-1]
    else:
        LOGGER.error("Unknown database type.")
        return ''


def show_backupset(backupset_path):
    """
    解析备份集信息
    :param backupset_path:备份集路径
    :return:
    """
    backupser_xml_path = f"{DM_FILE_PATH}/backupset_info_{JobData.PID}.xml"
    cmd = DamengStrFormat.SHOW_BACKUPSET.format(backupset_path, backupser_xml_path)
    result_info = DmRmanTool().run_rman_tool((cmd, "exit;"), DM_FILE_PATH)
    if not result_info.get("result"):
        LOGGER.error(f"Failed to run the dmrman command.")
        return {}

    with open(backupser_xml_path, "r") as f_object:
        backupset_info = xmltodict.parse(f_object.read())
    del_file(backupser_xml_path)
    return backupset_info


def get_db_name(db_info):
    """
    查询数据库状态
    :return: '状态','模式'
    """

    single_or_cluser = db_info.get("single_or_cluser", '')
    if single_or_cluser == "single":
        tool = DmSqlTool(db_info)
        disql_cmd = f"select NAME from V$database;"
        sql_status, result_info = tool.run_disql_tool([disql_cmd])
        if not sql_status:
            LOGGER.error("Get database name fail.")
            return False, ''
        result = result_info[0].strip('\n').split('\n')[-3].split(' ')
        result = del_space_in_list(result)
        try:
            db_name = result[-1]
        except Exception:
            return False, ''
        return True, db_name
    elif single_or_cluser == "cluser":
        tool = DmSqlTool(db_info)
        cmd = []
        disql_cmd = "select NAME from V$database;"
        cmd.append(disql_cmd)
        sql_status, result_info = tool.run_disql_tool(cmd, mpp_type='local')
        if not sql_status:
            LOGGER.error("Get database name fail.")
            return False, ''
        result = result_info[0].strip('\n').split('\n')[-3].split(' ')
        result = del_space_in_list(result)
        try:
            db_name = result[-1]
        except Exception:
            return False, ''
        return True, db_name
    else:
        LOGGER.error("Unknown database type.")
        return False, ''


def get_oguid(db_info_):
    cmd = "select OGUID from v$instance;"
    cmd_listb = [cmd]
    sql_status, oguid_info = DmSqlTool(db_info_).run_disql_tool(cmd_listb, mpp_type='local')
    if not sql_status:
        LOGGER.error(f"Get oguid fail")
        return ''
    oguid_info = oguid_info[ArrayIndex.INDEX_FIRST_0]
    oguid_info = oguid_info.strip('\n').split('\n')[ArrayIndex.INDEX_LAST_3]
    oguid = oguid_info.strip().split(" ")[ArrayIndex.INDEX_LAST_1]
    return oguid


def get_primary_node_name(db_info):
    dm_tool = DmSqlTool(db_info)
    cmd = ("SELECT COUNT(INSTANCE_NAME) FROM V$INSTANCE;", "SELECT INSTANCE_NAME FROM V$INSTANCE;")
    status, result = dm_tool.run_disql_tool(cmd)
    primary_node_name = []
    if not status or len(result) != len(cmd):
        return primary_node_name
    count_info = result[0].strip('\n').split('\n')
    # 3：输出结果至少为3行，sql结果占一行，一个空行，最后一个默认输出内容占一行
    if len(count_info) < 3:
        return primary_node_name
    count_info = count_info[-3].split(' ')
    count = del_space_in_list(count_info)[-1]
    db_magic_info = result[1].strip('\n').split('\n')
    # 2：输出结果最后两行默认为一个空行和默认输出内容占一行
    if len(db_magic_info) < 2 + int(count):
        return primary_node_name
    for line in db_magic_info[-(2 + int(count)):-2]:
        db_magic = del_space(line)[-1]
        primary_node_name.append(db_magic)
    return primary_node_name


def query_auth_permisson(db_info):
    """
    查看当前登录数据库用户是否有DBA权限
    :param db_info: 数据库登录信息
    :return: 查询结果
    """
    cmd = "select * from session_privs where PRIVILEGE='DBA';"
    disql_tool = DmSqlTool(db_info)
    ret, out = disql_tool.run_disql_tool((cmd,))
    succ_code = 0
    if not ret:
        LOGGER.error(f"Query auth permisson fail")
        return ErrCode.AUTH_INFO_ERR
    if "no rows" in out[0] or "未选定行" in out[0]:
        LOGGER.error(f"The auth permission is not DBA.")
        return ErrCode.ERR_DB_AUTH_INSUFFICIENT_PERMISSION
    return succ_code


def get_version(_db_info):
    """
    获取达梦数据库版本
    :return: 版本信息
    """
    LOGGER.info("Get dameng version.")
    version = ''
    # 获取数据库安装信息
    result_type, user_info, _ = DamengSource.discover_application()
    if result_type:
        username = user_info
    else:
        LOGGER.error("Get dameng application fail.")
        return version
    LOGGER.info("Get bin path.")

    # 获取数据库安装路径
    bin_path = DamengSource.get_bin_path(username)
    path = os.path.join(bin_path, "disql")
    if check_command_injection(path):
        LOGGER.error("The disql path contains invalid characters.")
        return version
    # 查看数据库版本
    cmd = f"su - {username} -c '{path} -h'"
    result = cmd_grep("disql", cmd)
    if not result:
        return version
    # 结果展示 “disql V8”  or “disql V7.6.1.112-Build(2021.10.28-149686-10036)ENT”
    version_info = result[0]
    version = version_info.strip(" ").split(" ")
    if len(version) < 2:
        LOGGER.error("Failed to parse the result.")
        return ''
    version = version[1]
    if version == 'V8':
        return get_v8_version(username, bin_path, _db_info)
    else:
        version = version.split('-', 1)[0]
    LOGGER.info(f"Get version succ.version: {version}")
    return version


def get_v8_version(username, bin_path, _db_info):
    """
    获取达梦8的小版本信息
    :param _db_info:
    :param username:
    :param bin_path:
    :return:
    """
    version = get_v8_version_new(_db_info)
    if version != "":
        return version
    cmd = f"su - {username} -c '{bin_path}/disql -id'"
    ret, version_info, err = execute_cmd(cmd)
    # 结果展示："DM Database 64 V8 03134283890-20220720-165295-10045"
    if ret == ExecCmdResult.SUCCESS:
        version_info = version_info.strip('\\\n').strip(' ')
        version_info = version_info.split(' ')
        if len(version_info) >= 5:
            version = version_info[4].split('-', 1)[0]
            version = f"V8.{version}"
            return version
    tool_path = bin_path.replace('bin', "tool")
    cmd = f'su - {username} -c "sh {tool_path}/version.sh"'
    ret, version, err = execute_cmd(cmd)
    if ret == ExecCmdResult.SUCCESS:
        version = version.strip("\\\n")
        return version
    LOGGER.error(f"Get version fail")
    return ''


def get_v8_version_new(_db_info):
    sql_cmd = "select id_code;"
    tool = DmSqlTool(_db_info)
    status, version_info = tool.run_disql_tool((sql_cmd,))
    if not status:
        return ''
    version_info = version_info[ArrayIndex.INDEX_FIRST_0]
    version_info = version_info.strip("\n").split("\n")
    version_info = version_info[ArrayIndex.INDEX_LAST_3]
    version_info = del_space(version_info)
    version_info = version_info[1:]
    version = " ".join(version_info)
    version = f"V8.{version}"
    return version
