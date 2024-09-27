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

import grp
import json
import os
import pwd
import stat
import uuid
from logging import Logger

from common.common import execute_cmd
from common.common_models import SubJobDetails
from common.const import RpcToolInterface, ParamConstant, CMDResult


def get_uid_by_os_user(os_user: str) -> int:
    """根据用户名获取用户ID"""
    return pwd.getpwnam(os_user).pw_uid


def get_gid_by_os_user(os_user: str) -> int:
    """根据用户名获取用户组ID"""
    return pwd.getpwnam(os_user).pw_gid


def get_group_name_by_os_user(os_user: str) -> str:
    """根据用户名获取用户组名"""
    group_id = pwd.getpwnam(os_user).pw_gid
    return grp.getgrgid(group_id).gr_name


def get_path_uid_and_gid(input_path: str) -> (int, int):
    """获取输入路径所属用户ID和用户组ID"""
    stat_info = os.stat(input_path)
    return stat_info.st_uid, stat_info.st_gid


def exec_rpc_tool_cmd(api_name: str, in_path: str, out_path: str, logger_inst: Logger) -> bool:
    """
    执行rpctool命令
    :param api_name: API名称
    :param in_path: 输入路径
    :param out_path: 输出路径
    :param logger_inst: 日志实例
    :return: True：成功；False：失败
    """
    rpc_tool_path = os.path.realpath(os.path.join(ParamConstant.BIN_PATH, 'rpctool.sh'))
    rpc_tool_cmd = f"{rpc_tool_path} {api_name} {in_path} {out_path}"
    logger_inst.info("Start executing rpc tool command: %s.", rpc_tool_cmd)
    return_code, out, err = execute_cmd(rpc_tool_cmd)
    if return_code != CMDResult.SUCCESS.value:
        logger_inst.error(f"Execute rpc tool command failed, return code: %s, out: %s, err: %s.", return_code, out, err)
        return False
    logger_inst.info(f"Execute rpc tool command success.")
    return True


def exec_rpc_tool_cmd_with_output(api_name, in_path, out_path, logger_inst: Logger):
    """
    执行rpctool命令返回输出
    :param api_name: API名称
    :param in_path: 输入路径
    :param out_path: 输出路径
    :param logger_inst: 日志实例
    :return: True：成功；False：失败
    """
    rpc_tool_path = os.path.realpath(os.path.join(ParamConstant.BIN_PATH, 'rpctool.sh'))
    rpc_tool_cmd = f"{rpc_tool_path} {api_name} {in_path} {out_path}"
    logger_inst.info(f"Start executing rpc tool command: {rpc_tool_cmd} with output ...")
    return_code, out, err = execute_cmd(rpc_tool_cmd)
    if return_code != CMDResult.SUCCESS.value:
        logger_inst.error(f"Execute rpc tool command with output failed, return code: {return_code}, "
                          f"out: {out}, err: {err}.")
        return False, dict()
    with open(out_path, "r", encoding='utf-8') as tmp_file:
        output_dict = json.loads(tmp_file.read())
    logger_inst.info(f"Execute rpc tool command with output success.")
    return True, output_dict


def proactively_report_progress(pid: str, sub_job_detail: SubJobDetails, logger_inst: Logger) -> None:
    """
    主动上报进度
    :param pid: 任务PID
    :param sub_job_detail: 子任务详情
    :param logger_inst: 日志实例
    :return: None
    """
    file_name_pre = f"{pid}_{str(uuid.uuid4())}"
    input_file = os.path.join(ParamConstant.RESULT_PATH, file_name_pre + '_input')
    output_file = os.path.join(ParamConstant.RESULT_PATH, file_name_pre + '_output')
    flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
    modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
    with os.fdopen(os.open(input_file, flags, modes), 'w') as file_io:
        file_io.write(json.dumps(sub_job_detail.dict(by_alias=True)))
    try:
        exec_rpc_tool_cmd(RpcToolInterface.REPORT_JOB_DETAIL, input_file, output_file, logger_inst)
    except Exception as ex:
        with open(output_file, 'r', encoding='utf-8') as file_read:
            logger_inst.error("Report job detail fail, pid: %s, result: %s, error: %s.", pid, file_read.readlines(), ex)
    finally:
        for path in (input_file, output_file):
            if os.path.isfile(path):
                os.remove(path)


def change_dir_owner_recursively(input_path: str, uid: int, gid: int):
    """
    递归修改目录所属用户和用户组
    :param input_path: 待修改目录
    :param uid: 用户ID
    :param gid: 用户组ID
    """
    if not os.path.isdir(input_path) or os.path.islink(input_path):
        return
    if not str(uid).isdigit() or not str(gid).isdigit():
        return
    uid, gid = int(uid), int(gid)
    os.lchown(input_path, uid, gid)
    for root, dirs, files in os.walk(input_path):
        for tmp_dir in dirs:
            tmp_dir_path = os.path.join(root, tmp_dir)
            os.lchown(tmp_dir_path, uid, gid)
        for tmp_file in files:
            tmp_file = os.path.join(root, tmp_file)
            os.lchown(tmp_file, uid, gid)