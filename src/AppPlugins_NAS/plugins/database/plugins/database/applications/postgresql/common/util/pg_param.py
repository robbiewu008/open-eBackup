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

from common.parse_parafile import ParamFileUtil
from common.logger import Logger
from validation.common.json_util import find_all_value_by_key
from validation.validator import ParamValidator

LOGGER = Logger().get_logger(filename="postgresql.log")


class JsonParam:
    @staticmethod
    def parse_param_with_jsonschema(pid):
        # postgresql的参数校验路径
        path = "postgresql/jsonschema/postgresql_base_define.json"
        agent_applications_jsonschema_path = "postgresql/jsonschema/pg_agent_applications_define.json"

        file_content = ParamFileUtil.parse_param_file_and_valid_by_schema(pid, path)
        # 校验agent_applications
        agent_applications_list = find_all_value_by_key(file_content, "agent_applications")
        for agent_applications in agent_applications_list:
            ParamValidator.valid_data_by_schema(json.loads(agent_applications), agent_applications_jsonschema_path)

        LOGGER.info("Param is valid!")

        return file_content
