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
from typing import Type
from pydantic import BaseModel

from app.common.event_messages.Eam import eam
from app.common.event_messages.Flows import livemount_vm
from app.common.event_messages.System.abort import AbortRequestEvent
from app.common.event_messages.event import EventMessage


class TopicInfo(BaseModel):
    event_cls: Type[EventMessage]
    partitions: int = 3
    replication_factor: int = 1


def topic_of(cls):
    return cls.__fields__['default_publish_topic'].default


TOPICS = {
    'AbortRequest': TopicInfo(event_cls=AbortRequestEvent),

    # Flows - /livemount vm
    'AddLivemountVMRequest': TopicInfo(event_cls=livemount_vm.AddLivemountVMRequest),
    'RemoveLivemountVMRequest': TopicInfo(event_cls=livemount_vm.RemoveLivemountVMRequest),

    # Eam - livemount vm
    'DeleteLivemountVMRequest': TopicInfo(event_cls=eam.RemoveLivemountVMRequest),
    'DeleteLivemountVMResponse': TopicInfo(event_cls=eam.RemoveLivemountVMResponse),
}
