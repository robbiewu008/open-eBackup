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

from pydantic import BaseModel, Field


class TidbBkpResultInfo(BaseModel):
    copy_id: str = Field(None, description='copy_id')
    job_id: str = Field(None, description='job_id')
    cluster_name: str = Field(None, description='cluster_name')
    db: str = Field(None, description='db')
    tables: list = Field(default=None, description='tables')


class SqliteTable(BaseModel):
    uuid: str = Field(None, description='UUID')
    name: str = Field(None, description='name')
    type: str = Field(None, description='type')
    parent_path: str = Field(default='/', description='父节点路径')
    parent_uuid: str = Field(None, description='PARENT_UUID')
    size: str = Field(None, description='大小')
    create_time: str = Field(None, description='创建时间')
    modify_time: str = Field(None, description='修改时间')
    extend_info: str = Field(None, description='扩展信息')
    res_type: str = Field(None, description='RES_TYPE')
    res_sub_type: str = Field(None, description='RES_SUB_TYPE')
