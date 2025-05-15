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
from typing import Type, List

from sqlalchemy import BOOLEAN, func
from sqlalchemy.orm import Session

from app.common.logger import get_logger
from app.copy_catalog.models.tables_and_sessions import CopyProtectionTable
from app.protection.object.schemas.protected_copy_object import SlaResourceQuantityRelationship

log = get_logger(__name__)


class CRUDCopyProtectedObject(object):

    def __init__(self, model: Type[CopyProtectionTable]):
        self.model = model

    def batch_delete(self, session: Session, obj_id_list: List):
        obj_list = session.query(self.model).filter(
            self.model.uuid.in_(obj_id_list)).all()
        for obj in obj_list:
            session.delete(obj)
        session.flush()

    def delete_by_condition(self, session: Session, conditions: list):
        obj_list = session.query(self.model).filter(*conditions).all()
        for obj in obj_list:
            session.delete(obj)
        session.flush()

    def query_by_resource_ids(self, session: Session, resource_ids: List[str]) -> List[CopyProtectionTable]:
        return session.query(self.model).filter(CopyProtectionTable.protected_resource_id.in_(resource_ids)).all()

    def query_one_by_resource_id(self, session: Session, resource_id: str) -> CopyProtectionTable:
        return session.query(self.model).filter(CopyProtectionTable.protected_resource_id == resource_id).first()

    def query_obj_by_sla_id(self, session: Session, sla_id: str) -> List[CopyProtectionTable]:
        return session.query(self.model).filter(CopyProtectionTable.protected_sla_id == str(sla_id)).all()

    def count_obj_by_sla_id(self, session: Session, sla_id: str) -> int:
        return session.query(self.model).filter(CopyProtectionTable.protected_sla_id == str(sla_id)).count()

    def update_status(self, session: Session, resource_ids: List[str], protected_status: BOOLEAN):
        query = session.query(self.model).filter(
            CopyProtectionTable.protected_resource_id.in_(resource_ids))
        query.update({CopyProtectionTable.protected_status: protected_status},
                     synchronize_session='fetch')

    def query_copy_projected_object(self, session: Session) -> List[SlaResourceQuantityRelationship]:
        copy_obj_info = session.query(self.model.protected_sla_id, func.count('*')).group_by(
            self.model.protected_sla_id).all()
        sla_resource_quantity_relationship_list = \
            [SlaResourceQuantityRelationship(sla_id=item[0], resource_count=item[1]) for item in copy_obj_info]
        return sla_resource_quantity_relationship_list


copy_projected_object = CRUDCopyProtectedObject(CopyProtectionTable)
