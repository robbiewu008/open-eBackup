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
SCRIPT_PATH_MAX_LENGTH = 8192
FILESET_NAME_MAX_LENGTH = 64
HOST_ONLINE = "1"
HOST_OFFLINE = "0"
BYTE_SIZE_CONST = 1024
# 文件集模板数目上限
FILESET_TEMPLATE_MAX_NUM = 128
# 单个模板文件路径数目上限
SINGLE_TEMPLATE_FILE_PATH_MAX_NUM = 64
# Windows绝对路径最大字符数，260个字符排除末尾的NUL('\0')
DEFAULT_WINDOWS_PATH_MAX = 259
# 非Windows绝对路径最大字符数
DEFAULT_NON_WINDOWS_PATH_MAX = 4096
# 文件集模板关联的最大文件集数量
TEMPLATE_ASSOCIATE_MAX_FILESETS = 2000
NOT_VOLUME_PROTECT_OS_LIST = ["unix", "aix", "hp_ux", "sunos", "solaris"]
DATABASES_CONST = "DATABASES"
WORKING_STATUS_LIST = ["RUNNING", "READY", "PENDING", "ABORTING"]
CAN_NOT_REMOVE_PROTECT_TYPES = ["BACKUP", "GROUP_BACKUP", "copy_replication", "archive"]
CERT_DIR = "/opt/OceanProtect/protectmanager/cert/internal/OpenAPI/OpenAPI.crt.pem"
KEY_DIR = "/opt/OceanProtect/protectmanager/cert/internal/OpenAPI/OpenAPI.pem"
CNF_DIR = "/opt/OceanProtect/protectmanager/cert/internal/OpenAPI/OpenAPI.cnf"
CA_DIR = "/opt/OceanProtect/protectmanager/cert/internal/ProtectAgent/ca/ca.crt.pem"
CRL_DIR = "/opt/OceanProtect/protectmanager/cert/crl/ProtectAgent.crl"
