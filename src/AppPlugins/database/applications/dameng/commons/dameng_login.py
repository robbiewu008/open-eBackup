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
import re
from pathlib import Path
from typing import List, Tuple

import pexpect

from common.logger import Logger
from common.security.anonym_utils.anonymity import Anonymity

LOGGER = Logger().get_logger("dameng_login.log")


class DamengLogin:
    def __init__(self, child: pexpect.spawn, user: str, password: str, port: str, mpp_type: str, version: str = 'V7',
                 protocol_type: str = '', ssl_path: str = '', ssl_pwd: str = '', ukey_name: str = '',
                 ukey_pin: str = '', config_template: str = 'prompt_response_map_{version}.json'):
        """
        初始化登录参数

        :param child: pexpect.spawn
        :param user: 用户名
        :param password: 密码
        :param port: 端口号
        :param mpp_type: MPP 类型
        :param protocol_type: 协议类型（可选）
        :param version: dameng版本（可选）
        :param ssl_path: SSL 路径（可选）
        :param ssl_pwd: SSL 密码（可选）
        :param ukey_name: UKEY 名称（可选）
        :param ukey_pin: UKEY PIN 码（可选）
        """
        self.child = child
        self.user = user
        self.password = password
        self.port = port
        self.mpp_type = mpp_type
        self.protocol_type = protocol_type
        self.ssl_path = ssl_path
        self.ssl_pwd = ssl_pwd
        self.ukey_name = ukey_name
        self.ukey_pin = ukey_pin

        config_file = config_template.format(version=version)
        # 获取上级目录
        parent_dir = Path(__file__).resolve().parent.parent
        cur_path = os.path.join(parent_dir, f"conf_file/{config_file}")
        with open(cur_path, 'r', encoding='utf-8') as f:
            mappings = json.load(f)
            self.prompt_response_map: List[Tuple[re.Pattern, str]] = []
            for item in mappings:
                pattern = re.compile(item['pattern'], re.IGNORECASE)
                response = item['response'].format(
                    user=self.user,
                    password=self.password,
                    port=self.port,
                    mpp_type=self.mpp_type,
                    protocol_type=self.protocol_type,
                    ssl_path=self.ssl_path,
                    ssl_pwd=self.ssl_pwd,
                    ukey_name=self.ukey_name,
                    ukey_pin=self.ukey_pin
                )
                self.prompt_response_map.append((pattern, response))

    def login(self):
        """
        执行登录过程
        """
        # 合并所有需要匹配的提示
        expect_patterns = ([pattern for (pattern, _) in self.prompt_response_map] + [pexpect.EOF, pexpect.TIMEOUT])

        try:
            return self.login_dameng(expect_patterns)
        except pexpect.exceptions.EOF:
            LOGGER.error("子进程已结束（EOF）")
            return False, self.child.before
        except pexpect.exceptions.TIMEOUT:
            LOGGER.error("等待输入超时（TIMEOUT）")
            return False, self.child.before
        except Exception as e:
            LOGGER.error(f"发生异常: {Anonymity.process(Anonymity.process(e))}")
            return False, self.child.before

    def login_dameng(self, expect_patterns):
        self.child.sendline("login")
        while True:
            index = self.child.expect(expect_patterns)

            # 匹配提示-响应映射
            if index < len(self.prompt_response_map):
                pattern, response = self.prompt_response_map[index]
                if response == "end":
                    return True, self.child.before
                # 仅在响应不为空时发送
                if response:
                    self.child.sendline(response)
                else:
                    # 发送空响应（即换行）
                    self.child.sendline('')
            elif index == len(expect_patterns) - 2:
                # 匹配到 EOF
                LOGGER.error("子进程已结束")
                return False, self.child.before
            elif index == len(expect_patterns) - 1:
                # 匹配到 TIMEOUT
                LOGGER.error("等待输入超时")
                return False, self.child.before
