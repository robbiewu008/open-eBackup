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
from enum import Enum


class FunctionEnum(Enum):
    # -------------------- Basic Function ---------------------------
    # 备份
    BACKUP = 'BACKUP'
    # 恢复
    RECOVERY = 'RECOVERY'
    # 即时恢复
    INSTANT_RECOVERY = 'INSTANT_RECOVERY'
    # 细粒度恢复
    FINE_GRAINED_RECOVERY = 'FINE_GRAINED_RECOVERY'
    # 跨域复制
    CROSS_DOMAIN_REPLICATION = 'CROSS_DOMAIN_REPLICATION'
    # 副本过期删除
    EXPIRE_COPY_DELETE = 'EXPIRE_COPY_DELETE'
    # LiveMount
    LIVE_MOUNT = 'LIVE_MOUNT'
    # 归档流程
    ARCHIVE = 'ARCHIVE'
    # 复制
    REPLICATION = 'REPLICATION'
    # 副本
    COPY = 'COPY'
    # -------------------- Advance Function ---------------------------
    # 云数据保护：云容灾
    CLOUD_DATA_PROTECT_CLOUD_DISASTER_TOLERANCE = 'CLOUD_DATA_PROTECT_CLOUD_DISASTER_TOLERANCE'
    # 云数据保护：公有云上数据保护
    CLOUD_DATA_PROTECT_DATA_PROTECTION_ON_PUBLIC_CLOUD = 'CLOUD_DATA_PROTECT_DATA_PROTECTION_ON_PUBLIC_CLOUD'
    # 数据管理：全局检索
    DATA_MANANGE_GLOBALE_SEARCH = 'DATA_MANANGE_GLOBALE_SEARCH'
    # 数据管理：非结构化数据管理
    DATA_MANAGE_UNSTRUCTURED_DATA_MANAGEMENT = 'DATA_MANAGE_UNSTRUCTURED_DATA_MANAGEMENT'
    # 数据保护：数据脱敏
    DATA_PROTECT_DATA_DESENSITIZATION = 'DATA_PROTECT_DATA_DESENSITIZATION'
    # 数据保护：防恶意软件
    DATA_PROTECT_ANTI_MALWARE = 'DATA_PROTECT_ANTI_MALWARE'
    # 数据保护：个人数据法规遵从
    DATA_PROTECT_PERSONAL_DATA_COMPLIANCE = 'DATA_PROTECT_PERSONAL_DATA_COMPLIANCE'
    # 多集群管理
    MULTI_CLUSTER_MANAGEMENT = 'MULTI_CLUSTER_MANAGEMENT'
