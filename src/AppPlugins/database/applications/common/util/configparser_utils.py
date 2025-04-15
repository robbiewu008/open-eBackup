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

import configparser
import os

from common.logger import Logger

LOGGER = Logger().get_logger(filename="db2.log")


def read_config_section(config_file, section):
    if not os.path.exists(config_file):
        raise FileExistsError(f'The configuration file {config_file} does not exist.')
    config = configparser.ConfigParser()
    config.read(config_file, encoding='utf-8')
    if config.has_section(section):
        return dict(config.items(section))
    raise Exception(f"The configuration file does not have section {section}")
