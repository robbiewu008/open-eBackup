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
import glob
import importlib.util
import os
import sys

from app.protection.object.service.validator import Validator


class ValidatorManager(object):

    def __init__(self):
        self._extends_dir: str = "validators"
        self._package: str = "app.protection.object.service"
        self._load_validators()

    def _load_validators(self):
        extend_path = os.path.join(os.path.abspath(
            os.path.dirname(__file__)), self._extends_dir)
        py_files = glob.glob("{}/*.py".format(extend_path).rstrip("/"))

        for py_file in py_files:
            if py_file.endswith("__init__.py"):
                continue
            script_module_name = "{}.{}".format(
                self._package, os.path.basename(py_file)[:-len(".py")])
            spec = importlib.util.spec_from_file_location(
                script_module_name, py_file)

            script_module = importlib.util.module_from_spec(spec)
            sys.modules[script_module_name] = script_module
            spec.loader.exec_module(script_module)

    def do_validate(self, validate_rule, validate_param):
        """
        根据校验规则为True获取校验器, 并执行处理逻辑
        :param validate_rule: 资源类型
        :param validate_param: 资源信息
        :return:
        """
        for sub_class in Validator.__subclasses__():
            if sub_class.is_support(validate_rule):
                sub_class.do_validate(validate_param)
