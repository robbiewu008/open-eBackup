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
from typing import List
import configparser
import jsonschema
from jsonschema import validators

from common.exception.common_exception import ErrCodeException
from common.file_common import log
from validation.common.error_code import ErrorCode
from validation.common.jsonschema_utils import add_additional_properties, check_string_has_pattern, check_array_has_item
from validation.common.merge_schema_util import merge_schema
from validation.common.validation_consts import ValidationConsts


class ParamValidator:
    @staticmethod
    def valid(data, paths: List[str]):
        """
        根据jsonschema校验参数,先校验公共参数规则，再校验传入的规则
        """
        if ParamValidator.is_validation_enable():
            # 先校验公共参数
            ParamValidator.valid_by_jsonschema(data, ValidationConsts.DEFAULT_SCHEMA_DEFINE_PATH)
            # 再校验定制规则
            for path in paths:
                ParamValidator.valid_by_jsonschema(data, path)
        return True

    @staticmethod
    def valid_data(data, path: str):
        """
        根据jsonschema校验参数,校验公共参数与定制参数规则
        """
        if ParamValidator.is_validation_enable():
            # 先校验公共参数
            validator = ParamValidator.get_validator(path)
            try:
                validator.validate(data)
                return True
            except jsonschema.exceptions.ValidationError as e:
                ParamValidator.throw_exception(e)
        return True

    @staticmethod
    def throw_exception(e):
        if e.validator == "additionalProperties":
            message = f"Data is invalid. {e.message} in [{e.json_path}]"
        elif e.validator == "type":
            message = f"Data is invalid. [{e.json_path}] type is not {e.validator_value}"
        elif e.validator == "pattern":
            message = f"Data is invalid. [{e.json_path}] is not match pattern {e.validator_value}"
        else:
            message = f"Data is invalid. invalid param: [{e.json_path}], schema is: {e.schema}"
        log.error(message)
        # 异常脱敏， 不要打印参数的值，防止敏感数据被打印
        e.instance = "****"
        e.message = "****"
        raise ErrCodeException(ErrorCode.PARAMS_IS_INVALID, message=message) from e

    @staticmethod
    def is_validation_enable():
        config = configparser.ConfigParser()
        config.read(ValidationConsts.DEFAULT_CONFIG_PATH)
        allow_check = config.getboolean('General', 'AllowCheck')
        return allow_check

    @staticmethod
    def valid_data_by_schema(data, path: str):
        """
        根据jsonschema校验参数,仅校验传入的规则
        """
        if ParamValidator.is_validation_enable():
            custom_define_schema = ParamValidator.read_jsonschema(ValidationConsts.JSON_SCHEMA_PATH_PREFIX + path)
            add_additional_properties(custom_define_schema)
            check_string_has_pattern(custom_define_schema)
            try:
                jsonschema.validate(data, custom_define_schema, format_checker=jsonschema.FormatChecker())
            except jsonschema.exceptions.ValidationError as e:
                ParamValidator.throw_exception(e)

    @staticmethod
    def valid_by_jsonschema(data, path):
        """
        根据jsonschema校验参数
        """
        schema = ParamValidator.read_jsonschema(ValidationConsts.JSON_SCHEMA_PATH_PREFIX + path)
        try:
            jsonschema.validate(data, schema, format_checker=jsonschema.FormatChecker())
            return True
        except jsonschema.exceptions.ValidationError as e:
            ParamValidator.throw_exception(e)
        return True

    @staticmethod
    def read_jsonschema(path):
        """
        读取参数校验jsonschema文件
        """
        if not os.path.exists(path):
            raise ErrCodeException(ErrorCode.PARAMS_IS_INVALID, message=f"File:{path} not exist")

        try:
            with open(path, "r", encoding='utf-8') as tmp:
                result = json.load(tmp)
        except Exception as ex:
            log.error(f"read_jsonschema Exception: {ex}")
            raise ErrCodeException(ErrorCode.PARAMS_IS_INVALID, message="read_jsonschema, parse file failed") from ex
        return result

    @staticmethod
    def get_validator(path: str):
        common_define_schema = ParamValidator.read_jsonschema(
            ValidationConsts.JSON_SCHEMA_PATH_PREFIX + ValidationConsts.DEFAULT_JSON_SCHEMA_PATH)
        custom_template_define_schema = ParamValidator.read_jsonschema(
            ValidationConsts.JSON_SCHEMA_PATH_PREFIX + ValidationConsts.EXTEND_INFO_SCHEMA_DEFINE_PATH)
        custom_define_schema = ParamValidator.read_jsonschema(ValidationConsts.JSON_SCHEMA_PATH_PREFIX + path)

        # 合并应用定制规则和通用extend规则
        merged_custom_schema = merge_schema(custom_template_define_schema, custom_define_schema)

        add_additional_properties(common_define_schema)
        check_string_has_pattern(common_define_schema)
        check_array_has_item(common_define_schema)

        add_additional_properties(merged_custom_schema)
        check_string_has_pattern(merged_custom_schema)
        check_array_has_item(merged_custom_schema)

        resolver = validators.RefResolver("", {},
                                          store={'http://example.com/custom_jsonschema.json': merged_custom_schema,
                                                 'http://example.com/common_define.json': common_define_schema})
        return validators.Draft7Validator(
            common_define_schema,
            resolver,
            format_checker=jsonschema.FormatChecker()
        )
