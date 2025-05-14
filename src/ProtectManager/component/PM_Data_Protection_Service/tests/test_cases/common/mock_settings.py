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
from unittest.mock import Mock, patch

from urllib3 import HTTPResponse

from app.common.clients.client_util import InfrastructureHttpsClient, ProtectEngineEDmaHttpsClient

fake_db_data = {
    "data": [
        {"database.generalPassword": "generaldbpwd"},
        {"database.superPassword": "gaussdbpwd"},
        {"redis.username": "default"},
        {"redis.password": "redispwd"},
        {"kafka.username": "kafka_usr"},
        {"kafka.password": "kafkapwd"}
    ]
}
fake_timezone_data = {
    "data": [
        {"CMO_SYS_TIME_ZONE_NAME": "Asia/Shanghai"}
    ]
}
from app.common.security.kmc_util import Kmc
decrypt_file_patcher = patch.object(Kmc, "decrypt", Mock(return_value=None))
read_file_patcher = patch("app.common.util.file_utils.read_file_by_utf_8", Mock(return_value="encrypt_text"))
req_infra_patcher = patch.object(
    InfrastructureHttpsClient, "request",
    Mock(return_value=HTTPResponse(status=200, body=bytes(json.dumps(fake_db_data), encoding="utf-8"))))
req_dma_patcher = patch.object(
    ProtectEngineEDmaHttpsClient, "request",
    Mock(return_value=HTTPResponse(status=200, body=bytes(json.dumps(fake_timezone_data), encoding="utf-8"))))
read_file_patcher.start()
decrypt_file_patcher.start()
req_infra_patcher.start()
req_dma_patcher.start()
from app.common import config
fake_settings = config.settings
read_file_patcher.stop()
decrypt_file_patcher.stop()
req_infra_patcher.stop()
req_dma_patcher.stop()
