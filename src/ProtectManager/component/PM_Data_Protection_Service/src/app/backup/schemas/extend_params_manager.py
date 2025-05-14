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

from app.backup.schemas.base_ext_param import BaseExtendParam
from app.backup.schemas.extends.common_backup_ext_param import CommonExtendParam


class ExtendParamsManager(object):

    def __init__(self):
        self._extends_dir: str = "extends"
        self._package: str = "app.backup.schemas.extends"
        self._load_classes()
        self._ext_parameters: dict = {}

    def _load_classes(self):
        extend_path = os.path.join(os.path.abspath(
            os.path.dirname(__file__)), self._extends_dir)
        py_files = [py_file for py_file in glob.glob(
            "{}/*.pyc".format(extend_path).rstrip("/"))]

        for py_file in py_files:
            if py_file.endswith("__init__.pyc"):
                continue
            script_module_name = "{}.{}".format(
                self._package, os.path.basename(py_file)[:-len(".pyc")])
            spec = importlib.util.spec_from_file_location(
                script_module_name, py_file)

            script_module = importlib.util.module_from_spec(spec)
            if not sys.modules.get(script_module_name):
                sys.modules[script_module_name] = script_module
                spec.loader.exec_module(script_module)

    def get_all_extend_params_classes(self):
        """
        获取所有扩展参数的子类
        :return:
        """
        return tuple(BaseExtendParam.__subclasses__())

    def get_extend_param(self, application, policy_type, ext_params, action):
        """
        获取对应的子类信息
        :return:
        """
        for sub_class in BaseExtendParam.__subclasses__():
            if sub_class.is_support(application, policy_type, action):
                return sub_class(**ext_params)
        return CommonExtendParam(**ext_params)
