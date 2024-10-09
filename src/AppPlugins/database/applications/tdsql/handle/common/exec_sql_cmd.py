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

import pymysql

from tdsql.handle.common.const import ErrorCode
from tdsql.handle.common.tdsql_exception import ErrCodeException
from tdsql.logger import log


def get_mysql_db_session(host_ip, port, user, pwd, socket=""):
    """
    连接MySQL数据库
    @:param host_ip 主机ip
    @:param port mysql端口号
    @:param user 用户名
    @:param pwd 密码
    @:param socket 套接字文件路径
    """
    try:
        return pymysql.connect(user=user,
                               host=host_ip,
                               password=pwd,
                               port=int(port),
                               unix_socket=socket)
    except Exception as except_str:
        log.error(f"Connect MySQL :{host_ip} service failed!")
        raise ErrCodeException(ErrorCode.ERROR_AUTH, "Check connectivity: auth info error!") from except_str


def exec_mysql_sql_cmd(host_ip, port, user, pwd, sql_str):
    """
    连上数据库,执行一条sql命令,返回执行结果
    @:param host_ip 主机ip
    @:param port mysql端口号
    @:param user 用户名
    @:param pwd 密码
    @:param sql_str 执行的sql语句
    """
    try:
        mysql_connection = get_mysql_db_session(host_ip, port, user, pwd)
    except pymysql.Error as except_str:
        log.error(f"Connect MySQL service failed!")
        return False, "raise except"
    cursor = mysql_connection.cursor()
    cursor.execute(sql_str)
    results = cursor.fetchall()
    mysql_connection.close()
    return True, results
