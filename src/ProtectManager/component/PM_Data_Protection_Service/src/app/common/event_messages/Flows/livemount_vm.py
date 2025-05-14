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
from app.common.event_messages.event import EventMessage
from app.common.event_messages.Common import livemount_vm


class AddLivemountVMRequest(EventMessage):
    add_livemount_vm_data: livemount_vm.AddLivemountVMData
    default_publish_topic: str = 'AddLivemountVMRequest'


class RemoveLivemountVMRequest(EventMessage):
    remove_livemount_vm_data: livemount_vm.RemoveLivemountVMData
    default_publish_topic: str = 'RemoveLivemountVMRequest'
