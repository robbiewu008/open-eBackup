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
from typing import List

from pydantic import Field
from pydantic.main import BaseModel


class FilterRule(str, Enum):
    # 全匹配
    ALL = "ALL"
    # 前缀匹配
    START_WITH = "START_WITH"
    # 后缀匹配
    END_WITH = "END_WITH"
    # 模糊匹配
    FUZZY = "FUZZY"


class FilterMode(str, Enum):
    EXCLUDE = "EXCLUDE"
    INCLUDE = "INCLUDE"


class FilterType(str, Enum):
    DISK = "DISK"
    VM = "VM"
    VOLUME = "VOLUME"


class DiskType(str, Enum):
    IDE = "IDE"
    SATA = "SATA"
    SCSI = "SCSI"
    NVME = "NVME"


class FilterColumn(str, Enum):
    NAME = "NAME"
    ID = "ID"
    SLOT = "SLOT"
    TAG = "TAG"


class SlaApplyType(str, Enum):
    APPLY_TO_ALL = "APPLY_TO_ALL"
    APPLY_TO_NEW = "APPLY_TO_NEW"


class ResourceFilter(BaseModel):
    filter_by: FilterColumn = Field(..., description="过滤条件，根据什么字段过滤。如ID、name等")
    type: FilterType = Field(..., description="过滤的类型：如磁盘、虚拟机等")
    rule: FilterRule = Field(description="过滤规则：如模糊匹配、全匹配等")
    mode: FilterMode = Field(description="过滤类型：INCLUDE、EXCLUDE")
    values: List[str] = Field(description="具体过滤的值")


class VmwareAgentHostInfo(BaseModel):
    proxy_id: str = Field(None, description="代理主机id")
    host: str = Field(None, description="主机ip")
    name: str = Field(None, description="主机名称")
    port: str = Field(None, description="主机端口")


class ProtectPostAction(str, Enum):
    """
    保护后置操作
    """
    BACKUP = "BACKUP"


class ProxyHostMode(int, Enum):
    # 选择代理主机模式：0-自动，1-手动
    AUTO = 0
    MANUAL = 1


class NasProtocolType(int, Enum):
    # 选择协议：0-CIFS，1-NFS, 3-NDMP
    CIFS = 0
    NFS = 1
    NFS_CIFS = 2
    NDMP = 3
    NONE_SHARE = -1


class SmallFileAggregation(int, Enum):
    # 小文件聚合模式：1-不聚合，2-聚合
    NATIVE = 1
    AGGREGATE = 2


class PermissionsAndAttributesEnum(int, Enum):
    # 权限与属性：0-文件夹，1-文件和文件夹
    FOLDER = 0
    FILE_AND_FOLDER = 1


class GaussDBTableTypeEnum(int, Enum):
    ROACH = 0
    GDS = 1


class HypervFormatEnum(str, Enum):
    VHD = "VHD"
    VHDX = "VHDX"
    VHDSet = "VHDSet"


class StorageInfoEnum(str, Enum):
    STORAGE_TYPE = "storage_type"
    STORAGE_UNIT = "storage_unit"
    STORAGE_INFO = "storage_info"
    STORAGE_UNIT_GROUP = "storage_unit_group"
    STORAGE_ID = "storage_id"
    BASIC_DISK = "BasicDisk"

