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
from datetime import datetime
from zoneinfo import ZoneInfo

from sqlalchemy import (DateTime, TypeDecorator)

from app.common.constants.constant import CommonConstants
from app.common.util.time_util import get_local_timezone


class TZDateTime(TypeDecorator):
    impl = DateTime

    def process_result_value(self, value, dialect):
        if value is None:
            return None
        beijing_tz = ZoneInfo(CommonConstants.DEFAULT_TIMEZONE)
        bz_time = value.replace(tzinfo=beijing_tz)
        local_time = bz_time.astimezone(tz=ZoneInfo(get_local_timezone()))
        return local_time.replace(tzinfo=None)

    def process_bind_param(self, value, dialect):
        if value is None:
            return None
        # 根据时间查询副本时，不走这段逻辑，有自己的时区逻辑，直接return
        if isinstance(value, str):
            return value
        beijing_tz = ZoneInfo(CommonConstants.DEFAULT_TIMEZONE)
        database_time = value.astimezone(tz=beijing_tz)
        return database_time
