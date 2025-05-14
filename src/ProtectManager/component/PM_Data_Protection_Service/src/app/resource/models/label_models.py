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
from sqlalchemy import Column, String, BOOLEAN, BigInteger, ForeignKey
from sqlalchemy.orm import relationship

from app.base.db_base import Base


class LabelTable(Base):
    __tablename__ = 't_label'
    __table_args__ = {'extend_existing': True}

    uuid = Column(String(64), primary_key=True)
    name = Column(String(256))
    is_built = Column(BOOLEAN)
    aspect = Column(String(32))
    label_key = Column(String(64))
    language_type = Column(String(32))
    create_time = Column(BigInteger)
    builder_name = Column(String(64))
    task_list = relationship('LabelResourceTable', lazy="joined", cascade='all, delete-orphan', passive_deletes=True)

    __mapper_args__ = {
        'primary_key': [uuid]
    }

    def as_dict(self):
        temp = {col.name: getattr(self, col.name) for col in self.__table__.columns}
        return temp


class LabelResourceTable(Base):
    __tablename__ = 't_label_r_resource_object'
    __table_args__ = {'extend_existing': True}

    uuid = Column(String(64), primary_key=True)
    label_id = Column(String(64), ForeignKey(LabelTable.uuid), nullable=False)
    resource_object_id = Column(String(64))
    type = Column(String(64))

    def as_dict(self):
        temp = {col.name: getattr(self, col.name) for col in self.__table__.columns}
        return temp
