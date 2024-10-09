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

from common.const import ParamConstant


class ValidationConsts:
    DEFAULT_JSON_SCHEMA_PATH = "validation/jsonschema/common_define.json"
    DEFAULT_SCHEMA_DEFINE_PATH = "validation/jsonschema/common_define2.json"
    EXTEND_INFO_SCHEMA_DEFINE_PATH = "validation/jsonschema/extend_info_jsonschema.json"
    DEFAULT_CONFIG_PATH = f"{os.path.abspath(os.path.dirname(__file__))}/../config.ini"
    JSON_SCHEMA_PATH_PREFIX = f"{ParamConstant.BIN_PATH}/applications/"
