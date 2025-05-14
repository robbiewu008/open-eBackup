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
from app.common.clients.client_util import SystemBaseHttpsClient, parse_response_data
from app.common.exter_attack import exter_attack

LOGGER = logger.get_logger(__name__)


class ArchiveClient(object):
    def query_storage_info(self, storage_id) -> any:
        storage_info = self._query_storage_info(storage_id)
        if not storage_info:
            storage_info = self._query_media_set_info(storage_id)
        return storage_info

    def _query_storage_info(self, storage_id) -> any:
        '''
        返回结构体
        {
  "storageName": "storageName_ecaa31ee8236",
  "endpoint": "endpoint_9e40d52fb9f0",
  "certName": "certName_18c89c73b4ac",
  "certId": "certId_aeae664e76f6",
  "type": 0,
  "cloudType": 0,
  "port": 0,
  "connectType": 0,
  "ak": "ak_9869e54a23b5",
  "bucketName": "bucketName_8f8c021f942a",
  "indexBucketName": "indexBucketName_53a577fa3a8c",
  "proxyEnable": false,
  "proxyHostName": "proxyHostName_f3a5d1d2ec2b",
  "proxyUserName": "proxyUserName_669cfa8a8030",
  "useHttps": false,
  "alarmEnable": false,
  "alarmThreshold": 0,
  "noSensitiveSk": "noSensitiveSk_e7c67386df73",
  "alarmLimitValueUnit": "alarmLimitValueUnit_7a8e1e3f3f84",
  "status": 0,
  "totalSize": 0.00,
  "usedSize": 0.00,
  "freeSize": 0.00
}
        '''
        url = f'/v1/internal/storages/{storage_id}'
        return self._query_info(url)

    def _query_media_set_info(self, media_set_id):
        '''
        返回结构体
        {
  "mediaSetId": "mediaSetId_ea6488bd0a6b",
  "mediaSetName": "mediaSetName_19f811bc015a",
  "type": "UNKNOWN",
  "appTypes": [
    "ORACLE"
  ],
  "node": "node_9760222813cb",
  "alarmEnable": false,
  "alarmThreshold": 0,
  "retentionType": "IMMEDIATE",
  "retentionDuration": 0,
  "retentionUnit": "INVALID",
  "tapes": [
    {
      "tapeUUID": "tapeUUID_7fa36a08af64",
      "tapeLabel": "tapeLabel_5538a53a8317",
      "mediaSetId": "mediaSetId_8f241048bc3c",
      "mediaSetName": "mediaSetName_dc7e244c25dd",
      "tapeLibrarySn": "tapeLibrarySn_38d8078a7292",
      "writeStatus": "UNKNOWN",
      "usedCapacity": 0,
      "totalCapacity": 0,
      "location": "location_dc562f2d9435",
      "worm": "UNKNOWN",
      "lastWriteTime": "lastWriteTime_1ed4122e6dd0",
      "status": "NOT_IN_LIBRARY",
      "lockKey": "lockKey_f4971b277e5f",
      "commitedWritePos": 0,
      "isDepended": false,
      "extend": "extend_436c7d74e6be"
    }
  ],
  "availableTapeCount": 0,
  "totalTapeCount": 0,
  "blockSize": 0,
  "compressionStatus": "DISABLE",
  "esn": "esn_ca3db51d9fc6"
}
        '''
        url = f'/v1/internal/tape-library/media-sets/{media_set_id}'
        return self._query_info(url)

    @staticmethod
    @exter_attack
    def _query_info(url):
        response = SystemBaseHttpsClient().request("GET", url)
        if response.status != HTTPStatus.OK:
            LOGGER.error(
                f"invoke api query({url}) information from error, response is response_status:{response.status}")
            return {}
        return parse_response_data(response.data)
