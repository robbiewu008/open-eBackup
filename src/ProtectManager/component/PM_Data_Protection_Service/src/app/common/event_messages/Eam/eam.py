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
from pydantic import Field

from app.common.event_messages.Common import livemount_vm
from app.common.event_messages.Common.consts import MAX_NAME_LEN
from app.common.event_messages.event import EventBase, EventResponseBase, EventMessage


class BackupVmRequest(EventBase):
    default_topic = 'BackupVmRequest'

    def __init__(self, request_id, es_task_id, protected_obj, chain_id, backup_type, config, response_topic=''):
        super().__init__(request_id, BackupVmRequest.default_topic, response_topic)
        self.es_task_id = es_task_id
        self.protected_obj = protected_obj
        self.chain_id = chain_id
        self.backup_type = backup_type
        self.config = config


class BackupFileSetRequest(EventBase):
    default_topic = 'BackupFileSetRequest'

    def __init__(self, request_id, es_task_id, protected_obj, chain_id, backup_type, config, response_topic=''):
        super().__init__(request_id, BackupFileSetRequest.default_topic, response_topic)
        self.es_task_id = es_task_id
        self.protected_obj = protected_obj
        self.chain_id = chain_id
        self.backup_type = backup_type
        self.config = config


class BackupRequest(EventBase):
    default_topic = 'protection.backup'

    def __init__(self, request_id, task_id, resource_id, protected_object, chain_id, backup_type, config, sla,
                 response_topic=''):
        super().__init__(request_id, "protection.backup", response_topic)
        self.task_id = task_id
        self.resource_id = resource_id
        self.protected_obj = protected_object
        self.chain_id = chain_id
        self.backup_type = backup_type
        self.config = config
        self.sla = sla


class BackupDBRequest(EventBase):
    default_topic = 'BackupDBRequest'

    def __init__(self, request_id, es_task_id, protected_obj, chain_id, backup_type, config, response_topic=''):
        super().__init__(request_id, BackupDBRequest.default_topic, response_topic)
        self.es_task_id = es_task_id
        self.protected_obj = protected_obj
        self.chain_id = chain_id
        self.backup_type = backup_type
        self.config = config


class BackupVmResponse(EventResponseBase):
    default_topic = 'BackupVmResponse'

    def __init__(self, request_id, snap_id, timestamp, metadata=None, status="success", error_desc=''):
        super().__init__(request_id, BackupVmResponse.default_topic, status, error_desc)
        if metadata is None:
            metadata = []
        self.snap_id = snap_id
        self.timestamp = timestamp
        self.metadata = metadata


class BackupFileSetResponse(EventResponseBase):
    default_topic = 'BackupFileSetResponse'

    def __init__(self, request_id, snap_id, timestamp, status="success", error_desc=''):
        super().__init__(request_id, BackupFileSetResponse.default_topic, status, error_desc)
        self.snap_id = snap_id
        self.timestamp = timestamp


class BackupDBResponse(EventResponseBase):
    default_topic = 'BackupDBResponse'

    def __init__(self, request_id, snap_id, timestamp, status="success", error_desc=''):
        super().__init__(request_id, BackupDBResponse.default_topic, status, error_desc)
        self.snap_id = snap_id
        self.timestamp = timestamp


class NativeBackupRequest(EventBase):
    default_topic = 'NativeBackupRequest'

    def __init__(
            self,
            request_id,
            es_task_id,
            protected_obj,
            chain_id,
            backup_type,
            response_topic='',
    ):
        super().__init__(request_id, NativeBackupRequest.default_topic, response_topic)
        self.es_task_id = es_task_id
        self.protected_obj = protected_obj
        self.chain_id = chain_id
        self.backup_type = backup_type


class NativeBackupResponse(EventResponseBase):
    default_topic = 'NativeBackupResponse'

    def __init__(self, request_id, snap_id, timestamp, metadata=None, status="success", error_desc=''):
        super().__init__(
            request_id, NativeBackupResponse.default_topic, status, error_desc
        )
        if metadata is None:
            metadata = []
        self.snap_id = snap_id
        self.timestamp = timestamp
        self.metadata = metadata


class EamScanRequest(EventBase):
    default_topic = 'EamScanRequest'

    def __init__(self, request_id, snap_id, response_topic=''):
        super().__init__(request_id, EamScanRequest.default_topic, response_topic)
        self.snap_id = snap_id


class EamScanResponse(EventResponseBase):
    default_topic = 'EamScanResponse'

    def __init__(self, request_id, path, status="success", error_desc=''):
        super().__init__(request_id, EamScanResponse.default_topic, status, error_desc)
        self.path = path


class RestoreVmRequest(EventBase):
    default_topic = 'RestoreVmRequest'

    def __init__(
            self,
            request_id,
            env_type,
            snap_id,
            src_restore_paths,
            dest_info,
            post_restore_config,
            response_topic='',
    ):
        super().__init__(request_id, RestoreVmRequest.default_topic, response_topic)
        self.env_type = env_type
        self.snap_id = snap_id
        self.src_restore_paths = src_restore_paths
        self.dest_info = dest_info
        self.post_restore_config = post_restore_config


class RestoreFileSetRequest(EventBase):
    default_topic = 'RestoreFileSetRequest'

    def __init__(
            self,
            request_id,
            env_type,
            snap_id,
            src_restore_paths,
            dest_info,
            replace_mode,
            response_topic='',
    ):
        super().__init__(request_id, RestoreFileSetRequest.default_topic, response_topic)
        self.env_type = env_type
        self.snap_id = snap_id
        self.src_restore_paths = src_restore_paths
        self.dest_info = dest_info
        self.replace_mode = replace_mode


class RestoreDBRequest(EventBase):
    default_topic = 'RestoreDBRequest'

    def __init__(
            self,
            request_id,
            env_type,
            snap_id,
            original_database,
            time_point,
            scn,
            dest_info,
            restore_config,
            response_topic='',
    ):
        super().__init__(request_id, RestoreDBRequest.default_topic, response_topic)
        self.env_type = env_type
        self.snap_id = snap_id
        self.original_database = original_database
        self.time_point = time_point
        self.scn = scn
        self.dest_info = dest_info
        self.restore_config = restore_config


class RestoreVmResponse(EventResponseBase):
    default_topic = 'RestoreVmResponse'

    def __init__(self, request_id, status="success", error_desc=''):
        super().__init__(request_id, RestoreVmResponse.default_topic, status, error_desc)


class RestoreFileSetResponse(EventResponseBase):
    default_topic = 'RestoreFileSetResponse'

    def __init__(self, request_id, status="success", error_desc=''):
        super().__init__(request_id, RestoreFileSetResponse.default_topic, status, error_desc)


class RestoreDBResponse(EventResponseBase):
    default_topic = 'RestoreDBResponse'

    def __init__(self, request_id, status="success", error_desc=''):
        super().__init__(request_id, RestoreDBResponse.default_topic, status, error_desc)


class LivemountHostRequest(EventBase):
    default_topic = 'LivemountHostRequest'

    def __init__(self, request_id, snap_id, dest_info, response_topic=''):
        super().__init__(request_id, LivemountHostRequest.default_topic, response_topic)
        self.dest_info = dest_info
        self.snap_id = snap_id


class LivemountHostResponse(EventResponseBase):
    default_topic = 'LivemountHostResponse'

    def __init__(
            self, request_id, mount_id, mount_path, lu_info, status="success", error_desc=''
    ):
        super().__init__(
            request_id, LivemountHostResponse.default_topic, status, error_desc
        )
        self.mount_id = mount_id
        self.mount_path = mount_path
        self.lu_info = lu_info


class EamDeleteSnap(EventBase):
    default_topic = 'EamDeleteSnap'

    def __init__(self, request_id, snap_id, response_topic=''):
        super().__init__(request_id, EamDeleteSnap.default_topic, response_topic)
        self.snap_id = snap_id


class EamDeleteSnapResponse(EventResponseBase):
    default_topic = 'EamDeleteSnapResponse'

    def __init__(self, request_id, status="success", error_desc=''):
        super().__init__(
            request_id, EamDeleteSnapResponse.default_topic, status, error_desc
        )


class EamDeleteNativeSnap(EventBase):
    default_topic = 'EamDeleteNativeSnap'

    def __init__(self, request_id, snap_id, response_topic=''):
        super().__init__(request_id, EamDeleteNativeSnap.default_topic, response_topic)
        self.snap_id = snap_id


class EamDeleteNativeSnapResponse(EventResponseBase):
    default_topic = 'EamDeleteNativeSnapResponse'

    def __init__(self, request_id, status="success", error_desc=''):
        super().__init__(
            request_id, EamDeleteNativeSnapResponse.default_topic, status, error_desc
        )


class LivemountVMRequest(EventMessage):
    default_publish_topic: str = 'LivemountVMRequest'
    dest_info: livemount_vm.DestinationInfo
    mount_info: livemount_vm.MountInfo
    snap_id: str = Field(..., max_length=MAX_NAME_LEN)


class LivemountVMResponse(EventMessage):
    default_publish_topic: str = 'LivemountVMResponse'
    mount_id: str = Field(..., max_length=MAX_NAME_LEN)
    mount_path: str
    lu_info: str
    status: str = 'success'
    error_desc: str = ''


class RemoveLivemountVMRequest(EventMessage):
    default_publish_topic: str = 'DeleteLivemountVMRequest'
    remove_info: livemount_vm.RemoveLivemountVMData


class RemoveLivemountVMResponse(EventMessage):
    default_publish_topic: str = 'DeleteLivemountVMResponse'
    status: str = 'success'
    error_desc: str = ''


class ProtectionRemoveEvent(EventBase):
    default_topic = 'protection.remove.event'

    def __init__(self, request_id, resource_id, sla_id, response_topic='', is_remove_line=True):
        super().__init__(request_id, ProtectionRemoveEvent.default_topic, response_topic)
        self.resource_id = resource_id
        self.sla_id = sla_id
        self.is_remove_line = is_remove_line
