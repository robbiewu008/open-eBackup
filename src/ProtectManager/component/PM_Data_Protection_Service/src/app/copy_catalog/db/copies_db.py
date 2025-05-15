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

from app.common.context.db_session import Session
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.sla_enum import BackupTypeEnum
from app.copy_catalog.models.tables_and_sessions import CopyTable
from app.copy_catalog.schemas import CloudBackupCopySchema

CLOUD_BACKUP_COPY_FLAG = None


def query_first_association_copy(session, copy):
    condition = {
        CopyTable.resource_id == copy.resource_id,
        CopyTable.resource_sub_type == ResourceSubTypeEnum.CloudBackupFileSystem.value,
        CopyTable.gn < copy.gn,
        CopyTable.backup_type == BackupTypeEnum.full.value
    }
    first_copy = session.query(CopyTable).filter(*condition).order_by(CopyTable.gn.desc()).first()
    if first_copy:
        return CloudBackupCopySchema(**first_copy.as_dict())
    return CLOUD_BACKUP_COPY_FLAG


def query_next_association_copy(session, copy):
    condition = {
        CopyTable.resource_id == copy.resource_id,
        CopyTable.resource_sub_type == ResourceSubTypeEnum.CloudBackupFileSystem.value,
        CopyTable.gn > copy.gn
    }
    next_copy = session.query(CopyTable).filter(*condition).order_by(CopyTable.gn.asc()).first()
    if next_copy:
        return CloudBackupCopySchema(**next_copy.as_dict())
    return CLOUD_BACKUP_COPY_FLAG


def query_last_association_copy(session, copy):
    condition = {
        CopyTable.resource_id == copy.resource_id,
        CopyTable.resource_sub_type == ResourceSubTypeEnum.CloudBackupFileSystem.value,
        CopyTable.gn > copy.gn,
        CopyTable.backup_type == BackupTypeEnum.full.value
    }
    last_copy = session.query(CopyTable).filter(*condition).order_by(CopyTable.gn.asc()).first()
    if last_copy:
        return CloudBackupCopySchema(**last_copy.as_dict())
    return CLOUD_BACKUP_COPY_FLAG


def query_association_copies(session, first_copy, last_copy):
    condition = {
        CopyTable.resource_id == first_copy.resource_id,
        CopyTable.resource_sub_type == ResourceSubTypeEnum.CloudBackupFileSystem.value,
        CopyTable.gn >= first_copy.gn,
        CopyTable.gn < last_copy.gn
    }
    association_copies = session.query(CopyTable).filter(*condition).order_by(CopyTable.gn.desc()).all()
    association_copy_list = []
    for association_copy in association_copies:
        association_copy_list.append(CloudBackupCopySchema(**association_copy.as_dict()))
    return association_copy_list


def count_by_user_id_and_resource_ids(session: Session, user_id: str, resource_uuid_list: List[str]) -> int:
    return session.query(CopyTable).filter(CopyTable.resource_id.in_(resource_uuid_list))\
        .filter(CopyTable.user_id == user_id)\
        .filter(CopyTable.deleted.is_(False)).count()
