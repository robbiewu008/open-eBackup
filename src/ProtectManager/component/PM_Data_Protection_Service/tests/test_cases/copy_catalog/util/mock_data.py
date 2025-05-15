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
import datetime
import json
import uuid
from tests.test_cases import common_mocker # noqa
from app.common.enums.sla_enum import BackupTypeEnum
from app.copy_catalog.models.tables_and_sessions import CopyTable

COPY_DATA_ASC = [CopyTable(uuid=str(uuid.uuid4()), backup_type=BackupTypeEnum.full.value,
                           expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                           display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=1,
                           properties=json.dumps({"backup_type": "full"}), timestamp="1638780643176000",
                           deletable=True,
                           status="Normal", generated_by="Backup", indexed="Indexing", generation=1, retention_type=2,
                           resource_id="f124bdf9-25b7-47bf-9dd4-5926f345b9e9", resource_name="test_resource",
                           resource_type="GaussDB", resource_location="8.40.106.11", resource_status="EXIST",
                           resource_properties=json.dumps({"name": "test_resource", "sla_name": "test001"}),
                           worm_status=1, storage_snapshot_flag=False, extend_type="ABC", browse_mounted="Umount"),
                 CopyTable(uuid=str(uuid.uuid4()), backup_type=BackupTypeEnum.cumulative_increment.value,
                           expiration_time=datetime.datetime(2021, 4, 21, 2, 0, 0), chain_id=str(uuid.uuid4()),
                           display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=2,
                           properties=json.dumps({"backup_type": "cumulative_increment"}),
                           resource_properties=json.dumps({"name": "test_resource", "sla_name": "test001"})),
                 CopyTable(uuid=str(uuid.uuid4()), backup_type=BackupTypeEnum.cumulative_increment.value,
                           expiration_time=datetime.datetime(2021, 4, 20, 2, 0, 0), chain_id=str(uuid.uuid4()),
                           display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=3,
                           properties=json.dumps({"backup_type": "cumulative_increment"}),
                           resource_properties=json.dumps({"name": "test_resource", "sla_name": "test001"})),
                 CopyTable(uuid=str(uuid.uuid4()), backup_type=BackupTypeEnum.full.value,
                           expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                           display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=4,
                           properties=json.dumps({"backup_type": "full"}),
                           resource_properties=json.dumps({"name": "test_resource", "sla_name": "test001"}))
                 ]

OLD_SLA = {"uuid": "f124bdf9-25b7-47bf-9dd4-5926f345b9e9", "name": "test001"}
NEW_SLA = {"uuid": "f124bdf9-25b7-47bf-9dd4-5926f345b9e9", "name": "fileset001"}