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

import pexpect

from common.cleaner import clear
from common.common import execute_cmd, check_command_injection
from common.const import AuthType
from common.logger import Logger
from common.util.check_utils import is_port
from dameng.commons.common import get_env_value, check_port, cmd_grep, open_grep
from dameng.commons.const import DamengStrFormat, DamengStrConstant, ArrayIndex
from dameng.resource.damengsource import DamengSource

LOGGER = Logger().get_logger("dameng_tool.log")


class DmSqlTool:

    def __init__(self, dblogininfo_dir):
        self.ip = dblogininfo_dir.get("ip", '127.0.0.1')
        self.port = str(dblogininfo_dir.get("port", 0))
        self.auth_type = dblogininfo_dir.get("auth_type", '')
        self.install_user = ""
        self.bin_path = ""
        self.timeout = 10
        self.child = None
        # 当使用查询的版本连接失败时是否尝试其他版本
        self.is_connect_other_version = dblogininfo_dir.get("is_connect_other_version", True)
        self.version = dblogininfo_dir.get("big_version", '')
        if self.auth_type == AuthType.APP_PASSWORD:
            self.userkey = dblogininfo_dir.get("userkey", '')
            self.pwdkey = dblogininfo_dir.get("pwdkey", '')

    def exec_disql_by_app_password(self, cmd_list=(), mpp_type=''):
        result = []
        status = True
        ret, info = self.login_database(mpp_type)
        if not ret:
            info = info.replace("\r\n", "\n")
            return False, [info]
        # 执行sql
        code = self.child.expect(["SQL>"])
        for disql in cmd_list:
            self.child.sendline(s=disql)
            code = self.child.expect(["\[-", "SQL>", pexpect.TIMEOUT, pexpect.EOF])
            if code == 1:
                info = self.child.before
            else:
                status = False
                self.child.expect(["SQL>"])
                info = self.child.before
                LOGGER.error(f"Failed to execute the SQL.")
            info = info.replace("\r\n", "\n")
            result.append(info)
        self.child.close()
        return status, result

    def login_database(self, mpp_type):
        LOGGER.info("Start login_database.")
        user = get_env_value(self.userkey)
        password = get_env_value(self.pwdkey)
        expect_list_dm8 = [
            ("server", "服务名"), ("username", "用户名"), ("password", "密码"),
            ("SSL path", "SSL路径"), ("SSL PWD", "SSL密码"), ("UKEY NAME", 'UKEY名称'),
            ("UKEY PIN", "UKEY PIN码"), ("MPP TYPE", "MPP类型"), ("y/n", "y/n"),
            ("protocol type", "协议类型"), (f"127.0.0.1:{self.port}", f"127.0.0.1:{self.port}")
        ]
        send_list_dm8 = ["login", f"127.0.0.1:{self.port}", user, password, '', '', '', '', mpp_type, '', '']
        expect_list_dm7 = [
            ("server", "服务名"), ("username", "用户名"),
            ("password", "密码"), ("port", "端口号"),
            ("SSL path", "SSL路径"), ("SSL PWD", "SSL密码"),
            ("MPP TYPE", "MPP类型"), ("y/n", "y/n"),
            ("protocol type", "协议类型"), (f"\[127.0.0.1:{self.port}\]", f"\[127.0.0.1:{self.port}\]")
        ]
        send_list_dm7 = ["login", "127.0.0.1", user, password, self.port, '', '', mpp_type, '', '']
        if self.version:
            dm_version = self.version
        else:
            dm_version = self.get_databser_version()
        LOGGER.info(f"get version {dm_version}")
        user_locale = self.get_user_encoding()
        if not dm_version or not user_locale:
            clear(password)
            return False, ''
        ret, info = self.make_exec_child(user_locale)
        if not ret:
            clear(password)
            return False, self.child.before
        login_dict = {
            "V7": (send_list_dm7, expect_list_dm7),
            "V8": (send_list_dm8, expect_list_dm8)
        }
        ret, info = self.login_dm_all_version(dm_version, login_dict, user_locale)
        clear(password)
        LOGGER.info(f"end login_database port.")
        return ret, info

    def make_exec_child(self, user_locale):
        cmd = DamengStrFormat.DISQL_LOGIN_PASSWORD.format(self.install_user, self.bin_path)
        self.child = pexpect.spawn(cmd, timeout=self.timeout, encoding=user_locale)
        code = self.child.expect(["SQL>", pexpect.TIMEOUT])
        if code != 0:
            LOGGER.info(f"Failed to load the disql tool.code: {code}.")
            return False, self.child.before
        return True, self.child.before

    def login_dm_all_version(self, version, login_dict, user_locale):
        # 现网存在Dameng 8。7000版本，大版本是8但回显结构为7类型，做此适配
        login_info_list = login_dict.get(f"{version}", "")
        ret, info = self.login_dm(login_info_list)
        if ret:
            LOGGER.info("Login database succ.")
            return ret, info
        else:
            if not self.is_connect_other_version:
                LOGGER.error("just connect once")
                return ret, info

            # 如果版本不在兼容性范围内，或者根据版本预期的返回值不匹配，走以下逻辑
            LOGGER.warning(f"dm version and expect list not match")
            for version, login_info_list in login_dict.items():
                self.child.close()
                ret, info = self.make_exec_child(user_locale)
                if not ret:
                    return False, self.child.before
                LOGGER.warning(f"trying dm {version}")
                ret, info = self.login_dm(login_info_list)
                if ret:
                    LOGGER.warning(f"Looks like {version} matches")
                    return ret, info
            return ret, info

    def login_dm(self, login_check_info):
        if not login_check_info:
            # 不在兼容性版本范围之内
            LOGGER.error(f"Out of compatibility")
            return False, self.child.before
        send_list = login_check_info[0]
        expect_list = login_check_info[1]
        for index, cmd in enumerate(send_list):
            self.child.sendline(cmd)
            code = self.child.expect(
                [expect_list[index][0], expect_list[index][1], "SQL>", pexpect.EOF, pexpect.TIMEOUT])
            if code not in (0, 1):
                LOGGER.error("Expect %s fail.", expect_list[index])
                return False, self.child.before
        return True, self.child.before

    def exec_disql_by_os(self, cmd_list=(), mpp_type=''):
        LOGGER.info("Start exec disql by os.")
        result = []
        status = True
        ret, child = self.login_database_by_os(mpp_type)
        if not ret:
            LOGGER.error("Create SOCKET fail.")
            return False, result
        code = child.expect(["SQL>"])
        for disql in cmd_list:
            child.sendline(s=disql)
            sql_exec_code = child.expect(["\[-", "SQL>", pexpect.TIMEOUT, pexpect.EOF])
            if sql_exec_code == 1:
                msg = child.before
            else:
                status = False
                child.expect(["SQL>"])
                msg = child.before
                LOGGER.error(f"Fail exec sql.")
            info = msg.replace("\r\n", "\n")
            result.append(info)
        child.close()
        return status, result

    def login_database_by_os(self, mpp_type):
        child = None
        if not check_port(self.port):
            LOGGER.error("Failed to check the port.")
            return False, child

        user_locale = self.get_user_encoding()
        if not user_locale:
            LOGGER.error("Failed to get the locale.")
            return False, child
        if not mpp_type:
            cmd = DamengStrFormat.DISQL_LOGIN_OS.format(self.install_user, self.bin_path, self.ip, self.port)
        else:
            cmd = DamengStrFormat.DISQL_LOGIN_OS_WITH_MPP_TYPE.format(self.install_user, self.bin_path, self.ip,
                                                                      self.port)
        child = pexpect.spawn(cmd, timeout=self.timeout, encoding=user_locale)
        create_socket_code = child.expect(["Create SOCKET connection failure",
                                           f"{self.ip}:{self.port}", pexpect.TIMEOUT, pexpect.EOF])
        if create_socket_code != 1:
            LOGGER.error("Create SOCKET fail.")
            return False, child
        return True, child

    def run_disql_tool(self, disql_cmd=(), timeout=10, mpp_type=''):
        """
        执行具体disql命令
        :param disql_cmd: 命令列表
        :return: 命令执行结果列表
        """
        LOGGER.info("Start run disql tool.")
        self.timeout = timeout
        result = []
        status = False
        return_type, self.install_user, _ = DamengSource.discover_application()
        if not return_type:
            LOGGER.error("Get install user fail.")
            return status, result
        self.bin_path = DamengSource.get_bin_path(self.install_user)
        if not self.bin_path:
            LOGGER.error("Get bin_path user fail.")
            return status, result
        if not is_port(self.port):
            LOGGER.error("Invalid port information.port:%s", self.port)
            return status, result
        if disql_cmd:
            if self.auth_type == AuthType.OS_PASSWORD:
                status, result = self.exec_disql_by_os(cmd_list=disql_cmd, mpp_type=mpp_type)
            elif self.auth_type == AuthType.APP_PASSWORD:
                status, result = self.exec_disql_by_app_password(cmd_list=disql_cmd, mpp_type=mpp_type)
            else:
                LOGGER.error("Unknow auth_type.")
        return status, result

    def get_databser_version(self):
        LOGGER.info("Start get database version.")
        cmd = f"su - {self.install_user} -c '{self.bin_path}/disql -h'"
        result = cmd_grep("disql", cmd)
        # 1：返回结果至少1行
        if not result:
            LOGGER.error("Get version fail.")
            return ''
        version_info = result[0]
        version = version_info.strip(" ").split(" ")
        if len(version) < 2:
            LOGGER.error("Failed to parse the result.")
            return ''
        version = version[1]
        if version == 'V8':
            return 'V8'
        return 'V7'

    def get_db_real_version(self, mpp_type=''):
        """
        只登录使用此方法.现网V8版本返回V7，影响资源接入。现网未影响备份，恢复功能。故暂未对备份，恢复做处理
        :param mpp_type:
        :return: dameng version
        """
        LOGGER.info("Start get_db_real_version")
        return_type, self.install_user, _ = DamengSource.discover_application()
        if not return_type:
            LOGGER.error("Get install user fail.")
            return ""
        self.bin_path = DamengSource.get_bin_path(self.install_user)
        if not self.bin_path:
            LOGGER.error("Get bin_path user fail.")
            return ""
        versions = ["V7", "V8"]
        db_version = self.get_databser_version()
        self.version = db_version
        ret = False
        if self.auth_type == AuthType.OS_PASSWORD:
            ret, info = self.login_database_by_os(mpp_type)
        elif self.auth_type == AuthType.APP_PASSWORD:
            ret, child = self.login_database(mpp_type)
        else:
            LOGGER.error("Unknow auth_type.")
        if not ret:
            versions.remove(db_version)
            LOGGER.error(f"end get_db_real_version:{versions[0]}")
            return versions[0]
        LOGGER.info(f"end get_db_real_version:{db_version}")
        return db_version


    def get_user_shell_type(self):
        LOGGER.info("Start obtain the shell type of the install user.")
        # 1.读取shell类型
        # dmdba:x:54323:54331::/home/dmdba:/bin/bash
        result = open_grep(self.install_user, DamengStrConstant.PWD_FILE)
        if not result:
            LOGGER.error("Get user shell type fail.")
            return ''

        # 2. 截取shell类型
        shell_type = result[0].strip('\n').split(":")[ArrayIndex.INDEX_LAST_1]
        if shell_type != DamengStrConstant.SHELL_TYPE_BASH and shell_type != DamengStrConstant.SHELL_TYPE_SH:
            LOGGER.error(f"Failed to obtain user type information.")
            return ""
        return shell_type

    def get_user_encoding(self):
        LOGGER.info("Start get user encoding.")
        # 1. 获取用户shell类型
        shell_type = self.get_user_shell_type()
        if not shell_type:
            return ""

        # 2. 获取用户locale
        cmd = DamengStrFormat.SU_CMD.format(self.install_user, shell_type, f"echo ${DamengStrConstant.ENV_LANG}")
        ret, out, err = execute_cmd(cmd)
        if not ret or not out:
            LOGGER.error(f"Get user encoding fail, ret: {ret}, out: {out}, err: {err}.")
            return ""
        out = out.strip('\n')
        LOGGER.info(f"Obtain user locale succ, locale: {out}. ")
        if check_command_injection(out):
            LOGGER.error(f"Check locale fail, out: {out}.")
            return ""

        user_locale = out.split(".")
        if len(user_locale) != ArrayIndex.INDEX_FIRST_2:
            LOGGER.error("The locale format is incorrect.")
            return ""
        return user_locale[ArrayIndex.INDEX_LAST_1]
