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
from typing import List
import uuid

from pydantic import BaseModel, Field

from app.common.event_messages.event import EventMessage


class PolicyBackupSettingChangedData(BaseModel):
    sla_name: str = Field(..., description='SLA name for which policy settings changed')
    sla_id: uuid.UUID = Field(..., description='SLA ID for which policy settings changed')
    policy_ids: List[uuid.UUID] = Field(..., description='List of Policy IDs that where changed for current SLA')


class PolicyBackupSettingsChanged(EventMessage):
    policy_backup_settings_changed_data: PolicyBackupSettingChangedData
    default_publish_topic: str = 'PolicyBackupSettingsChanged'


class ObjectData(BaseModel):
    object_id: uuid.UUID = Field(..., description='Host ID of the a host which was deleted from the system')


class ObjectDeletedEvent(EventMessage):
    data: ObjectData
    default_publish_topic: str = 'ObjectDeleted'
