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

from mongodb.comm.const import ParamField
from mongodb.comm.utils import translate_to_snake_name


class BaseParam:
    """
    参数解析基本类，以参数字典构造，参数对象，初始对应基本字段的属性，可直接访问。
    具体参数对象在继承此类的基础上实现不同对象的特定参数获取及检验方法
    """
    default_field = (
        ParamField.JOB,
        ParamField.SUB_JOB,
        ParamField.APPLICATION,
        ParamField.APP_ENV,
        ParamField.EXTEND_INFO
    )

    def __init__(self, param_dict):
        self.param_dict = param_dict
        self.job = {}
        self.sub_job = {}
        self.application = {}
        self.app_env = {}
        self.extend_info = {}
        self._parse_default_field(param_dict)

    def get(self, field, param_dict: dict = None):
        if not param_dict:
            param_dict = self.param_dict
        value = param_dict.get(field, {})
        return value

    def dict(self):
        return self.param_dict

    def _parse_default_field(self, param_dict, fields=None):
        if not param_dict:
            return
        if fields is None:
            fields = BaseParam.default_field
        for field, value in param_dict.items():
            if field in fields:
                attr = translate_to_snake_name(field)
                setattr(self, attr, value)