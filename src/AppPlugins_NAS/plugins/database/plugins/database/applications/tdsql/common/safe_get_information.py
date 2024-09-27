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

from common.common import check_command_injection
from common.parse_parafile import ParamFileUtil
from tdsql.common.const import TdsqlSubJobName
from tdsql.logger import log


class ResourceParam:

    def __init__(self, pid):
        self.pid = pid
        # 通过pid读取到相应的参数文件
        try:
            self._body_param = ParamFileUtil.parse_param_file(self.pid)
        except Exception as err:
            raise Exception(f"Failed to parse job param file for {err}") from err
        if not self._body_param:
            raise Exception(f"Failed to parse job param file is none")

    @staticmethod
    def get_sub_job_name(param):
        sub_name = param.get("subJob", {}).get("jobName", "")
        if sub_name not in (TdsqlSubJobName.SUB_OSS, TdsqlSubJobName.SUB_EXEC, TdsqlSubJobName.SUB_BINLOG,
                            TdsqlSubJobName.SUB_FLUSH_LOG, TdsqlSubJobName.SUB_RM_BINLOG):
            log.error(f"{sub_name} not found in sub jobs!")
            return ""
        return sub_name

    @staticmethod
    def get_group_sub_job_name(param):
        sub_name = param.get("subJob", {}).get("jobName", "")
        if sub_name not in (
                TdsqlSubJobName.SUB_GROUP_MOUNT_BIND, TdsqlSubJobName.SUB_GROUP_EXEC, TdsqlSubJobName.SUB_GROUP_BINLOG):
            log.error(f"{sub_name} not found in sub jobs!")
            return ""
        return sub_name

    def get_param(self):
        return self._body_param

    def get_business_addr(self):
        business_addr = self._body_param.get("appEnv").get("extendInfo", {}).get("businessAddr", "")
        if check_command_injection(business_addr):
            log.error("The parameter verification fails.")
            return ''
        return business_addr
