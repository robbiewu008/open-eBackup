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

from goldendb.logger import log
from goldendb.handle.common.goldendb_common import check_repository_path
from common.common import check_command_injection
from common.const import RepositoryDataTypeEnum
from common.parse_parafile import ParamFileUtil
from goldendb.handle.common.const import SubJobName
from validation.common.json_util import find_all_value_by_key
from validation.validator import ParamValidator


class JsonParam:
    @staticmethod
    def parse_param_with_jsonschema(pid):
        path = "goldendb/jsonschema/goldendb_base_define.json"
        cluster_info_jsonschema_path = "goldendb/jsonschema/cluster_info_define.json"
        golden_db_jsonschema_path = "goldendb/jsonschema/golden_db_define.json"

        file_content = ParamFileUtil.parse_param_file_and_valid_by_schema(pid, path)
        # 校验clusterInfo
        cluster_info_list = find_all_value_by_key(file_content, "clusterInfo")
        for cluster_info in cluster_info_list:
            ParamValidator.valid_data_by_schema(json.loads(cluster_info), cluster_info_jsonschema_path)

        # 校验GoldenDB
        golden_db_list = find_all_value_by_key(file_content, "GoldenDB")
        for golden_db in golden_db_list:
            ParamValidator.valid_data_by_schema(json.loads(golden_db), golden_db_jsonschema_path)

        return file_content

    @staticmethod
    def get_data_path(param: dict):
        data_path = ''
        repositories = param.get("job", {}).get("copies", [{}])[0].get("repositories", [{}])
        for reps in repositories:
            if reps.get("repositoryType", -1) == RepositoryDataTypeEnum.DATA_REPOSITORY:
                path_list = reps.get("path", [])
                data_path = check_repository_path(path_list)
        if not data_path or check_command_injection(data_path):
            log.error("Fail get data path!")
            return ""
        return data_path

    @staticmethod
    def get_meta_path(param: dict):
        data_path = ''
        repositories = param.get("job", {}).get("copies", [{}])[0].get("repositories", [{}])
        for reps in repositories:
            if reps.get("repositoryType", -1) == RepositoryDataTypeEnum.META_REPOSITORY:
                path_list = reps.get("path", [])
                data_path = check_repository_path(path_list)
        if not data_path or check_command_injection(data_path):
            log.error("Fail get meta path!")
            return ""
        return data_path

    @staticmethod
    def get_cache_path(param: dict):
        data_path = ''
        repositories = param.get("job", {}).get("copies", [{}])[0].get("repositories", [{}])
        for reps in repositories:
            if reps.get("repositoryType", -1) == RepositoryDataTypeEnum.CACHE_REPOSITORY:
                path_list = reps.get("path", [])
                data_path = check_repository_path(path_list)
        if not data_path or check_command_injection(data_path):
            log.error("Fail get cache path!")
            return ""
        return data_path

    @staticmethod
    def get_log_path(param: dict):
        log_path = ''
        job_dict = param.get("job", {})
        if not job_dict.get("extendInfo", {}).get("restoreTimestamp", "") or len(job_dict.get("copies", [{}])) <= 1:
            log.warning("Log copy info is empty")
            return ""
        copy_dict = job_dict.get("copies", [{}])[-1]
        repositories_info = copy_dict.get("repositories", [])
        for reps in repositories_info:
            if reps.get("repositoryType") == RepositoryDataTypeEnum.LOG_REPOSITORY and reps.get("path"):
                path_list = reps.get("path", [])
                path = check_repository_path(path_list)
                log_path = os.path.dirname(path)
        if not log_path or check_command_injection(log_path):
            log.error("Fail get log path!")
            return ""
        return os.path.join(log_path)

    @staticmethod
    def get_sub_job_name(param):
        sub_name = param.get("subJob", {}).get("jobName", "")
        if sub_name not in (
                SubJobName.EXEC_LOG_BACKUP, SubJobName.EXEC_BACKUP, SubJobName.EXEC_COPY_BINLOG, SubJobName.MOUNT,
                SubJobName.EXEC_REPORT_DATA_SIZE):
            log.error(f"{sub_name} not found in sub jobs!")
            return ""
        return sub_name
