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
from app.common import logger
from app.common.enums.sla_enum import BackupTypeEnum
from app.common.exception.unified_exception import EmeiStorBizException
from app.copy_catalog.copy_error_code import CopyErrorCode
from app.copy_catalog.db.copies_db import query_next_association_copy, query_last_association_copy, \
    query_first_association_copy, query_association_copies
from app.copy_catalog.models.tables_and_sessions import CopyTable, database

log = logger.get_logger(__name__)


def associated_deletion_copies_for_full(copy: CopyTable, association_type: str) -> list:
    with database.session() as session:
        if not query_next_association_copy(session, copy):
            # 当前副本是最新全量副本允许删除
            return [copy]
        last_copy = query_last_association_copy(session, copy)
        if not last_copy:
            if association_type == "all":
                return []
            # 当前副本最新一个副本链路不允许删除
            raise EmeiStorBizException(CopyErrorCode.FORBID_DELETE_LATEST_COPY_LINK, copy.display_timestamp)
        return query_association_copies(session, copy, last_copy)


def associated_deletion_copies_for_increase(copy: CopyTable, association_type: str):
    with database.session() as session:
        first_copy = query_first_association_copy(session, copy)
        if not first_copy:
            log.error(f"get first copy error copy:{copy.uuid}")
            # 副本链路上的全量副本丢失的情况下，允许删除和过期。
            return [copy]
        last_copy = query_last_association_copy(session, copy)
        if not last_copy:
            if association_type == "all":
                return []
            # 当前副本最新一个副本链路不允许删除
            raise EmeiStorBizException(CopyErrorCode.FORBID_DELETE_LATEST_COPY_LINK, copy.display_timestamp)
        if association_type == "down":
            first_copy = copy
        return query_association_copies(session, first_copy, last_copy)


def associated_deletion_copies(copy: CopyTable, association_type="all"):
    """
    :param association_type:  向后关联副本链:down 全部副本链:all
    :param copy:
    :return:GN值 降序返回关联副本链路列表
    """
    if copy.backup_type == BackupTypeEnum.full.value:
        return associated_deletion_copies_for_full(copy, association_type)
    else:
        return associated_deletion_copies_for_increase(copy, association_type)
