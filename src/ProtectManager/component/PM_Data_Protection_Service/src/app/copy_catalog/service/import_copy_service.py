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
from app.copy_catalog.schemas import CopyInfoWithArchiveFieldsSchema
from app.copy_catalog.models.tables_and_sessions import CopyTable, database, CopyArchiveMapTable
from app.common import logger


log = logger.get_logger(__name__)


def query_copy_info_by_copy_id(copy_id: str):
    with database.session() as session:
        copy_in_db, archive_map = session.query(CopyTable, CopyArchiveMapTable).filter(CopyTable.uuid == copy_id) \
            .outerjoin(CopyArchiveMapTable, CopyTable.uuid == CopyArchiveMapTable.copy_id).first()
        copy_info_schema = CopyInfoWithArchiveFieldsSchema.from_orm(copy_in_db)
        if archive_map is not None:
            copy_info_schema.storage_id = archive_map.storage_id
        return copy_info_schema
