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
from http import HTTPStatus

from app.common import logger
from app.common.clients.client_util import ProtectEngineEDmaHttpsClient, parse_response_data
from app.common.exter_attack import exter_attack

DEFAULT_TIMEZONE = "Asia/Shanghai"
RESPONSE_KEY_TIMEZONE = "CMO_SYS_TIME_ZONE_NAME"

LOGGER = logger.get_logger(__name__)


def replace_time_zone(time_zone):
    """
    将python不支持的时区替换成相邻的时区
    :param time_zone:
    :return:
    """
    if time_zone == 'Asia/Beijing':
        time_zone = 'Asia/Shanghai'
    elif time_zone == 'US/Pacific-New':
        time_zone = 'US/Pacific'
    return time_zone


class DeviceManagerClient:
    @staticmethod
    @exter_attack
    def init_time_zone():
        """
        调用dorado接口获取时区信息
        :return:
        """
        url = f'/deviceManager/rest/dorado/system_timezone'
        # 增加容错，调用失败或者通用服务器时，直接使用默认时区
        time_zone_name = DEFAULT_TIMEZONE
        LOGGER.info('invoke api to query timezone')
        try:
            response = ProtectEngineEDmaHttpsClient().request("GET", url)
            if response.status == HTTPStatus.OK:
                resp_data = parse_response_data(response.data)
                time_zone_name = resp_data.get("data")[0].get(RESPONSE_KEY_TIMEZONE)
        except Exception:
            LOGGER.exception(f'Failed to query query timezone')

        timezone = '/etc/timezone'
        time_zone_name = replace_time_zone(time_zone_name)
        try:
            with open(timezone, 'w') as out:
                out.write(time_zone_name)
        except Exception:
            LOGGER.exception("open time zone file failed")

        return time_zone_name


device_manager_client = DeviceManagerClient()
