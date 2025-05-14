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
from sqlalchemy import Column, String, ForeignKey, Integer, Boolean, Text

from app.resource.models.resource_models import ResourceTable
from app.resource.schemas.virtual_resource_schemas import VirtualResourceSchema


class VirtualResourceTable(ResourceTable):
    __tablename__ = "virtual_resource"
    __discriminator__ = "VirtualResource"
    __table_args__ = {'extend_existing': True}
    uuid = Column(String(64), ForeignKey(ResourceTable.uuid, ondelete='CASCADE'), primary_key=True)

    # Only for the esx host managed by vCenter
    vm_ip = Column(String(4096))
    env_ip = Column(String(256))
    link_status = Column(Integer)
    # Only for a DataStore:
    capacity = Column(Integer)
    free_space = Column(String(32))
    uncommitted = Column(String(32))

    # The ManagedObject Id in vCenter or ESX
    mo_id = Column(String(32))

    # The number of sub resource
    children = Column(String(32))

    # 仅仅用于vm，操作系统类型：Linux,Windows,Other
    os_type = Column(String(64))
    # 用于vm，虚拟机标记，用逗号分隔的编码后的标记
    tags = Column(Text)

    is_template = Column(Boolean, default=False)
    # 资源在vsphere环境中的唯一标识，用于某些搜索
    instance_id = Column(String(64))

    alias_type = Column(String(64))
    alias_value = Column(String(64))
    # 虚拟机设置中引导选项中的固件信息，如"bios"
    firmware = Column(String(64))
    __filter_fields__ = [
        "%name%", "!discriminator",
        "!properties", "path%", "%authorized_user%", "%tags%"
    ]
    __mapper_args__ = {
        'polymorphic_identity': __discriminator__,
    }
    __relation__ = {
        "schema": VirtualResourceSchema,
        "orders": ["children"]
    }

    def as_dict(self):
        temp = {col.name: getattr(self, col.name)
                for col in self.__table__.columns}
        return temp
