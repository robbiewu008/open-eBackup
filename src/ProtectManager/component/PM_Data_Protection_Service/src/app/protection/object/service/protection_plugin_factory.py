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
import importlib
import os

from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException


class ProtectionPluginFactory(object):
    _factory = None

    @staticmethod
    def _list_plugins():
        error_result = []
        plugins = []
        plugins_dir = os.path.join(os.path.dirname(__file__), 'plugins')
        if not os.path.isabs(plugins_dir):
            return error_result
        for file in os.listdir(plugins_dir):
            name, ext = os.path.splitext(file)
            if name != '__init__' and ext == '.py':
                plugins.append('app.protection.object.service.plugins.%s' % name)
        return plugins

    @classmethod
    def get_plugin_module(cls, res_sub_type):
        if cls._factory is None:
            factory = {}
            for module in cls._list_plugins():
                module = importlib.import_module(module)
                for service in module.SERVICES:
                    factory[service] = module
            cls._factory = factory
        return cls._factory.get(res_sub_type)

    @classmethod
    def create_plugin(cls, res_sub_type):
        module = cls.get_plugin_module(res_sub_type)
        if module is None:
            raise EmeiStorBizException(CommonErrorCodes.SYSTEM_ERROR, message="Plugin is not exists")
        return module.create()


protection_plugin_factory = ProtectionPluginFactory()
