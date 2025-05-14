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
import uuid
from typing import Union, Optional
from datetime import datetime
from pydantic import BaseModel, constr
from app.common.event_messages.Common import livemount_vm
from app.common.event_messages.Common.enumerations import SchedulingType, SchedulerAction


class SchedulerCreateScheduleIn(BaseModel):
    schedule_type: SchedulingType = None
    policy_id: uuid.UUID = None
    action: SchedulerAction = None
    object: Union[
        livemount_vm.AddLivemountVMData, livemount_vm.RemoveLivemountVMData,
    ] = None
    params: dict = None
    interval: constr(regex=r'(?i)\d+[wdhm]') = None
    start_date: Optional[Union[datetime, None]] = None
    end_date: Optional[Union[datetime, None]] = None
    day_of_week: Optional[constr(regex=r'\b([0-6])\b')] = None


class SchedulerCreateScheduleOut(BaseModel):
    schedule_id: str
