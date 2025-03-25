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
#!/usr/bin/python
import os
import os.path
import sys
import pexpect
import shlex
import subprocess
import time
import logging
import gzip
import stat

log_file_max_size = 50
log_file_max_count = 6
logfile_suffix = '.gz'

class Result:
    SUCCESS = 0
    FAILED = 1


class LinkType:
    UN_KNOWN = 0
    IP = 1
    FC = 2


class DedupSwitch:
    ON = "ON"
    OFF = "OFF"


class ConstStr:
    STORAGE_NAME = "storageName"
    USER_NAME = "userName"
    PWD = "password"
    IP_LIST = "ipList"
    LINK_TYPE = "linkType"
    DEDUP_SWITCH = "dedupSwitch"
    FC = "FC"
    IPS = "Ips"
    ID = "ID"
    STATUS = "Status"
    NORMAL = "Normal"


class LinkStatus:

    def __init__(self):
        self.object_create_flag = False
        self.all_link_fault_flag = True
        self.link_ip_list = []
        self.link_type = LinkType.IP


class InputParam:

    def __init__(self, message):
        self.storage_name = ""
        self.user_name = ""
        self.password = ""
        self.ip_list = ""
        self.link_type = LinkType.IP
        self.dedup_switch = DedupSwitch.ON
        self._convert_param(message)

    def check_param(self):
        if not self.storage_name:
            logging.error("Get storageName failed.")
            return Result.FAILED
        if not self.user_name:
            logging.error("Get dataturbo username failed.")
            return Result.FAILED
        if not self.password:
            logging.error("Get dataturbo password failed.")
            return Result.FAILED
        if self.link_type == LinkType.IP and not self.ip_list:
            logging.error("Get ip list failed, link type is ip.")
            return Result.FAILED

        return Result.SUCCESS

    def _convert_param(self, message):
        parameter = message.strip().split('\n')
        for item in parameter:
            param = item.split("=", 1)
            if len(param) < 2:
                continue
            if param[0] == ConstStr.STORAGE_NAME:
                self.storage_name = str(param[1])
            if param[0] == ConstStr.USER_NAME:
                self.user_name = str(param[1])
            if param[0] == ConstStr.PWD:
                self.password = str(param[1])
            if param[0] == ConstStr.IP_LIST:
                self.ip_list = str(param[1])
            if param[0] == ConstStr.LINK_TYPE:
                self.link_type = LinkType.FC if str(param[1]).upper() == ConstStr.FC else LinkType.IP
            if param[0] == ConstStr.DEDUP_SWITCH:
                self.dedup_switch = DedupSwitch.OFF if str(param[1]).upper() == DedupSwitch.OFF else DedupSwitch.ON


def exec_shell_cmd(command):
    std_ret = 1
    std_out = ""
    std_error = ""
    try:
        child = subprocess.Popen(
            shlex.split(command), shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE, encoding='utf-8')
        std_out, std_error = child.communicate()
        std_ret = child.returncode
    except Exception as err:
        logging.error("Exec shell cmd failed, err: %s." % err)

    return std_ret, std_out.strip(), std_error.strip()


def filter_result(result_str, storage_name):
    storage_name_index = -1
    tmp_arr = [item.strip() for item in result_str.split("\n") if item.strip() != ""]
    for index, per_line in enumerate(tmp_arr):
        if "Storage Name" in per_line and storage_name in per_line:
            storage_name_index = index
            break
    if storage_name_index == -1:
        return []
    return tmp_arr[storage_name_index:]

def check_return_value(value):
    res = 1
    tmp_arr = [item.strip() for item in value.split("\n") if item.strip() != ""]
    for index, per_line in enumerate(tmp_arr):
        if "Cause" in per_line and "The number of commands being executed has reached the upper limit. New commands cannot be executed." in per_line:
            res = 0
    return res

def get_dataturbo_link_status(storage_name):
    def get_ip_list(str_info):
        tmp_list = str_info.split(":")
        if len(tmp_list) < 2:
            return []
        tmp_ip_list = tmp_list[1].strip().strip(',').split(",")
        return [item.strip() for item in tmp_ip_list if item != ""]

    link_status = LinkStatus()
    command = "dataturbo show storage_object storage_name=" + storage_name
    ret_code, out, err = exec_shell_cmd(command)
    check_times = 0

    while (check_return_value(out) == 0 and ret_code != Result.SUCCESS and check_times < 3 ):
        time.sleep(5)
        logging.error("Execute dataturbo show command failed, err: %s. Start to retry,retry time: %d" % (err, check_times))
        command = "dataturbo show storage_object storage_name=" + storage_name
        ret_code, out, err = exec_shell_cmd(command)
        check_times = check_times + 1
    if(ret_code != Result.SUCCESS):
        logging.error("Execute dataturbo show command failed, err: %s." % err)
        return link_status

    object_out = filter_result(out, storage_name)
    if not object_out:
        logging.error("The storage_name object not created.")
        return link_status

    link_status.object_create_flag = True
    link_status.link_type = LinkType.IP if ConstStr.IPS in object_out[2] else LinkType.FC
    if link_status.link_type == LinkType.IP:
        # IP链路才需要获取ipList
        link_status.link_ip_list = get_ip_list(object_out[2])
        if not link_status.link_ip_list:
            logging.error("The storage_name hasn't link information.")
            return link_status

    # 检查是否所有链接都失效了，只要有一个未失效就可以继续使用
    start_flag = False
    for per_line in object_out:
        if start_flag and ConstStr.NORMAL in per_line:
            link_status.all_link_fault_flag = False
            logging.info("The storage_name link [%s] normal." % per_line)
            break
        if ConstStr.ID in per_line and ConstStr.STATUS in per_line:
            start_flag = True

    return link_status


def create_dataturbo_object(input_param):
    command = "dataturbo create storage_object storage_name=" + input_param.storage_name
    if input_param.link_type == LinkType.IP:
        command += " ip_list=" + input_param.ip_list
    if input_param.link_type == LinkType.FC:
        command += ' link_type=FC'
    logging.info("Create storage object cmd: %s." % command)
    child = pexpect.spawn(command)

    try:
        result = child.expect([pexpect.TIMEOUT, 'input username:'])
        if not result:
            logging.error("Create object failed!,input username not found.")
            return Result.FAILED
        child.sendline(input_param.user_name)
        result = child.expect([pexpect.TIMEOUT, 'input password:'])
        if not result:
            logging.error("Create object failed!,input password not found.")
            return Result.FAILED
        child.sendline(input_param.password)
        result = child.expect([pexpect.TIMEOUT, "Create storage object successfully."])
        if not result:
            logging.error("Create object failed.")
            return Result.FAILED
    except Exception as err:
        logging.error("Create object failed! Exception is %s." % err)
        return Result.FAILED
    return Result.SUCCESS


def delete_dataturbo_object(storage_name, root_path):
    command = "dataturbo delete storage_object storage_name=" + storage_name
    logging.info("Delete storage object cmd: %s." % command)
    child = pexpect.spawn(command)

    try:
        index = child.expect(['\(y/n\):', pexpect.EOF, pexpect.TIMEOUT])
        if index != 0:
            logging.error("Delete dataturbo object failed, time out or not find.")
            return Result.FAILED

        child.sendline('y')
        index = child.expect(['\(y/n\):', pexpect.EOF, pexpect.TIMEOUT])
        if index != 0:
            logging.error("Delete dataturbo object failed, time out or not find!")
            return Result.FAILED

        child.sendline('y')
        index = child.expect([pexpect.TIMEOUT, "Delete storage object successfully."])
        if index == 0:
            std_ret = 1
            std_out = ""
            std_error = ""
            if os.name == 'posix':  
                xmlcfg_linux_path = root_path + "/bin/xmlcfg"
                command = xmlcfg_linux_path + " write Backup backup_esn ''";
                child = subprocess.Popen(["su", "-", "rdadmin", "-s", "/bin/sh", "-c", command],
                    shell = False, stdout = subprocess.PIPE, stderr = subprocess.PIPE, encoding = 'utf-8')
                std_out, std_error = child.communicate()
                std_ret = child.returncode
            elif os.name == 'nt':
                xmlcfg_win_path = root_path + "/bin/xmlcfg.exe"
                child = subprocess.Popen(["cmd", "/c", "call", xmlcfg_win_path, "write", "Backup", "backup_esn", ""],
                    stdout = subprocess.PIPE, stderr = subprocess.PIPE, encoding = 'utf-8')
                std_out, std_error = child.communicate()
                std_ret = child.returncode
            if std_ret != Result.SUCCESS:
                logging.error("CMD failed to Clear ESN.")
            logging.info("Delete dataturbo object success!")
            return Result.SUCCESS
        else:
            logging.error("Delete dataturbo object failed!")
            return Result.FAILED

    except Exception as err:
        logging.error("Delete dataturbo object failed! Exception is %s." % err)
        return Result.FAILED


def add_iplist_to_dataturbo_object(storage_name, ip_list):
    command = "dataturbo add storage_object_ip_list storage_name=" + storage_name + " ip_list=" + ip_list
    logging.info("Add cmd: %s." % command)
    try:
        child = pexpect.spawn(command)
        result = child.expect([pexpect.TIMEOUT, "Command executed successfully.", \
            "The number of commands being executed has reached the upper limit. New commands cannot be executed.", \
            "The IP addresses to be added are from different storage devices and cannot be added."])
    except Exception as err:
        logging.error("Add iplist to dataturbo object failed! Exception is %s." % err)
        return Result.FAILED
    check_times = 0
    while (result == 2 and check_times < 3 ):
        time.sleep(5)
        try:
            child = pexpect.spawn(command)
            result = child.expect([pexpect.TIMEOUT, "Command executed successfully.", \
                "The number of commands being executed has reached the upper limit. New commands cannot be executed.", \
                "The IP addresses to be added are from different storage devices and cannot be added."])
        except Exception as err:
            logging.error("Add iplist to dataturbo object failed! Exception is %s." % err)
            return Result.FAILED
        check_times = check_times + 1
    if result == 3:
        logging.error("The IP addresses to be added are from different storage devices and cannot be added.")
        return result
    if result != 1:
        logging.error("Add iplist to dataturbo object failed.")
        return Result.FAILED
    return Result.SUCCESS


def read_input_parameter(param_num):
    out = ""
    param_counter = 0
    while param_counter < int(param_num):
        try:
            param = sys.stdin.readline()
        except Exception as err:
            logging.error("Read input param exception! Exception is %s." % err)
            return Result.FAILED, out
        out = out + param
        param_counter = param_counter + 1
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
    log_path = root_path + '/slog/CheckAndCreateDataturbo.log'
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


def update_link_ip(link_ip_list, ip_list, storage_name):
    # 获取链路中已有的IP
    ip_dict = ip_list.strip(',').split(",")
    new_ip_list = ""
    # 获取输入参数IP List中不在链路IP中的IP
    for ip in ip_dict:
        if ip not in link_ip_list:
            new_ip_list = new_ip_list + ip + ','

    if not new_ip_list:
        logging.info("The input ip list not contain new ip in the link IP list")
        return Result.SUCCESS

    new_ip_list = new_ip_list.strip(',')
    logging.info("New IP list is %s." % new_ip_list)
    return add_iplist_to_dataturbo_object(storage_name, new_ip_list)


def set_dedup_switch(input_param):
    command = "dataturbo change deduplication_compression" + \
              " deduplication=" + input_param.dedup_switch + " compression=" + input_param.dedup_switch
    ret_code, out, std_err = exec_shell_cmd(command)
    check_times = 0
    while (check_return_value(out) == 0 and ret_code != Result.SUCCESS and check_times < 3 ):
        time.sleep(5)
        logging.error("Execute dataturbo change command failed, err: %s. Start to retry,retry time: %d" % (err, check_times))
        command = "dataturbo change deduplication_compression" + \
                  " deduplication=" + input_param.dedup_switch + " compression=" + input_param.dedup_switch
        ret_code, out, err = exec_shell_cmd(command)
        check_times = check_times + 1
    if ret_code != Result.SUCCESS:
        logging.error("Execute change deduplication_compression failed, err: %s." % std_err)
        return Result.FAILED
    logging.info("Execute change deduplication_compression success.")
    return Result.SUCCESS


def exec_main(args):
    if len(args) < 4:
        print("Input parameter error.")
        return Result.FAILED
    agent_root_path = args[1]
    param_num = args[3]
    init_log(agent_root_path)

    logging.info("Agent root path is " + agent_root_path)
    ret_code, message = read_input_parameter(param_num)
    if ret_code != Result.SUCCESS:
        logging.error("Read input paramter failed.")
        return ret_code

    input_param, ret_code = get_input_param_item(message)
    if ret_code != Result.SUCCESS:
        logging.error("Read input paramter item failed.")
        return ret_code

    link_status = get_dataturbo_link_status(input_param.storage_name)
    if link_status.link_type != input_param.link_type:
        logging.error("Link type does not match, cur: %s, input: %s." % (link_status.link_type, input_param.link_type))
        link_status.all_link_fault_flag = True

    if link_status.object_create_flag and link_status.all_link_fault_flag:
        logging.info("Dataturbo link has been created, all link status are fault.")
        delete_dataturbo_object(input_param.storage_name, agent_root_path)
        link_status.object_create_flag = False

    if not link_status.object_create_flag:
        logging.error("Device not create dataturbo link, now Create dataturbo link.")
        if create_dataturbo_object(input_param) != Result.SUCCESS:
            return Result.FAILED

        check_times = 0
        while check_times < 3:
            tmp_link_status = get_dataturbo_link_status(input_param.storage_name)
            if tmp_link_status.object_create_flag and (not tmp_link_status.all_link_fault_flag):
                logging.info("Dataturbo link create success, and status is normal.")
                return Result.SUCCESS
            time.sleep(5)
            check_times = check_times + 1
        logging.error("Dataturbo link not normal.")
        return Result.FAILED
    elif input_param.link_type == LinkType.IP:
        # 向已有的链路中添加系统的IP
        ret_code = update_link_ip(link_status.link_ip_list, input_param.ip_list, input_param.storage_name)
        if ret_code != 0:
            return Result.FAILED
    logging.info("Dataturbo link has been created, and status is normal.")
    return set_dedup_switch(input_param)


if __name__ == "__main__":
    ret = exec_main(sys.argv)
    sys.exit(ret)
