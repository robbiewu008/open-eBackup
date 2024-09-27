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
import logging
import sys


def init_log():
    log = logging.getLogger(__name__)

    formatter = logging.Formatter('%(asctime)s %(levelname)-3s: [%(filename)s] [%(lineno)s] [%(funcName)s] %(message)s ')
    file_handler = logging.FileHandler('install.log')
    file_handler.setFormatter(formatter)

    console_handler = logging.StreamHandler(sys.stdout)
    console_handler.formatter = formatter

    log.setLevel(logging.DEBUG)

    file_handler.setLevel(logging.DEBUG)
    console_handler.setLevel(logging.DEBUG)

    log.addHandler(file_handler)
    log.addHandler(console_handler)

    return log


logger = init_log()
