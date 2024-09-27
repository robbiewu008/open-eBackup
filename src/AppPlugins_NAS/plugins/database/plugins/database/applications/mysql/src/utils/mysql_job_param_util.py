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

from mysql import log
from mysql.src.common.constant import MySQLJsonConstant


class MysqlJobParamUtil:
    @staticmethod
    def get_mysql_version(job_json_param: dict, job_id: str, pid: str):
        """
        从备份或者恢复的任务参数中，读取mysql的版本信息
        :param job_json_param: 备份或者恢复的任务参数
        :param job_id: 任务id
        :param pid: 子任务id
        :return:
        """
        object_json = job_json_param.get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.PROTECTOBJECT, {})
        if not object_json:
            object_json = job_json_param.get(MySQLJsonConstant.JOB, {}).get(MySQLJsonConstant.TARGETOBJECT, {})
        if not object_json:
            log.error(f"Get object json failed. pid:{pid} jobId:{job_id}")
            return ""
        return object_json.get(MySQLJsonConstant.EXTENDINFO, {}).get(MySQLJsonConstant.VERSION, "")
