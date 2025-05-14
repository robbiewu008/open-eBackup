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
import json
import uuid
from http import HTTPStatus

from app.common import logger
from app.common.clients.client_util import SystemBaseHttpsClient, DataEnableEngineParserHttpsClient, parse_response_data
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.event_messages.event import EventBase
from app.common.events import producer
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import IllegalParamException
from app.common.redis_session import redis_session
from app.copy_catalog.service.import_copy_service import query_copy_info_by_copy_id
from app.restore.schema.restore import DownloadRequestSchema

__all__ = ("Download",)

log = logger.get_logger(__name__)


class Download:
    """
    下载vmware和副本文件(nas等)
    """
    def __init__(self, request: DownloadRequestSchema):
        """
        初始化
        :param request: 请求参数
        """
        self.request = request

    def download(self):
        """
        屏蔽差异，向外提供统一下载入口
        :return: request_id
        """
        copy = query_copy_info_by_copy_id(self.request.copy_id)
        resource_sub_type = copy.resource_sub_type or ResourceSubTypeEnum.VirtualMachine
        # 判断副本是否已经在使用中，如果在使用中，异常中断
        if self._check_copy_is_using():
            raise IllegalParamException(CommonErrorCodes.COPY_IS_BEING_USED, [])
        # 判断是否为vmware
        if resource_sub_type == ResourceSubTypeEnum.VirtualMachine:
            self._download_vm()
        else:
            self._download_copy_files()

    def _download_vm(self):
        """
        下载vmware副本文件，v1接口特殊处理
        :return: request_id
        """
        request_id = str(uuid.uuid4())
        redis_session.hset(request_id, "request_id", request_id)
        redis_session.hset(request_id, "copy_id", self.request.copy_id)
        redis_session.hset(request_id, "user_id", self.request.user_id)
        redis_session.hset(request_id, "paths", str(self.request.paths))
        redis_session.hset(request_id, "object_type", "vim.VirtualMachine")
        redis_session.hset(request_id, "restore_type", "download")
        redis_session.hset(request_id, "record_id", self.request.record_id)
        message = EventBase(request_id=request_id, default_publish_topic='protection.restore')
        producer.produce(message)

    def _download_copy_files(self):
        """"
        下载其他类型（nas）副本文件
        """
        url = f"/v2/internal/copies/{self.request.copy_id}/action/download"
        body = {"paths": self.request.paths, "recordId": self.request.record_id}
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(body))
        if response.status != HTTPStatus.OK:
            log.error(f"invoke api query storage information error, response is {response}")

    def _check_copy_is_using(self):
        """"
        判断副本文件是否已经被使用
        """
        url = f"/v1/internal/flr/{self.request.copy_id}/using"
        response = DataEnableEngineParserHttpsClient().request("GET", url)
        if response.status != HTTPStatus.OK:
            log.error(f"invoke api query copy is using error, response is {response}")
            return False
        else:
            resp_data = parse_response_data(response.data)
            return resp_data.get("is_copy_using")
