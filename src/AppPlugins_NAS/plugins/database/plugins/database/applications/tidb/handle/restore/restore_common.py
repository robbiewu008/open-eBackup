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
import stat
import pymysql

from tdsql.logger import log


def write_progress_file(message: str, file_name: str):
    log.info(f'Write message into progress file: {message}')
    flags = os.O_WRONLY | os.O_CREAT
    modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
    with os.fdopen(os.open(file_name, flags, modes), 'a+') as out_file:
        out_file.write(message)
        out_file.write('\n')


def write_file_append(message: str, file_name: str):
    log.info(f'Write : {message} to file: {file_name}')
    flags = os.O_WRONLY | os.O_CREAT
    modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
    with os.fdopen(os.open(file_name, flags, modes), 'a+') as out_file:
        out_file.write(message)
        out_file.write('\n')
