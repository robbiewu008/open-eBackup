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
from sqlalchemy import Column, String, BOOLEAN, BigInteger

from app.base.db_base import Base


class ResourceSetResourceObjectTable(Base):
    __tablename__ = 't_resource_set_r_resource_object'
    __table_args__ = {'extend_existing': True}

    resource_set_id = Column(String(256))
    resource_object_id = Column(String(256))
    resource_group_id = Column(String(256))
    type = Column(String(128))
    scope_module = Column(String(256))
    create_time = Column(BigInteger)
    is_manual_add = Column(BOOLEAN)

    __mapper_args__ = {
        'primary_key': [resource_set_id, resource_object_id, type, scope_module]
    }

    def as_dict(self):
        temp = {col.name: getattr(self, col.name) for col in self.__table__.columns}
        return temp


class DomainResourceObjectTable(Base):
    __tablename__ = 't_domain_r_resource_object'
    __table_args__ = {'extend_existing': True}

    domain_id = Column(String(256))
    resource_object_id = Column(String(256))
    type = Column(String(128))

    __mapper_args__ = {
        'primary_key': [domain_id, resource_object_id, type]
    }

    def as_dict(self):
        temp = {col.name: getattr(self, col.name) for col in self.__table__.columns}
        return temp


class AuthTable(Base):
    __tablename__ = 't_auth'
    __table_args__ = {'extend_existing': True}

    uuid = Column(String(256), primary_key=True)
    name = Column(String(256))
    auth_operation = Column(String(256))
    scope_module = Column(String(256))

    def as_dict(self):
        temp = {col.name: getattr(self, col.name) for col in self.__table__.columns}
        return temp


class RoleAuthTable(Base):
    __tablename__ = 't_role_r_auth'
    __table_args__ = {'extend_existing': True}

    role_id = Column(String(256))
    role_auth_id = Column(String(256))

    __mapper_args__ = {
        'primary_key': [role_id, role_auth_id]
    }

    def as_dict(self):
        temp = {col.name: getattr(self, col.name) for col in self.__table__.columns}
        return temp


class DomainRoleTable(Base):
    __tablename__ = 't_domain_r_role'
    __table_args__ = {'extend_existing': True}

    role_id = Column(String(256))
    resource_set_id = Column(String(256))
    domain_id = Column(String(256))
    is_default = Column(BOOLEAN)

    __mapper_args__ = {
        'primary_key': [domain_id, role_id, resource_set_id]
    }

    def as_dict(self):
        temp = {col.name: getattr(self, col.name) for col in self.__table__.columns}
        return temp


class ResourceSetTable(Base):
    __tablename__ = 't_resource_set'
    __table_args__ = {'extend_existing': True}

    uuid = Column(String(256), primary_key=True)
    name = Column(String(256))
    is_default = Column(BOOLEAN)
    is_public = Column(BOOLEAN)
    description = Column(String(256))
    create_time = Column(BigInteger)

    def as_dict(self):
        temp = {col.name: getattr(self, col.name) for col in self.__table__.columns}
        return temp


class RoleTable(Base):
    __tablename__ = 't_role'
    __table_args__ = {'extend_existing': True}

    role_id = Column(String(256), primary_key=True)
    creator_id = Column(String(256))
    create_time = Column(BOOLEAN)
    role_name = Column(String(256))
    role_description = Column(String(256))
    is_default = Column(BOOLEAN)
    role_type = Column(String(256))

    def as_dict(self):
        temp = {col.name: getattr(self, col.name) for col in self.__table__.columns}
        return temp


class UserDomainTable(Base):
    __tablename__ = 't_user_r_domain'
    __table_args__ = {'extend_existing': True}

    user_id = Column(String(256))
    domain_id = Column(String(256))

    __mapper_args__ = {
        'primary_key': [user_id, domain_id]
    }

    def as_dict(self):
        temp = {col.name: getattr(self, col.name) for col in self.__table__.columns}
        return temp


class UserTable(Base):
    __tablename__ = 't_user'
    __table_args__ = {'extend_existing': True}

    user_id = Column(String(256))
    user_name = Column(String(256))

    __mapper_args__ = {
        'primary_key': [user_id]
    }

    def as_dict(self):
        temp = {col.name: getattr(self, col.name) for col in self.__table__.columns}
        return temp
