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
import sys
from enum import Enum
from typing import List, Optional

from pydantic import Field, BaseModel

from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import IllegalParamException


class Parameter(BaseModel):
    key: str = Field(description="Key")
    value: str = Field(description="Key对应的参数值")


class TargetDetail(BaseModel):
    src_id: List[str] = Field(description="源ID", max_items=sys.maxsize)
    target_id: str = Field(description="目标ID", max_length=64)
    target_type: str = Field(description="目标类型", max_length=2048)


class RestoreTarget(BaseModel):
    env_id: str = Field(description="恢复环境ID", max_length=64)
    env_type: str = Field(description="恢复环境类型", max_length=1024)
    restore_target: str = Field(None, description="恢复目标， 文件集恢复该字段为恢复目标路径，虚拟机恢复该字段为目标ESX/Cluster",
                                max_length=10240)
    details: List[TargetDetail] = Field(description="恢复目标详情， 文件集恢复该字段为空。", max_items=sys.maxsize)


class RestoreLocation(str, Enum):
    origin = "O"
    new = "N"


class RestoreType(str, Enum):
    CR = "CR"
    IR = "IR"
    MR = "MR"
    FLR = "FLR"


class Filter(BaseModel):
    type: int = Field(ge=1, le=4, description="过滤类型")
    model: int = Field(ge=1, le=2, description="过滤模式")
    content: str = Field(description="过滤内容", max_length=10240)


class Source(BaseModel):
    source_name: str = Field(max_length=2048, description="数据源名称")
    source_location: str = Field(max_length=10240, description="数据源位置")


class DownloadRequestSchema(BaseModel):
    paths: List[str] = Field(description="文件信息，例如：/root", max_items=sys.maxsize)
    record_id: str = Field(alias="recordId", description="导出文件记录id", max_length=64)
    user_id: str = Field(alias="userId", description="用户id", max_length=64)
    copy_id: str = Field(alias="copyId", max_length=64, description="副本id")


def check_paths(download_req: DownloadRequestSchema):
    paths = download_req.paths
    illegal_param_error = IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["paths"])
    if len(paths) <= 0:
        raise illegal_param_error
    for path in paths:
        legal_flag = (len(path) > 0) and (path[0:1] == "/") and (".." not in path)
        if not legal_flag:
            raise illegal_param_error


class DownloadResponseSchema(BaseModel):
    request_id: str = Field(description="请求id")


class RestoreRequestStringSchema(BaseModel):
    user_id: str = Field(description="用户ID", min_length=1, max_length=64)
    restore_req_string: str = Field(description="任务信息中data中保存的前端下发的恢复参数信息", min_length=1, max_length=4096)


class RestoreRequestSchema(BaseModel):
    copy_id: str = Field(description="副本ID", max_length=64)
    object_type: str = Field(description="对象类型", max_length=2048)
    source: Source = Field(description="数据源信息")
    restore_location: Optional[RestoreLocation] = Field(description="恢复位置， 取值如下：O--原位置；N-新位置")
    restore_type: Optional[RestoreType] = Field(description="恢复类型，取值如下： CR--普通恢复；IR--即时恢复;MR--挂载恢复;FLR--Vmware的FLR")
    filters: List[Filter] = Field(description="过滤条件。", max_items=sys.maxsize)
    restore_objects: List[str] = Field(description="恢复的对象列表，文件集恢复可以指定多个文件或目录，不指定则全部恢复。", max_items=sys.maxsize)
    target: RestoreTarget = Field(description="恢复目标")
    ext_parameters: dict = Field(description="恢复参数")

    class Config:
        extra = 'allow'
        arbitrary_types_allowed = True
        schema_extra = {
            'example': {
                'request_id': '266ea41d-adf5-480b-af50-15b940c2b846',
                'copy_id': 'gns://069d6e86d23511eaa0e45ce883e5baf6/1596717691120012',
                'source': {
                    'source_name': 'str',
                    'source_location': 'str'
                },
                'object_type': 'File',
                'restore_location': 'O',
                'restore_type': 'CR',
                'restore_objects': [
                    'gns://069d6e86d23511eaa0e45ce883e5baf6/1596717691120012/boot/grub',
                    'gns://069d6e86d23511eaa0e45ce883e5baf6/1596717691120012/boot/efi'
                ],
                'restore_target': {
                    'env_id': '266ea41d-adf5-480b-af50-15b940c2b846',
                    'env_type': 'Host',
                    'restore_target': '/tmp',
                    'details': [{
                        'src_id': 'gns://069d6e86d23511eaa0e45ce883e5baf6/1596717691120012/boot/ef',
                        'target_id': '/tmp',
                        'target_type': 'File'
                    }
                    ]
                },
                'filters': [{
                    'filter_type': 1,
                    'filter_mode': 1,
                    'content': ''

                }],
                'ext_parameters': {
                    'FILE_REPLACE_STRATEGY': 'replace/ignore/replace_old_file',
                    'BEFORE_RESTORE_SCRIPT': 'before_restore_script.sh',
                    'AFTER_RESTORE_SCRIPT': 'after_restore_script.sh',
                    'RESTORE_FAILED_SCRIPT': 'restore_failed_script.sh',
                    'RESTORE_TO_SINGLE_DIRECTORY': 'true',
                    'CHANNELS': 100,  # 并行管道数
                    'IS_SINGLE_RESTORE': 0,  # 文件集是否单目录恢复：0否，1是
                }
            }
        }
