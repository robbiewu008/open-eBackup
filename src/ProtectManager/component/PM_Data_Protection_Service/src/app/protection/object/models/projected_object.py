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
from sqlalchemy import Column, String, Boolean, Text, Integer, ForeignKey, DateTime, JSON
from sqlalchemy.orm import relationship

from app.base.db_base import Base
from app.common.type_decorator.time_type_decorator import TZDateTime


class ProtectedObject(Base):
    __tablename__ = 'protected_object'

    uuid = Column(String(64), primary_key=True)
    sla_id = Column(String(64))
    sla_name = Column(String(256))
    name = Column(String(512))
    env_id = Column(String(64))
    env_type = Column(String(16))
    resource_id = Column(String(64), nullable=False)
    resource_group_id = Column(String(64))
    type = Column(String(64), nullable=False)
    sub_type = Column(String(64))
    sla_compliance = Column(Boolean)
    path = Column(Text)
    ext_parameters = Column(JSON)
    status = Column(Integer, default=0)
    latest_time = Column(TZDateTime)
    earliest_time = Column(TZDateTime)
    chain_id = Column(String(64), nullable=False)
    consistent_status = Column(String(64))
    consistent_results = Column(String(2048))
    task_list = relationship('ProtectedTask', lazy="joined", cascade='all, delete-orphan', passive_deletes=True)

    def as_dict(self):
        protected_object_dict = {col.name: getattr(self, col.name)
                for col in self.__table__.columns}
        return protected_object_dict


class ProtectedTask(Base):
    __tablename__ = 'protected_task'

    uuid = Column(String(64), primary_key=True)
    protected_object_id = Column(String(64), ForeignKey(ProtectedObject.uuid), nullable=False)
    policy_id = Column(String(64), nullable=False)
    schedule_id = Column(String(64), nullable=False)
