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

import pexpect

from common.common import execute_cmd, check_command_injection
from common.const import ParamConstant
from common.logger import Logger
from dameng.commons.common import get_path_user_and_group, matching_dameng_field, open_grep, cmd_grep, \
    check_path_user
from dameng.commons.const import DamengStrConstant, ArrayIndex, DamengStrFormat
from dameng.commons.const import ExecCmdResult

LOGGER = Logger().get_logger('dameng.log')


class DamengSource:

    @staticmethod
    def check_result_path():
        """
        check path exist or not
        """
        LOGGER.info("Check path exist or not.")
        if os.path.exists(ParamConstant.RESULT_PATH):
            return True
        else:
            LOGGER.error("The result file path does not exist.")
            return False

    @staticmethod
    def check_param_file_path():
        """
        check path exist or not
        """
        LOGGER.info("Check path exist or not.")
        if os.path.exists(ParamConstant.PARAM_FILE_PATH):
            return True
        else:
            LOGGER.error("The param file path does not exist.")
            return False

    @staticmethod
    def discover_application():
        """
        查看是否安装达梦数据库，以及安装用户
        :return:
        """
        LOGGER.info("Start discover dameng app.")
        #1. 查找达梦安装配置文件
        LOGGER.info("Step 1: searching for the dameng configure file.")
        cmd = f"ls -l {DamengStrConstant.DM_CONFIG_FILE}"
        return_code, out_info, _ = execute_cmd(cmd)
        if return_code != ExecCmdResult.SUCCESS:
            LOGGER.error("Dameng Application is not exists.")
            return False, "", ""

        #2. 获取配置文件用户、用户组信息
        LOGGER.info("Step 1: obtain user and user group info.")
        out_info_list = out_info.split(' ')
        if (len(out_info_list) < ArrayIndex.INDEX_FIRST_4):
            LOGGER.error("Failed to verify the file permission information.")
            return False, "", ""

        username, usergroup = out_info_list[ArrayIndex.INDEX_FIRST_2], out_info_list[ArrayIndex.INDEX_FIRST_3]
        if username == DamengStrConstant.USER_ROOT:
            #2.1 root用户安装，根据环境变量DM_HOME查找实际的安装用户
            dm_home_path = os.getenv(DamengStrConstant.ENV_DM_HOME)
            if not dm_home_path:
                LOGGER.error("Failed to obtain the environment variable DM_HOME.")
                return False, "", ""
            return_info, username, usergroup = get_path_user_and_group(dm_home_path)
            if not return_info:
                LOGGER.error("Failed to obtain the owner and group permissions.")
                return False, "", ""
        LOGGER.info("Discover dameng app succ.")
        return True, username, usergroup

    @staticmethod
    def get_dm_home_path(username):
        """
        获取达梦数据库根目录
        :param username: 安装用户名
        :return: 安装目录
        """
        LOGGER.info("Start obtaining the database install path.")
        #1. 获取安装用户的shell类型
        LOGGER.info("Step 1: obtain the shell type of the install user.")
        #dmdba:x:54323:54331::/home/dmdba:/bin/bash
        result = open_grep(username, DamengStrConstant.PWD_FILE)
        if not result:
            return ''
        shell_type = result[0].strip('\n').split(":")[ArrayIndex.INDEX_LAST_1]
        if shell_type != DamengStrConstant.SHELL_TYPE_BASH and shell_type != DamengStrConstant.SHELL_TYPE_SH:
            LOGGER.error(f"Failed to obtain user type information.")
            return ""
        cmd = DamengStrFormat.SU_CMD.format(username, shell_type, f"echo ${DamengStrConstant.ENV_DM_HOME}")
        bin_retutn_info = pexpect.run(cmd).decode()
        bin_retutn_info = bin_retutn_info.split("\r\n")[ArrayIndex.INDEX_FIRST_0]
        if not os.path.exists(bin_retutn_info):
            return ''
        if not check_path_user(bin_retutn_info, username):
            return ''
        return bin_retutn_info

    @staticmethod
    def get_bin_path(username):
        """
        获取达梦数据库安装目录
        :param username: 安装用户名
        :return: 安装目录
        """
        home_path = DamengSource.get_dm_home_path(username)
        if not home_path:
            return ""
        bin_path = os.path.join(home_path, "bin")
        return bin_path

    @staticmethod
    def get_script_path(username):
        """
        获取达梦数据库脚本目录
        :param username: 安装用户名
        :return: 安装目录
        """
        home_path = DamengSource.get_dm_home_path(username)
        if not home_path:
            return ""
        script_path = os.path.join(home_path, "script")
        return script_path

    @staticmethod
    def discover_all_server(bin_path, server_prefix="DmService"):
        """
        获取注册的所有实例服务
        :param bin_path: 数据库安装路径
        :return: 所有实例服务文件
        """
        LOGGER.info("Start discover all database services.")
        #1. 查找所有实例服务文件列表信息
        LOGGER.info("Step 1: obtain all database service files.")
        all_server_name = []
        cmd = f"ls -l {bin_path}"
        result = cmd_grep(server_prefix, cmd)
        if not result:
            LOGGER.error(f"Get server fail.")
            return all_server_name
        #2. 拆分查询结果
        # -rwxr-xr-x 1 dmdba dinstall 16126 Jul  4 19:54 DmServiceA
        LOGGER.info("Step 2: obtain all database service name.")
        for line in result:
            server_name = line.split(' ')[ArrayIndex.INDEX_LAST_1]
            if check_command_injection(server_name):
                continue
            all_server_name.append(server_name)
        LOGGER.info("Discover all database services succ.")
        return all_server_name

    @staticmethod
    def get_dmini_path(username, bin_path, server_name):
        """
        获取注册服务对应的dm.ini文件路径
        :param bin_path: 数据库安装目录
        :param server_name: 注册实例服务名
        :return:
        """
        LOGGER.info("Start get dm.ini path.")
        ini_path = ''
        result = open_grep("INI_PATH", os.path.join(bin_path, server_name))
        if not result:
            LOGGER.info(f"Get dm.ini path fail.")
            return ini_path
        ini_path = result[0].strip().split("=")[ArrayIndex.INDEX_LAST_1]
        return ini_path

    @staticmethod
    def get_instancename_port(dmini_path):
        LOGGER.info("Start obtaining the instance port and instance name of a database instance.")
        #1. 获取实例名称
        LOGGER.info("Step 1: obtaining the instance name.")
        result_info = []
        result = matching_dameng_field("INSTANCE_NAME", dmini_path)
        if len(result) != 1:
            LOGGER.error(f"Get instance name fail.{result}")
            return result_info
        instance_name = result[0]
        result_info.append(instance_name)

        #2. 获取实例的端口
        LOGGER.info("Step 1: obtaining the instance port.")
        result = matching_dameng_field("PORT_NUM", dmini_path)
        if len(result) != 1:
            LOGGER.error(f"Get instance port fail.")
            return []
        port = result[0]
        result_info.append(port)
        LOGGER.info("Obtain the instance info succ.")
        return result_info

    def get_all_dmini_path(self):
        LOGGER.info("Start get all dm.ini.")
        #1. 获取数据库软件安装用户
        LOGGER.info("Step 1: obtaining the install user.")
        dmini_path_list = []
        result_info, username, _ = self.discover_application()
        if not result_info:
            LOGGER.error("Failed to find the Dameng app.")
            return dmini_path_list

        #2. 获取所有数据库服务
        LOGGER.info("Step 2: obtaining all database service.")
        bin_path = self.get_bin_path(username)
        if not bin_path:
            LOGGER.error("Failed to obtain the installation path.")
            return dmini_path_list

        server_name_list = self.discover_all_server(bin_path)

        if not server_name_list:
            bin_path = "/etc/rc.d/init.d"
            server_name_list = self.discover_all_server(bin_path)
        for server_name in server_name_list:
            dmini_path = self.get_dmini_path(username, bin_path, server_name)
            dmini_path_list.append(dmini_path)
        LOGGER.info(f"Get all dm.ini end.{len(dmini_path_list)}")
        return dmini_path_list

    def get_big_version(self):
        LOGGER.info("Get dameng version.")
        version = ''
        #获取数据库安装信息
        result_type, user_info, _ = self.discover_application()
        if result_type:
            username = user_info
        else:
            LOGGER.error("Get dameng application fail.")
            return version
        LOGGER.info("Get bin path.")

        #获取数据库安装路径
        bin_path = self.get_bin_path(username)
        path = os.path.join(bin_path, "disql")

        #查看数据库版本
        cmd = f"su - {username} -c '{path} -h'"
        result = cmd_grep("disql", cmd)
        if not result:
            return version
        version_info = result[0]
        version = version_info.split(" ")[-1]
        LOGGER.info(f"Get version succ.version: {version}")
        if version == "V8":
            return version
        return "V7"