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

from sqlalchemy.orm import Session

from app.protection.object.models.projected_object import ProtectedTask


class CRUDProtectedTask:

    @staticmethod
    def batch_create(db: Session, task_list: List[ProtectedTask]):
        db.add_all(task_list)

    @staticmethod
    def create(db: Session, task: ProtectedTask):
        db.add(task)

    @staticmethod
    def batch_delete(db: Session, task_id_list: List[str]):
        db.query(ProtectedTask).filter(ProtectedTask.uuid.in_(task_id_list)).delete(synchronize_session=False)

    @staticmethod
    def delete_by_protected_obj_id(db: Session, protected_obj_id: str):
        db.query(ProtectedTask).filter(ProtectedTask.protected_object_id == protected_obj_id).delete(
            synchronize_session=False)

    @staticmethod
    def modify_task(db: Session, uuid: str, update_conditions: dict):
        return db.query(ProtectedTask).filter(ProtectedTask.uuid == uuid).update(update_conditions)

    @staticmethod
    def query_one_task(db: Session, protected_object_id: str, policy_id: str) -> ProtectedTask:
        return db.query(ProtectedTask).filter(
            ProtectedTask.protected_object_id == protected_object_id and ProtectedTask.policy_id == policy_id).first()
