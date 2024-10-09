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
from enum import Enum


class SecurityConstants:
    SSL_CIPHERS = "ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256"
    INTERNAL_CERT_DIR = "/opt/logpath/infrastructure/cert/internal"
    INTERNAL_CA_FILE = f"{INTERNAL_CERT_DIR}/ca/ca.crt.pem"
    INTERNAL_CERT_FILE = f"{INTERNAL_CERT_DIR}/internal.crt.pem"
    INTERNAL_KEY_FILE = f"{INTERNAL_CERT_DIR}/internal.pem"
    INTERNAL_KEYFILE_PWD_FILE = f"{INTERNAL_CERT_DIR}/internal_cert"

    # 依赖的KMC动态库
    LIBKMCV3_SO_PATH = "/usr/lib64/libkmcv3.so"
    MODULE_NAME = "datamoveengine"
    DEFAULT_DOMAIN_ID = 0
    KMC_MASTER_KS_PATH = "/opt/logpath/protectmanager/kmc/master.ks"
    KMC_BACKUP_KS_PATH = "/kmc_conf/backup.ks"