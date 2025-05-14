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
import os
import ssl

import uvicorn
from app.common import logger
from app.common.config import settings
from app.common.constants.constant import SecurityConstants
from app.common.security.kmc_util import Kmc
from app.common.util import file_utils

from app import consumer as protect_consumer
from app.routers import api
from app import monitor

log = logger.get_logger(__name__)

protect_consumer.consumer_start()
monitor.start_memory_monitor(interval=120)

DPS_SERVICE_PORT = os.getenv("DPS_SERVICE_PORT")
if DPS_SERVICE_PORT is None:
    DPS_SERVICE_PORT = settings.SERVICE_PORT

APPLICATION_NAME = "PM_Data_Protection_Service" if settings.APPLICATION_NAME == "" else settings.APPLICATION_NAME
logging_config = logger.LoggerConfig(APPLICATION_NAME, settings.LOGGING_PATH)
ssl_config = {
    "ssl_keyfile": SecurityConstants.INTERNAL_KEY_FILE,
    "ssl_certfile": SecurityConstants.INTERNAL_CERT_FILE,
    "ssl_cert_reqs": ssl.CERT_REQUIRED,
    "ssl_ca_certs": SecurityConstants.INTERNAL_CA_FILE,
    "ssl_ciphers": SecurityConstants.SSL_CIPHERS
}
uvicorn.run(api, host=settings.SERVICE_HOST, port=int(DPS_SERVICE_PORT), log_config=logging_config.get_config(),
            ssl_keyfile_password=Kmc().decrypt(
                file_utils.read_file_by_utf_8(SecurityConstants.INTERNAL_KEYFILE_PWD_FILE)), **ssl_config)
