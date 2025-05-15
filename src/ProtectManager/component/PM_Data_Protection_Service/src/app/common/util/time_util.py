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
import datetime
import time
from zoneinfo import ZoneInfoNotFoundError

import pytz

from app.common.clients.device_manager_client import DEFAULT_TIMEZONE
from app.common.constants.constant import CommonConstants
from app.common.logger import get_logger
from app.common.util.provider.time_zone_provider import get_timezone_provider

log = get_logger(__name__)

local_time_zone = None
global_exception_time = None


def get_local_timezone():
    global local_time_zone
    global global_exception_time

    if local_time_zone:
        return local_time_zone

    # 如果获取时区出现过异常，则10s内不重复获取，防止接口重复大量调用
    if global_exception_time is not None and int(time.time()) - global_exception_time < 10:
        return CommonConstants.DEFAULT_TIMEZONE

    # 如果时区未初始化成功，则需要重新初始化查询
    try:
        local_time_zone = get_timezone_provider().get_timezone()
        # 没获取到，则先返回默认值
        if not local_time_zone:
            log.warning("No timezone found.")
            global_exception_time = int(time.time())
            return CommonConstants.DEFAULT_TIMEZONE
        log.info(f"Time zone of system is :{local_time_zone}")
    except ZoneInfoNotFoundError as ex:
        log.exception(f"No time zone found in zoneinfo: {ex}")
        # 检查时区信息是否能被python库识别到，若不能识别则使用默认时区
        local_time_zone = CommonConstants.DEFAULT_TIMEZONE
    except Exception as ex:
        log.exception(f"Get timezone occurs error: {ex}")
        # 出现异常时返回默认时区
        global_exception_time = int(time.time())
        return CommonConstants.DEFAULT_TIMEZONE

    global_exception_time = None
    return local_time_zone

