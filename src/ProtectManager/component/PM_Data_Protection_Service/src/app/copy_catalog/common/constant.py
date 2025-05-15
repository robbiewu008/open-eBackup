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
from app.common.enums.anti_ransomware_enum import AntiRansomwareEnum
from app.common.enums.copy_enum import GenerationType
from app.common.enums.resource_enum import ResourceSubTypeEnum


class AntiRansomwareCopyConstant:
    # 支持防勒索检测的副本的生成方式列表
    SUPPORT_ANTI_COPY_GENERATED_BY_LIST = [
        GenerationType.BY_BACKUP.value,
        GenerationType.BY_REPLICATED.value,
        GenerationType.BY_CASCADED_REPLICATION.value,
        GenerationType.BY_REVERSE_REPLICATION.value
    ]
    # 检测中/已检测完成的检测状态列表
    DETECTED_STATUS_LIST = [
        AntiRansomwareEnum.DETECTING.value,
        AntiRansomwareEnum.UNINFECTING.value,
        AntiRansomwareEnum.INFECTING.value,
        AntiRansomwareEnum.ERROR.value
    ]
    # 复制副本的生成方式的列表
    COPY_GENERATED_BY_REPLICATED_LIST = [
        GenerationType.BY_REPLICATED.value,
        GenerationType.BY_CASCADED_REPLICATION.value,
        GenerationType.BY_REVERSE_REPLICATION.value
    ]
    # 支持防勒索检测的资源类型
    SUPPORT_ANTI_RESOURCE_TYPE_LIST = [
        ResourceSubTypeEnum.FusionCompute.value, ResourceSubTypeEnum.NasFileSystem.value,
        ResourceSubTypeEnum.NasShare.value, ResourceSubTypeEnum.CNWARE_VM.value,
        ResourceSubTypeEnum.NUTANIX_VM.value, ResourceSubTypeEnum.HYPER_V_VM.value,
        ResourceSubTypeEnum.Fileset.value, ResourceSubTypeEnum.VirtualMachine.value,
        ResourceSubTypeEnum.HCSCloudHost.value, ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.value,
        ResourceSubTypeEnum.FUSION_ONE_COMPUTE.value
    ]


class RedisKey:
    COPY_CACHE_EXPIRE = "copy.cache.expire"


class JobLabel:
    EXPIRE_COPY_JOB_LABEL = "common_expire_copy_label"
    DELETE_COPY_JOB_LABEL = "common_delete_copy_label"


class ExtendRetentionConstant:
    FOREVER = -1
    ONE_DAY = 24 * 60 * 60


class CopyGeneratedType:
    COPY_GENERATED_BY_REPLICATED_LIST = [
        GenerationType.BY_REPLICATED.value,
        GenerationType.BY_CASCADED_REPLICATION.value,
        GenerationType.BY_REVERSE_REPLICATION.value
    ]
