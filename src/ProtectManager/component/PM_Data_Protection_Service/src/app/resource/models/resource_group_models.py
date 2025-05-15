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
from datetime import datetime

from sqlalchemy import Column, String, Text, DateTime, Integer

from app.base.db_base import Base


class ResourceGroup(Base):
    __tablename__ = 't_resource_group'

    uuid = Column(String(64), primary_key=True)
    name = Column(String(256), nullable=False)
    path = Column(String(1024))
    source_type = Column(String(64))
    source_sub_type = Column(String(128))
    created_time = Column(DateTime, nullable=False, default=datetime.now)
    extend_str = Column(Text)
    user_id = Column(String(256))
    protection_status = Column(Integer, default=0, comment="资源保护状态, 0-未保护，1-已保护,2-创建中")
    scope_resource_id = Column(String(64), nullable=False, default='')
    group_type = Column(String(64), nullable=False, default='manual',
                        comment="虚拟机组类型, manual-静态组，rule-动态组")

    def as_dict(self):
        return {col.name: getattr(self, col.name) for col in self.__table__.columns}


class ResourceGroupMember(Base):
    __tablename__ = 't_resource_group_member'

    uuid = Column(String(64), primary_key=True)
    resource_group_id = Column(String(64), nullable=False)
    source_id = Column(String(64))
    source_sub_type = Column(String(128))

    def as_dict(self):
        return {col.name: getattr(self, col.name) for col in self.__table__.columns}
