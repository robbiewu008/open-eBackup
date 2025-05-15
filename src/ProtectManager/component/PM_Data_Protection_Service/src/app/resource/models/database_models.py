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
from sqlalchemy import Column, String, ForeignKey, Integer, BOOLEAN, Text

from app.resource.models.resource_models import ResourceTable
from app.resource.schemas.database import DatabaseSchema, AsmInfoSchema, InternalDatabaseSchema
from app.base.db_base import Base


class DatabaseTable(ResourceTable):
    __tablename__ = 'databases'
    __table_args__ = {'extend_existing': True}
    uuid = Column(String(64), ForeignKey(ResourceTable.uuid, ondelete='CASCADE'), primary_key=True)
    link_status = Column(Integer)
    db_username = Column(String(255))
    db_password = Column(String(2048))
    verify_status = Column(BOOLEAN)
    auth_type = Column(Integer)
    db_role = Column(Integer)
    os_username = Column(String(256))
    inst_name = Column(String(64))
    is_asminst = Column(Integer)
    version = Column(String(64))
    valid = Column(BOOLEAN, default=True)
    asm = Column(String(512))
    asm_auth = Column(String(2048))
    is_cluster = Column(BOOLEAN, default=False)

    __mapper_args__ = {
        'polymorphic_identity': 'DATABASES',
    }

    __relation__ = {
        "filters": ["%inst_name%", "%name%", "!discriminator",
                    "!properties", "path%", "%authorized_user%"],
        "schema": DatabaseSchema
    }

    __internal_relation__ = {
            "filters": ["%inst_name%", "%name%", "!discriminator",
                        "!properties", "path%", "%authorized_user%"],
            "base": "__relation__",
            "schema": InternalDatabaseSchema
    }


class ClusterNodeTable(Base):
    __tablename__ = 'cluster_nodes'
    __table_args__ = {'extend_existing': True}

    host_uuid = Column(String(64), primary_key=True)
    cluster_name = Column(String(256))
    env_type = Column(String(64))
    type = Column(String(64))
    cluster_info = Column(Text)

    def as_dict(self):
        temp = {col.name: getattr(self, col.name) for col in self.__table__.columns}
        return temp


class AsmInfoTable(ResourceTable):
    __tablename__ = 'asminfo'
    __table_args__ = {'extend_existing': True}
    uuid = Column(String(64), ForeignKey(ResourceTable.uuid, ondelete='CASCADE'), primary_key=True)
    asm_instances = Column(String(512))
    username = Column(String(256))
    password = Column(String(2048))
    verify_status = Column(BOOLEAN, default=False)
    auth_type = Column(Integer)
    asm = Column(String(512))

    __mapper_args__ = {
        'polymorphic_identity': __tablename__,
    }
    __schema__ = AsmInfoSchema
