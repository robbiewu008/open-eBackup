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
from common.const import RepositoryDataTypeEnum
from common.parse_parafile import ParamFileUtil
from tdsql.logger import log


class JsonParam:

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
    def get_data_path(param: dict):
        data_path = ''
        paths = param.get("job", {}).get("copies", [{}])[0].get("repositories", [{}])
        for path in paths:
            if path.get("repositoryType", -1) == RepositoryDataTypeEnum.DATA_REPOSITORY:
                data_path = path.get("path", [""])[0]
        if not data_path or check_command_injection(data_path):
            log.error("Fail get data path!")
            return ""
        return data_path

    @staticmethod
    def get_meta_path(param: dict):
        data_path = ''
        paths = param.get("job", {}).get("copies", [{}])[0].get("repositories", [{}])
        for path in paths:
            if path.get("repositoryType", -1) == RepositoryDataTypeEnum.META_REPOSITORY:
                data_path = path.get("path", [""])[0]
                break
        if not data_path or check_command_injection(data_path):
            log.error("Fail get meta path!")
            return ""
        return data_path

    @staticmethod
    def get_cache_path(param: dict):
        data_path = ''
        paths = param.get("job", {}).get("copies", [{}])[0].get("repositories", [{}])
        for path in paths:
            if path.get("repositoryType", -1) == RepositoryDataTypeEnum.CACHE_REPOSITORY:
                data_path = path.get("path", [""])[0]
        if not data_path or check_command_injection(data_path):
            log.error("Fail get cache path!")
            return ""
        return data_path

    @staticmethod
    def get_log_path(param: dict):
        data_path = ''
        paths = param.get("job", {}).get("copies", [{}])[0].get("repositories", [{}])
        for path in paths:
            if path.get("repositoryType", -1) == RepositoryDataTypeEnum.LOG_REPOSITORY:
                data_path = path.get("path", [""])[0]
        if not data_path or check_command_injection(data_path):
            log.error("Fail get cache path!")
            return ""
        return data_path
