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
from pydantic import Field
from pydantic.main import BaseModel


class JobSchema(BaseModel):
    job_id: str = Field(None, alias='jobId')
    user_id: str = Field(None, alias='userId')
    type: str = Field(None)
    progress: int = Field(None)
    start_time: int = Field(None, alias='startTime')
    end_time: int = Field(None, alias='endTime')
    last_update_time: int = Field(None, alias='lastUpdateTime')
    status: str = Field(None)
    parent_id: str = Field(None, alias='parentId')
    speed: str = Field(None)
    detail: str = Field(None)
    detail_para: str = Field(None, alias='detailPara')
    extend_str: str = Field(None, alias='extendStr')
    associative_id: str = Field(None, alias='associativeId')
    source_id: str = Field(None, alias='sourceId')
    source_name: str = Field(None, alias='sourceName')
    source_sub_type: str = Field(None, alias='sourceSubType')
    source_location: str = Field(None, alias='sourceLocation')
    target_name: str = Field(None, alias='targetName')
    target_location: str = Field(None, alias='targetLocation')
    copy_id: str = Field(None, alias='copyId')
    copy_time: int = Field(None, alias='copyTime')
    enable_stop: bool = Field(None, alias='enableStop')
    system_job: bool = Field(None, alias='isSystem')
    request_id: str = Field(None, alias='requestId')
    visible: bool = Field(None, alias='isVisible')
    message: str = Field(None)
    data: str = Field(None)

    class Config:
        allow_population_by_field_name = True
