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
from dataclasses import dataclass

from sqlalchemy import Table, Column, String, Integer, MetaData, Text
from sqlalchemy.orm import mapper

DB_TABLE_RESOURCES = "lock_resources"

DB_COLUMN_LOCK_ID = "lock_id"
DB_COLUMN_RESOURCE_ID = "resource_id"
DB_COLUMN_LOCK_TYPE = "lock_type"
DB_COLUMN_LOCK_STATE = "lock_state"
DB_COLUMN_PRIORITY = "priority"
DB_COLUMN_TIMESTAMP = "timestamp"

metadata = MetaData()


resources = Table(
    DB_TABLE_RESOURCES,
    metadata,
    Column(DB_COLUMN_LOCK_ID, String(128), primary_key=True),
    Column(DB_COLUMN_RESOURCE_ID, String(64), primary_key=True),
    Column(DB_COLUMN_LOCK_TYPE, String(4)),
    Column(DB_COLUMN_LOCK_STATE, String(8)),
    Column(DB_COLUMN_PRIORITY, Integer),
    Column(DB_COLUMN_TIMESTAMP, Integer),
)


@dataclass
class DBResource(object):
    lock_id: str
    resource_id: str
    lock_type: str
    timestamp: int
    lock_state: str
    priority: int


resource_mapper = mapper(DBResource, resources)


DB_TABLE_MSG_REGISTRY = "msg_registry"
DB_COLUMN_MSG_REGISTRY_KEY = "key"
DB_COLUMN_MSG_REGISTRY_VALUE = "value"
msg_registry = Table(
    DB_TABLE_MSG_REGISTRY,
    metadata,
    Column(DB_COLUMN_MSG_REGISTRY_KEY, String(128), primary_key=True),
    Column(DB_COLUMN_MSG_REGISTRY_VALUE, Text),
)


@dataclass
class DBMsgRegistryObj(object):
    key: str
    value: str


lock_mapper = mapper(DBMsgRegistryObj, msg_registry)
