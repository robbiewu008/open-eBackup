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
from bs4 import BeautifulSoup

from common.cleaner import clear
from common.logger import Logger
from common.parse_parafile import get_env_variable
from kingbase.common.const import PexpectResult
from kingbase.common.error_code import ErrorCode

LOGGER = Logger().get_logger("kingbase.log")


def execute_cmd_and_parse_res(pid, cmd, prefix="application_auth_authPwd"):
    child = None
    db_pwd = None
    try:
        child = pexpect.spawn(cmd, timeout=10, encoding="utf-8")
        LOGGER.info("Exec cmd: %s", cmd)
        index = child.expect(PexpectResult.DB_LOGIN_PASSWORD)
        if index in (0, 1):
            LOGGER.error(f"Login database error!")
            child.close()
            return ErrorCode.AUTH_INFO_INCORRECT, ""
        db_pwd = get_env_variable(f"{prefix}_{pid}")
        child.sendline(db_pwd)
        db_result = child.expect(PexpectResult.HTML_RESULT)
        if db_result in (0, 1):
            child.close()
            LOGGER.error("Password incorrect!  db_result: %s", db_result)
            return ErrorCode.AUTH_INFO_INCORRECT, ""
        html_res = child.before
        LOGGER.info(f"Success to login pgsql by db verify!pid:%s", pid)
        child.close()
        return ErrorCode.SUCCESS, parse_html_result(html_res)
    finally:
        clear(db_pwd)
        if child:
            child.close()


def parse_html_result(html_res):
    LOGGER.info(f"Begin to parse data dir :%s", )
    soup = BeautifulSoup(html_res, "html.parser")
    trs = soup.find_all(name="tr")
    _soup = BeautifulSoup(str(trs[1]), "html.parser")
    table_td = _soup.find_all(name='td', attrs={"align": "left"})
    data = table_td[0].get_text()
    LOGGER.info("Success to parse data dir :%s", data)
    return data
