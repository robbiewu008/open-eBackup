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
import json
from unittest.mock import patch, MagicMock
from apscheduler.jobstores.memory import MemoryJobStore
from sqlalchemy import create_engine

__all__ = ["compose_patch", "RequestMock", "MockKafkaClient", "COPY_INFO", "POLICY"]

COPY_INFO = {"uuid": "8bc8df04-2894-4244-9446-9f8dfa8b65d0",
             "chain_id": "1814b678-3417-4cf1-bb65-d7cb015b90e6",
             "timestamp": "1628767640000000",
             "display_timestamp": "2021-08-12T19:27:20",
             "deletable": True,
             "status": "Normal",
             "location": "Local",
             "backup_type": 2,
             "device_esn": "abcde",
             "generated_by": "Backup",
             "generated_time": "2021-08-12T19:27:20",
             "features": 14,
             "indexed": "Unindexed",
             "generation": 1,
             "parent_copy_uuid": None,
             "retention_type": 1,
             "retention_duration": 0,
             "duration_unit": None,
             "expiration_time": None,
             "properties":"{\"backup_id\": \"8bc8df04-2894-4244-9446-9f8dfa8b65d0\",  \"format\":1,                   "
                          "\"vmware_metadata\": {                       \"disk_info \": [{\"DSNAME \": \"datastore1 ("
                          "4) \",                                       \"DSMOREF \": \"datastore-3304 \",            "
                          "                           \"BUSNUMBER \": \"SCSI(0:0) \",                                 "
                          "      \"GUID \": \"6000c297-57eb-7a82-a3d7-0f1b6d864888 \",                                "
                          "       \"NAME \": \"Hard disk 1 \",                                       \"SIZE \": "
                          "\"10240 \",                                       \"DISKDEVICENAME \": "
                          "\"15627037474758776192 \",                                       \"DISKSNAPSHOTDEVICENAME "
                          "\": \"14846757827670021202 \"}],                       \"vmx_datastore\": {\"uuid\": "
                          "\"datastore-3304:5f56e8d9-be1df0a8-1975-60081085f870\",                                    "
                          "     \"name\": \"datastore1 (4) \", \"mo_id \": \"datastore-3304 \"},                      "
                          " \"net_work\": [\"Network adapter 1 \"],                       \"runtime\": {\"host \": {"
                          "\"name\": \"127.0.0.1\",                                             \"uuid\": "
                          "\"c6018280-3fe5-11e6-80b4-749d8f90f4af \",                                             "
                          "\"version\": \"6.7.0 \",                                             \"mo_id\": "
                          "\"host-3303\"}},                       \"hardware\": {\"num_cpu \": 1,                     "
                          "               \"num_cores_per_socket \": 1,                                    "
                          "\"memory \": 1024,                                    \"controller \": [\"IDE \", "
                          "\"SCSI \", \"SATA \"]}}} ",
             "resource_id": "52fdecfe-2aa2-5b16-aa21-507c752ae4f7",
             "resource_name": "pjw_quick_bk03",
             "resource_type": "VM",
             "resource_sub_type": "vim.VirtualMachine",
             "resource_location": "127.0.0.1/Datacenter/127.0.0.1",
             "resource_status": "EXIST"
             }

INVALID_COPY_INFO = {"uuid": "8bc8df04-2894-4244-9446-9f8dfa8b65d0",
             "chain_id": "1814b678-3417-4cf1-bb65-d7cb015b90e6",
             "timestamp": "1628767640000000",
             "display_timestamp": "2021-08-12T19:27:20",
             "deletable": True,
             "status": "Invalid",
             "location": "Local",
             "backup_type": 2,
             "device_esn": "abcde",
             "generated_by": "Backup",
             "generated_time": "2021-08-12T19:27:20",
             "features": 14,
             "indexed": "Unindexed",
             "generation": 1,
             "parent_copy_uuid": None,
             "retention_type": 1,
             "retention_duration": 0,
             "duration_unit": None,
             "expiration_time": None,
             "properties":"{\"backup_id\": \"8bc8df04-2894-4244-9446-9f8dfa8b65d0\",  \"format\":1,                   "
                          "\"vmware_metadata\": {                       \"disk_info \": [{\"DSNAME \": \"datastore1 ("
                          "4) \",                                       \"DSMOREF \": \"datastore-3304 \",            "
                          "                           \"BUSNUMBER \": \"SCSI(0:0) \",                                 "
                          "      \"GUID \": \"6000c297-57eb-7a82-a3d7-0f1b6d864888 \",                                "
                          "       \"NAME \": \"Hard disk 1 \",                                       \"SIZE \": "
                          "\"10240 \",                                       \"DISKDEVICENAME \": "
                          "\"15627037474758776192 \",                                       \"DISKSNAPSHOTDEVICENAME "
                          "\": \"14846757827670021202 \"}],                       \"vmx_datastore\": {\"uuid\": "
                          "\"datastore-3304:5f56e8d9-be1df0a8-1975-60081085f870\",                                    "
                          "     \"name\": \"datastore1 (4) \", \"mo_id \": \"datastore-3304 \"},                      "
                          " \"net_work\": [\"Network adapter 1 \"],                       \"runtime\": {\"host \": {"
                          "\"name\": \"127.0.0.1\",                                             \"uuid\": "
                          "\"c6018280-3fe5-11e6-80b4-749d8f90f4af \",                                             "
                          "\"version\": \"6.7.0 \",                                             \"mo_id\": "
                          "\"host-3303\"}},                       \"hardware\": {\"num_cpu \": 1,                     "
                          "               \"num_cores_per_socket \": 1,                                    "
                          "\"memory \": 1024,                                    \"controller \": [\"IDE \", "
                          "\"SCSI \", \"SATA \"]}}} ",
             "resource_id": "52fdecfe-2aa2-5b16-aa21-507c752ae4f7",
             "resource_name": "pjw_quick_bk03",
             "resource_type": "VM",
             "resource_sub_type": "vim.VirtualMachine",
             "resource_location": "127.0.0.1/Datacenter/127.0.0.1",
             "resource_status": "EXIST",
             "resource_properties": "",
             "browse_mounted": "Umount"
             }

DELETING_COPY_INFO = {"uuid": "8bc8df04-2894-4244-9446-9f8dfa8b65d0",
             "chain_id": "1814b678-3417-4cf1-bb65-d7cb015b90e6",
             "timestamp": "1628767640000000",
             "display_timestamp": "2021-08-12T19:27:20",
             "deletable": True,
             "status": "Deleting",
             "location": "Local",
             "backup_type": 2,
             "device_esn": "abcde",
             "generated_by": "Backup",
             "generated_time": "2021-08-12T19:27:20",
             "features": 14,
             "indexed": "Unindexed",
             "generation": 1,
             "parent_copy_uuid": None,
             "retention_type": 1,
             "retention_duration": 0,
             "duration_unit": None,
             "expiration_time": None,
             "properties":"{\"backup_id\": \"8bc8df04-2894-4244-9446-9f8dfa8b65d0\",  \"format\":1,                   "
                          "\"vmware_metadata\": {                       \"disk_info \": [{\"DSNAME \": \"datastore1 ("
                          "4) \",                                       \"DSMOREF \": \"datastore-3304 \",            "
                          "                           \"BUSNUMBER \": \"SCSI(0:0) \",                                 "
                          "      \"GUID \": \"6000c297-57eb-7a82-a3d7-0f1b6d864888 \",                                "
                          "       \"NAME \": \"Hard disk 1 \",                                       \"SIZE \": "
                          "\"10240 \",                                       \"DISKDEVICENAME \": "
                          "\"15627037474758776192 \",                                       \"DISKSNAPSHOTDEVICENAME "
                          "\": \"14846757827670021202 \"}],                       \"vmx_datastore\": {\"uuid\": "
                          "\"datastore-3304:5f56e8d9-be1df0a8-1975-60081085f870\",                                    "
                          "     \"name\": \"datastore1 (4) \", \"mo_id \": \"datastore-3304 \"},                      "
                          " \"net_work\": [\"Network adapter 1 \"],                       \"runtime\": {\"host \": {"
                          "\"name\": \"127.0.0.1\",                                             \"uuid\": "
                          "\"c6018280-3fe5-11e6-80b4-749d8f90f4af \",                                             "
                          "\"version\": \"6.7.0 \",                                             \"mo_id\": "
                          "\"host-3303\"}},                       \"hardware\": {\"num_cpu \": 1,                     "
                          "               \"num_cores_per_socket \": 1,                                    "
                          "\"memory \": 1024,                                    \"controller \": [\"IDE \", "
                          "\"SCSI \", \"SATA \"]}}} ",
             "resource_id": "52fdecfe-2aa2-5b16-aa21-507c752ae4f7",
             "resource_name": "pjw_quick_bk03",
             "resource_type": "VM",
             "resource_sub_type": "vim.VirtualMachine",
             "resource_location": "127.0.0.1/Datacenter/127.0.0.1",
             "resource_status": "EXIST",
             "resource_properties": "",
             "browse_mounted": "Umount"
             }

DELETEFAILED_COPY_INFO = {"uuid": "8bc8df04-2894-4244-9446-9f8dfa8b65d0",
             "chain_id": "1814b678-3417-4cf1-bb65-d7cb015b90e6",
             "timestamp": "1628767640000000",
             "display_timestamp": "2021-08-12T19:27:20",
             "deletable": True,
             "status": "DeleteFailed",
             "location": "Local",
             "backup_type": 2,
             "device_esn": "abcde",
             "generated_by": "Backup",
             "generated_time": "2021-08-12T19:27:20",
             "features": 14,
             "indexed": "Unindexed",
             "generation": 1,
             "parent_copy_uuid": None,
             "retention_type": 1,
             "retention_duration": 0,
             "duration_unit": None,
             "expiration_time": None,
             "properties":"{\"backup_id\": \"8bc8df04-2894-4244-9446-9f8dfa8b65d0\",  \"format\":1,                   "
                          "\"vmware_metadata\": {                       \"disk_info \": [{\"DSNAME \": \"datastore1 ("
                          "4) \",                                       \"DSMOREF \": \"datastore-3304 \",            "
                          "                           \"BUSNUMBER \": \"SCSI(0:0) \",                                 "
                          "      \"GUID \": \"6000c297-57eb-7a82-a3d7-0f1b6d864888 \",                                "
                          "       \"NAME \": \"Hard disk 1 \",                                       \"SIZE \": "
                          "\"10240 \",                                       \"DISKDEVICENAME \": "
                          "\"15627037474758776192 \",                                       \"DISKSNAPSHOTDEVICENAME "
                          "\": \"14846757827670021202 \"}],                       \"vmx_datastore\": {\"uuid\": "
                          "\"datastore-3304:5f56e8d9-be1df0a8-1975-60081085f870\",                                    "
                          "     \"name\": \"datastore1 (4) \", \"mo_id \": \"datastore-3304 \"},                      "
                          " \"net_work\": [\"Network adapter 1 \"],                       \"runtime\": {\"host \": {"
                          "\"name\": \"127.0.0.1\",                                             \"uuid\": "
                          "\"c6018280-3fe5-11e6-80b4-749d8f90f4af \",                                             "
                          "\"version\": \"6.7.0 \",                                             \"mo_id\": "
                          "\"host-3303\"}},                       \"hardware\": {\"num_cpu \": 1,                     "
                          "               \"num_cores_per_socket \": 1,                                    "
                          "\"memory \": 1024,                                    \"controller \": [\"IDE \", "
                          "\"SCSI \", \"SATA \"]}}} ",
             "resource_id": "52fdecfe-2aa2-5b16-aa21-507c752ae4f7",
             "resource_name": "pjw_quick_bk03",
             "resource_type": "VM",
             "resource_sub_type": "vim.VirtualMachine",
             "resource_location": "127.0.0.1/Datacenter/127.0.0.1",
             "resource_status": "EXIST",
             "resource_properties": "",
             "browse_mounted": "Umount"
             }

NOT_FULL_OR_NATIVE_COPY_INFO = {"uuid": "8bc8df04-2894-4244-9446-9f8dfa8b65d0",
             "chain_id": "1814b678-3417-4cf1-bb65-d7cb015b90e6",
             "timestamp": "1628767640000000",
             "display_timestamp": "2021-08-12T19:27:20",
             "deletable": True,
             "status": "Normal",
             "location": "Local",
             "backup_type": 4,
             "device_esn": "abcde",
             "generated_by": "Backup",
             "generated_time": "2021-08-12T19:27:20",
             "features": 14,
             "indexed": "Unindexed",
             "generation": 1,
             "parent_copy_uuid": None,
             "retention_type": 1,
             "retention_duration": 0,
             "duration_unit": None,
             "expiration_time": None,
             "properties":"{\"backup_id\": \"8bc8df04-2894-4244-9446-9f8dfa8b65d0\",  \"format\":1,                   "
                          "\"vmware_metadata\": {                       \"disk_info \": [{\"DSNAME \": \"datastore1 ("
                          "4) \",                                       \"DSMOREF \": \"datastore-3304 \",            "
                          "                           \"BUSNUMBER \": \"SCSI(0:0) \",                                 "
                          "      \"GUID \": \"6000c297-57eb-7a82-a3d7-0f1b6d864888 \",                                "
                          "       \"NAME \": \"Hard disk 1 \",                                       \"SIZE \": "
                          "\"10240 \",                                       \"DISKDEVICENAME \": "
                          "\"15627037474758776192 \",                                       \"DISKSNAPSHOTDEVICENAME "
                          "\": \"14846757827670021202 \"}],                       \"vmx_datastore\": {\"uuid\": "
                          "\"datastore-3304:5f56e8d9-be1df0a8-1975-60081085f870\",                                    "
                          "     \"name\": \"datastore1 (4) \", \"mo_id \": \"datastore-3304 \"},                      "
                          " \"net_work\": [\"Network adapter 1 \"],                       \"runtime\": {\"host \": {"
                          "\"name\": \"127.0.0.1\",                                             \"uuid\": "
                          "\"c6018280-3fe5-11e6-80b4-749d8f90f4af \",                                             "
                          "\"version\": \"6.7.0 \",                                             \"mo_id\": "
                          "\"host-3303\"}},                       \"hardware\": {\"num_cpu \": 1,                     "
                          "               \"num_cores_per_socket \": 1,                                    "
                          "\"memory \": 1024,                                    \"controller \": [\"IDE \", "
                          "\"SCSI \", \"SATA \"]}}} ",
             "resource_id": "52fdecfe-2aa2-5b16-aa21-507c752ae4f7",
             "resource_name": "pjw_quick_bk03",
             "resource_type": "VM",
             "resource_sub_type": "vim.VirtualMachine",
             "resource_location": "127.0.0.1/Datacenter/127.0.0.1",
             "resource_status": "EXIST",
             "resource_properties": "",
             "browse_mounted": "Umount"
             }

HAS_ARCHIVE_TO_STORAGE_COPY_INFO = {"uuid": "8bc8df04-2894-4244-9446-9f8dfa8b65d0",
             "chain_id": "1814b678-3417-4cf1-bb65-d7cb015b90e6",
             "timestamp": "1628767640000000",
             "display_timestamp": "2021-08-12T19:27:20",
             "deletable": True,
             "status": "Normal",
             "location": "Local",
             "backup_type": 1,
             "device_esn": "abcde",
             "generated_by": "Backup",
             "generated_time": "2021-08-12T19:27:20",
             "features": 14,
             "indexed": "Unindexed",
             "generation": 1,
             "parent_copy_uuid": None,
             "retention_type": 1,
             "retention_duration": 0,
             "duration_unit": None,
             "expiration_time": None,
             "properties":"{\"backup_id\": \"8bc8df04-2894-4244-9446-9f8dfa8b65d0\",  \"format\":0,                   "
                          "\"vmware_metadata\": {                       \"disk_info \": [{\"DSNAME \": \"datastore1 ("
                          "4) \",                                       \"DSMOREF \": \"datastore-3304 \",            "
                          "                           \"BUSNUMBER \": \"SCSI(0:0) \",                                 "
                          "      \"GUID \": \"6000c297-57eb-7a82-a3d7-0f1b6d864888 \",                                "
                          "       \"NAME \": \"Hard disk 1 \",                                       \"SIZE \": "
                          "\"10240 \",                                       \"DISKDEVICENAME \": "
                          "\"15627037474758776192 \",                                       \"DISKSNAPSHOTDEVICENAME "
                          "\": \"14846757827670021202 \"}],                       \"vmx_datastore\": {\"uuid\": "
                          "\"datastore-3304:5f56e8d9-be1df0a8-1975-60081085f870\",                                    "
                          "     \"name\": \"datastore1 (4) \", \"mo_id \": \"datastore-3304 \"},                      "
                          " \"net_work\": [\"Network adapter 1 \"],                       \"runtime\": {\"host \": {"
                          "\"name\": \"127.0.0.1\",                                             \"uuid\": "
                          "\"c6018280-3fe5-11e6-80b4-749d8f90f4af \",                                             "
                          "\"version\": \"6.7.0 \",                                             \"mo_id\": "
                          "\"host-3303\"}},                       \"hardware\": {\"num_cpu \": 1,                     "
                          "               \"num_cores_per_socket \": 1,                                    "
                          "\"memory \": 1024,                                    \"controller \": [\"IDE \", "
                          "\"SCSI \", \"SATA \"]}}} ",
             "resource_id": "52fdecfe-2aa2-5b16-aa21-507c752ae4f7",
             "resource_name": "pjw_quick_bk03",
             "resource_type": "VM",
             "resource_sub_type": "vim.VirtualMachine",
             "resource_location": "127.0.0.1/Datacenter/127.0.0.1",
             "resource_status": "EXIST",
             "resource_properties": "",
             "browse_mounted": "Umount"
             }

CUMULATIVE_INCREMENT_COPY_INFO = {"uuid": "8bc8df04-2894-4244-9446-9f8dfa8b65d0",
             "chain_id": "1814b678-3417-4cf1-bb65-d7cb015b90e6",
             "timestamp": "1628767640000000",
             "display_timestamp": "2021-08-12T19:27:20",
             "deletable": True,
             "status": "Normal",
             "location": "Local",
             "backup_type": 2,
             "device_esn": "abcde",
             "generated_by": "Backup",
             "generated_time": "2021-08-12T19:27:20",
             "features": 14,
             "indexed": "Unindexed",
             "generation": 1,
             "parent_copy_uuid": None,
             "retention_type": 1,
             "retention_duration": 0,
             "duration_unit": None,
             "expiration_time": None,
             "properties":"{\"backup_id\": \"8bc8df04-2894-4244-9446-9f8dfa8b65d0\",  \"format\":1,                   "
                          "\"vmware_metadata\": {                       \"disk_info \": [{\"DSNAME \": \"datastore1 ("
                          "4) \",                                       \"DSMOREF \": \"datastore-3304 \",            "
                          "                           \"BUSNUMBER \": \"SCSI(0:0) \",                                 "
                          "      \"GUID \": \"6000c297-57eb-7a82-a3d7-0f1b6d864888 \",                                "
                          "       \"NAME \": \"Hard disk 1 \",                                       \"SIZE \": "
                          "\"10240 \",                                       \"DISKDEVICENAME \": "
                          "\"15627037474758776192 \",                                       \"DISKSNAPSHOTDEVICENAME "
                          "\": \"14846757827670021202 \"}],                       \"vmx_datastore\": {\"uuid\": "
                          "\"datastore-3304:5f56e8d9-be1df0a8-1975-60081085f870\",                                    "
                          "     \"name\": \"datastore1 (4) \", \"mo_id \": \"datastore-3304 \"},                      "
                          " \"net_work\": [\"Network adapter 1 \"],                       \"runtime\": {\"host \": {"
                          "\"name\": \"127.0.0.1\",                                             \"uuid\": "
                          "\"c6018280-3fe5-11e6-80b4-749d8f90f4af \",                                             "
                          "\"version\": \"6.7.0 \",                                             \"mo_id\": "
                          "\"host-3303\"}},                       \"hardware\": {\"num_cpu \": 1,                     "
                          "               \"num_cores_per_socket \": 1,                                    "
                          "\"memory \": 1024,                                    \"controller \": [\"IDE \", "
                          "\"SCSI \", \"SATA \"]}}} ",
             "resource_id": "52fdecfe-2aa2-5b16-aa21-507c752ae4f7",
             "resource_name": "pjw_quick_bk03",
             "resource_type": "VM",
             "resource_sub_type": "vim.VirtualMachine",
             "resource_location": "127.0.0.1/Datacenter/127.0.0.1",
             "resource_status": "EXIST",
             "resource_properties": "",
             "browse_mounted": "Umount"
             }

DIFF_INCREMENT_COPY_INFO = {"uuid": "8bc8df04-2894-4244-9446-9f8dfa8b65d0",
             "chain_id": "1814b678-3417-4cf1-bb65-d7cb015b90e6",
             "timestamp": "1628767640000000",
             "display_timestamp": "2021-08-12T19:27:20",
             "deletable": True,
             "status": "Normal",
             "location": "Local",
             "backup_type": 3,
             "device_esn": "abcde",
             "generated_by": "Backup",
             "generated_time": "2021-08-12T19:27:20",
             "features": 14,
             "indexed": "Unindexed",
             "generation": 1,
             "parent_copy_uuid": None,
             "retention_type": 1,
             "retention_duration": 0,
             "duration_unit": None,
             "expiration_time": None,
             "properties":"{\"backup_id\": \"8bc8df04-2894-4244-9446-9f8dfa8b65d0\",  \"format\":1,                   "
                          "\"vmware_metadata\": {                       \"disk_info \": [{\"DSNAME \": \"datastore1 ("
                          "4) \",                                       \"DSMOREF \": \"datastore-3304 \",            "
                          "                           \"BUSNUMBER \": \"SCSI(0:0) \",                                 "
                          "      \"GUID \": \"6000c297-57eb-7a82-a3d7-0f1b6d864888 \",                                "
                          "       \"NAME \": \"Hard disk 1 \",                                       \"SIZE \": "
                          "\"10240 \",                                       \"DISKDEVICENAME \": "
                          "\"15627037474758776192 \",                                       \"DISKSNAPSHOTDEVICENAME "
                          "\": \"14846757827670021202 \"}],                       \"vmx_datastore\": {\"uuid\": "
                          "\"datastore-3304:5f56e8d9-be1df0a8-1975-60081085f870\",                                    "
                          "     \"name\": \"datastore1 (4) \", \"mo_id \": \"datastore-3304 \"},                      "
                          " \"net_work\": [\"Network adapter 1 \"],                       \"runtime\": {\"host \": {"
                          "\"name\": \"127.0.0.1\",                                             \"uuid\": "
                          "\"c6018280-3fe5-11e6-80b4-749d8f90f4af \",                                             "
                          "\"version\": \"6.7.0 \",                                             \"mo_id\": "
                          "\"host-3303\"}},                       \"hardware\": {\"num_cpu \": 1,                     "
                          "               \"num_cores_per_socket \": 1,                                    "
                          "\"memory \": 1024,                                    \"controller \": [\"IDE \", "
                          "\"SCSI \", \"SATA \"]}}} ",
             "resource_id": "52fdecfe-2aa2-5b16-aa21-507c752ae4f7",
             "resource_name": "pjw_quick_bk03",
             "resource_type": "VM",
             "resource_sub_type": "vim.VirtualMachine",
             "resource_location": "127.0.0.1/Datacenter/127.0.0.1",
             "resource_status": "EXIST",
             "resource_properties": "",
             "browse_mounted": "Umount"
             }

POLICY = {
    "uuid": "",
    "name": "full",
    "type": "archiving",
    "action": "full",
    "application": "VMware",
    "retention":
        {"retention_type": 2,
         "retention_duration": 2,
         "duration_unit": "d"},
    "schedule":
        {"trigger": 1,
         "interval": 2,
         "interval_unit": "h",
         "start_time": "2021-02-08 00:00:00",
         "window_start": "00:00:00",
         "window_duration": 2,
         "duration_unit": "h"},
    "ext_parameters":
        {"channel_number": 15,
         "encryption": True,
         "encryption_algorithm": "AES256",
         "qos_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
         "storage_id": "6000c297-57eb-7a82-a3d7-0f1b6d864888",
         "auto_retry": True,
         "auto_retry_times": 3,
         "auto_retry_wait_minutes": 5}
}


class RedisMock:
    def __init__(self):
        self.policy = POLICY
        self.redis_key_value = {"policy": json.dumps(self.policy),
                                "sla": json.dumps("sla"),
                                "2925c6d5-e313-4591-8940-5bebbbd72087": "2925c6d5-e313-4591-8940-5bebbbd72087",
                                "resource_id": "2925c6d5-e313-4591-8940-5bebbbd72087",
                                "AUTO_RETRY_FLAG2925c6d5-e313-4591-8940-5bebbbd72087":
                                    "2925c6d5-e313-4591-8940-5bebbbd72087",
                                "d8a9a370-2591-41c5-be08-6e8eca64c89b": "d8a9a370-2591-41c5-be08-6e8eca64c89b"
                                }

    def get(self, key):
        return self.redis_key_value.get(key)

    def set(self, key, value):
        if key is str:
            self.redis_key_value.update({key: value})
        return None

    def hget(self, name, key):
        return self.get(key)

    def expire(self, request_id, time):
        pass

    def scan_iter(self, key):
        key = key[:-1]
        ret_key = []
        for item in self.redis_key_value.keys():
            if item.startswith(key):
                ret_key.append(item)
        return ret_key

    def setnx(self, key, value):
        self.set(key, value)

    def ttl(self, key):
        return 10

    def delete(self, key):
        self.redis_key_value.pop(key)


class KafkaProducerMock:
    def produce(self, *args, **kwargs):
        pass

    def flush(self, *args, **kwargs):
        pass


class ComposePatch:
    database_mock = patch("sqlalchemy.create_engine", MagicMock(return_value=create_engine("sqlite:///:memory:")))
    sql_job_store_mock = patch("apscheduler.jobstores.sqlalchemy.SQLAlchemyJobStore",
                               MagicMock(return_value=MemoryJobStore()))
    producer_mock = patch("confluent_kafka.Producer", MagicMock(return_value=KafkaProducerMock()))
    redis_mock = patch("app.common.redis_session.redis_session", RedisMock())

    def __init__(self):
        self.funcs = (self.database_mock, self.sql_job_store_mock, self.producer_mock, self.redis_mock)

    def compose(self):
        def deco(f):
            for fun in reversed(self.funcs):
                f = fun(f)
            return f

        return deco


compose_patch = ComposePatch().compose()


class RequestMock:
    def __init__(self, request_id):
        self.request_id = request_id


class MockKafkaClient:
    def topic_handler(self, topic, **kw):
        def decorator(func):
            def wrapper(*args, **kwargs):
                return func(*args, **kwargs)

            return wrapper

        return decorator
