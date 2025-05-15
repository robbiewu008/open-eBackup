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
nas_filesystem_sla_with_interval_policies = {
    "name": "temp_sla",
    "type": 1,
    "application": "NasFileSystem",
    "created_time": "2022-02-19T18:38:29.337870",
    "uuid": "db2e0438-521a-4add-8da0-4837330f7f46",
    "is_global": False,
    "policy_list": [
        {
            "uuid": "541e5f3a-ce7e-466b-bf69-77a8ed705777",
            "name": "permanent_increment",
            "action": "permanent_increment",
            "ext_parameters": {
                "auto_retry": False,
                "qos_id": "",
                "auto_index": True
            },
            "retention": {
                "retention_type": 2,
                "duration_unit": "d",
                "retention_duration": 5
            },
            "schedule": {
                "trigger": 1,
                "interval": 5,
                "interval_unit": "h",
                "start_time": "2022-02-06T00:00:00",
                "window_start": "00:00:00",
                "window_end": "22:00:05",
            },
            "type": "backup"
        },
        {
            "uuid": "ffd5b855-6a3f-4859-8ae4-2e7aa9adb625",
            "name": "策略0",
            "action": "archiving",
            "ext_parameters": {
                "qos_id": "",
                "storage_id": "fd21c964596c4cb2afa904c53caa3171",
                "archive_target_type": 1,
                "archiving_scope": "latest",
                "protocol": 2
            },
            "retention": {
                "retention_type": 2,
                "duration_unit": "d",
                "retention_duration": 5
            },
            "schedule": {
                "trigger": 1,
                "interval": 5,
                "interval_unit": "h",
                "start_time": "2022-02-19T18:37:00",
            },
            "type": "archiving"
        },
        {
            "uuid": "373505e9-8acd-43e8-9575-ee5d3ad326eb",
            "name": "策略1",
            "action": "archiving",
            "ext_parameters": {
                "qos_id": "",
                "storage_id": "fd21c964596c4cb2afa904c53caa3171",
                "archive_target_type": 1,
                "archiving_scope": "latest",
                "network_access": True,
                "auto_retry": True,
                "auto_retry_times": 3,
                "auto_retry_wait_minutes": 5,
                "protocol": 2
            },
            "retention": {
                "retention_type": 2,
                "duration_unit": "d",
                "retention_duration": 12
            },
            "schedule": {
                "trigger": 2
            },
            "type": "archiving"
        },
        {
            "uuid": "e68b51e0-3f3a-4a18-8a9a-ce2f2e3b8d1a",
            "name": "策略0",
            "action": "replication",
            "ext_parameters": {
                "qos_id": "",
                "external_system_id": "2"
            },
            "retention": {
                "retention_type": 2,
                "duration_unit": "d",
                "retention_duration": 5
            },
            "schedule": {
                "trigger": 1,
                "interval": 5,
                "interval_unit": "h",
                "start_time": "2022-02-19T18:38:00",
            },
            "type": "replication"
        }
    ]
}
