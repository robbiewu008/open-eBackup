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
import os.path
import sys
import time
import logging
import gzip
import stat
import wexpect


log_file_max_size = 50
log_file_max_count = 6
logfile_suffix = '.gz'


class Result:
    SUCCESS = 0
    FAILED = 1


class ConstStr:
    UMOUNT_PATH = "umountPath"


class InputParam:

    def __init__(self, message):
        self.umount_path = ""
        self._convert_param(message)

    def check_param(self):
        if not self.umount_path:
            logging.error("Get umountpath failed.")
            return Result.FAILED

        return Result.SUCCESS

    def _convert_param(self, message):
        parameter = message.strip().split('\n')
        for item in parameter:
            param = item.split("=", 1)
            if len(param) < 2:
                continue
            if param[0] == ConstStr.UMOUNT_PATH:
                self.umount_path = str(param[1])


def read_input_parameter(file_path):
    out = ""
    with open(file_path, 'r') as file:
        for line in file:
            if line.strip():
                out += line
    if not out:
        logging.error("Read input param err, input param is null")
        return Result.FAILED, out
    return Result.SUCCESS, out


def get_filesize(file_path):
    if not os.path.exists(file_path):
        return 0
    file_size = os.path.getsize(file_path)
    file_size = file_size/float(1024 * 1024)
    return int(file_size)


def gzip_file(file, gzip_target):
    with open(file, 'rb') as plain_file:
        with gzip.open(gzip_target, 'wb') as zip_file:
            zip_file.writelines(plain_file)
    return


def dump_log(file_path):
    number = log_file_max_count - 1
    while number >= 0:
        backup_name = file_path + '.' + str(number) + logfile_suffix
        if number == 0:
            gzip_file(file_path, backup_name)
            os.remove(file_path)

        if os.path.exists(backup_name):
            dest_num = number + 1
            dest_file_name = file_path + '.' + str(dest_num) + logfile_suffix
            if os.path.exists(dest_file_name):
                os.remove(dest_file_name)
            os.rename(backup_name, dest_file_name)
            os.chmod(dest_file_name, stat.S_IRUSR + stat.S_IRGRP)
        number = number - 1


def save_log_file(file_path):
    if get_filesize(file_path) >= log_file_max_size:
        logging.info("Dump log file!")
        dump_log(file_path)


def init_log(root_path):
    log_path = root_path + '/log/UmountDataturbo.log'
    if not os.path.exists(log_path):
        mode = 'w'
    else:
        mode = 'a'
    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s %(filename)s[line:%(lineno)d] %(levelname)s %(message)s',
        filename=log_path,
        filemode=mode
    )
    save_log_file(log_path)


def get_input_param_item(message):
    input_param = InputParam(message)
    if input_param.check_param() != Result.SUCCESS:
        logging.error("Check input param failed.")
        return input_param, Result.FAILED
    return input_param, Result.SUCCESS


def umount_dataturbo_path(umount_path):
    command = "dataturbo umount mount_dir=" + umount_path
    logging.info("umount dataturbo path cmd: %s." % command)
    real_executable = sys.executable
    try:
        if sys._MEIPASS is not None:
            sys.executable = os.path.join(sys._MEIPASS, "wexpect", "wexpect.exe")
    except AttributeError:
        pass
    child = wexpect.spawn(command)
    sys.executable = real_executable

    try:
        index = child.expect(['\(y/n\):', wexpect.EOF, wexpect.TIMEOUT])
        if index != 0:
            logging.error("Umount dataturbo path failed, time out or not exist.")
            return Result.FAILED

        child.sendline('y')
        index = child.expect(['\(y/n\):', wexpect.EOF, wexpect.TIMEOUT])
        if index != 0:
            logging.error("Umount dataturbo path failed, time out or not exist!")
            return Result.FAILED

        child.sendline('y')
        result = child.expect([wexpect.TIMEOUT, "Command executed successfully."])
        if not result:
            logging.error("Umount dataturbo path failed!")
            return Result.FAILED
    except Exception as err:
        logging.error("Umount dataturbo path failed! Exception is %s." % err)
        return Result.FAILED
    return Result.SUCCESS


def exec_main(args):
    if len(args) < 2:
        print("Input parameter error.")
        return Result.FAILED
    agent_root_path = args[1]
    pid = args[2]
    init_log(agent_root_path)

    param_file = agent_root_path + f"\\tmp\\input_tmp{pid}"

    logging.info("Agent root path is " + agent_root_path)
    ret_code, message = read_input_parameter(param_file)
    if ret_code != Result.SUCCESS:
        logging.error("Read input paramter failed.")
        return ret_code

    input_param, ret_code = get_input_param_item(message)
    if ret_code != Result.SUCCESS:
        logging.error("Read input paramter item failed.")
        return ret_code

    retry_times = 1
    while retry_times < 4:
        umount_status = umount_dataturbo_path(input_param.umount_path)
        if umount_status == Result.SUCCESS:
            logging.info(f"Umount path {input_param.umount_path} success.")
            return Result.SUCCESS
        logging.error(f"Umount path {input_param.umount_path} failed, retry {retry_times} times.")
        time.sleep(5)
        retry_times = retry_times + 1
    logging.error(f"Mountpoint: {input_param.umount_path} still exist, umount failed.")
    return Result.FAILED


if __name__ == "__main__":
    ret = exec_main(sys.argv)
    sys.exit(ret)
