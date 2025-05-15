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
from abc import ABC, abstractmethod
from http import HTTPStatus
from zoneinfo import ZoneInfo

from app.common.clients.client_util import parse_response_data
from app.common.clients.client_util import OsaHttpsClient
from app.common.clients.system_base_client import SystemBaseClient
from app.common.config import settings
from app.common.constants.constant import CommonConstants
from app.common.deploy_type import DeployType
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.logger import get_logger
from app.replication.client.replication_client import ReplicationClient

log = get_logger(__name__)


class TimeZoneProvider(ABC):

    @abstractmethod
    def get_timezone(self) -> str:
        """获取当前时区字符串"""
        pass


class XSeriesTimeZoneProvider(TimeZoneProvider):
    def get_timezone(self) -> str:
        esn = settings.get_esn_from_sys_base()
        user_name = get_business_username()
        url = f'/v1/internal/deviceManager/rest/{esn}/system_timezone?username={user_name}'
        response = OsaHttpsClient().request("GET", url)
        if response.status == HTTPStatus.OK:
            res_data = parse_response_data(response.data).get('data', [])
            result = res_data[0].get("CMO_SYS_TIME_ZONE_NAME", '')
            # 检查时区信息是否能被python库识别到，若不能识别则使用默认时区
            ZoneInfo(result)
            return result if res_data else CommonConstants.DEFAULT_TIMEZONE
        else:
            log.error("Invoke get device manager timezone api failed.")
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       message=f"Invoke get device manager timezone api failed.")


class CyberEngineTimeZoneProvider(TimeZoneProvider):
    def get_timezone(self) -> str:
        time_zone = SystemBaseClient.get_time_zone()
        log.debug(f"time zone is {time_zone}")
        return time_zone if time_zone else CommonConstants.DEFAULT_TIMEZONE


class DefaultTimeZoneProvider(TimeZoneProvider):
    def get_timezone(self) -> str:
        return CommonConstants.DEFAULT_TIMEZONE


def get_business_username():
    if is_need_admin():
        return CommonConstants.ADMIN
    return CommonConstants.DATAPROTECT_ADMIN


def is_need_admin():
    return any([DeployType().is_cloud_backup_type(), DeployType().is_cyber_engine_deploy_type(),
                DeployType().is_hyper_detect_deploy_type(), ReplicationClient.is_hcs_service()])


def get_timezone_provider():
    if DeployType().is_ocean_protect_type():
        return XSeriesTimeZoneProvider()
    if DeployType().is_cyber_engine_deploy_type():
        return CyberEngineTimeZoneProvider()
    else:
        return DefaultTimeZoneProvider()
