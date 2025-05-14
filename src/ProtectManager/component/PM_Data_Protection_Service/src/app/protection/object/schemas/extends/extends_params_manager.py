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

from app.protection.object.schemas.extends.base_ext_param import BaseExtParam
from app.protection.object.schemas.extends.params.common_ext_param import CommonResourceExtParam


class ExtendsParamsManager(object):

    def __init__(self):
        self._extends_dir: str = "params"
        self._package: str = "app.protection.object.schemas.extends.params"
        self._load_classes()
        self._ext_parameters: dict = {}

    def _load_classes(self):
        extend_path = os.path.join(os.path.abspath(
            os.path.dirname(__file__)), self._extends_dir)
        py_files = glob.glob("{}/*.py".format(extend_path).rstrip("/"))
        py_files += glob.glob("{}/*.pyc".format(extend_path).rstrip("/"))

        for py_file in py_files:
            if py_file.endswith("__init__.py") or py_file.endswith("__init__.pyc"):
                continue
            if py_file.endswith(".pyc"):
                script_module_name = "{}.{}".format(
                    self._package, os.path.basename(py_file)[:-len(".pyc")])
            else:
                script_module_name = "{}.{}".format(
                    self._package, os.path.basename(py_file)[:-len(".py")])
            spec = importlib.util.spec_from_file_location(
                script_module_name, py_file)

            script_module = importlib.util.module_from_spec(spec)
            if not sys.modules.get(script_module_name):
                sys.modules[script_module_name] = script_module
                spec.loader.exec_module(script_module)

    @staticmethod
    def get_all_ext_params():
        return tuple(BaseExtParam.__subclasses__())

    def get_ext_params_class(self, resource_type, ext_params):
        """
        获取所有扩展参数的子类
        :return:
        """
        if not self._ext_parameters:
            for sub_class in BaseExtParam.__subclasses__():
                support_values = sub_class.support_values()
                if not support_values:
                    continue
                for value in support_values:
                    self._ext_parameters[value] = sub_class
        ext_class = self._ext_parameters.get(resource_type)
        if ext_class:
            return ext_class(**ext_params)
        return CommonResourceExtParam(**ext_params)
