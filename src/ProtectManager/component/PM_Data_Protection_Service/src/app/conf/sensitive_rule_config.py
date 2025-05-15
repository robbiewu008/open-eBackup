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
import configparser
import os

from typing import List


class Config:
    def __init__(self):
        ini_config = configparser.ConfigParser()
        ini_config.sections()
        ini_config.read(os.path.join(os.path.abspath(os.path.dirname(__file__)), 'sensitive-rule.ini'))
        self._config = ini_config

    def get_rule_keys(self, rule: str) -> List[str]:
        keys = self._config[rule].get("keys")
        return [key.lower() for key in keys.split(",")]

    def filter_enable(self):
        return self._config["switcher"].getboolean("enable")


config = Config()
