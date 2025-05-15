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
from app.common.event_messages.event import EventBase, EventResponseBase


class GetChainInfo(EventBase):
    default_topic = 'GetChainInfo'

    def __init__(
            self, request_id, protected_obj, policy_id, es_user_id, response_topic=''
    ):
        super().__init__(request_id, GetChainInfo.default_topic, response_topic)
        self.protected_obj = protected_obj
        self.policy_id = policy_id
        self.es_user_id = es_user_id


class ChainInfoResponse(EventResponseBase):
    default_topic = 'ChainInfoResponse'

    def __init__(
            self, request_id, resource_id, chain_id, status="success", error_desc=''
    ):
        super().__init__(
            request_id, ChainInfoResponse.default_topic, status, error_desc
        )
        self.resource_id = resource_id
        self.chain_id = chain_id


class AddBackupSnap(EventBase):
    default_topic = 'AddBackupSnap'

    def __init__(
            self,
            request_id,
            protected_obj,
            policy_obj,
            es_user_id,
            snap_id,
            timestamp,
            chain_id,
            response_topic='',
    ):
        super().__init__(request_id, AddBackupSnap.default_topic, response_topic)
        self.protected_obj = protected_obj
        self.policy_obj = policy_obj
        self.es_user_id = es_user_id
        self.snap_id = snap_id
        self.timestamp = timestamp
        self.chain_id = chain_id


class AddBackupSnapResponse(EventResponseBase):
    default_topic = 'AddBackupSnapResponse'

    def __init__(
            self,
            request_id,
            snap_id,
            policy_obj,
            prot_obj_str,
            status="success",
            error_desc='',
    ):
        super().__init__(
            request_id, AddBackupSnapResponse.default_topic, status, error_desc
        )
        self.snap_id = snap_id
        self.policy_obj = policy_obj
        self.prot_obj = prot_obj_str


class GetSnapAndDestInfo(EventBase):
    default_topic = 'GetSnapAndDestInfo'

    def __init__(self, request_id, snap_id, dest_ip, dest_path, response_topic=''):
        super().__init__(request_id, GetSnapAndDestInfo.default_topic, response_topic)
        self.snap_id = snap_id
        self.dest_ip = dest_ip
        self.dest_path = dest_path


class SnapAndDestInfoResponse(EventResponseBase):
    default_topic = 'SnapAndDestInfoResponse'

    def __init__(
            self,
            request_id,
            dest_resource_id,
            chain_id,
            timestamp,
            status="success",
            error_desc='',
    ):
        super().__init__(
            request_id, SnapAndDestInfoResponse.default_topic, status, error_desc
        )
        self.dest_resource_id = dest_resource_id
        self.chain_id = chain_id
        self.timestamp = timestamp


class GetSnapInfo(EventBase):
    default_topic = 'GetSnapInfo'

    def __init__(self, request_id, snap_id, response_topic=''):
        super().__init__(request_id, GetSnapInfo.default_topic, response_topic)
        self.snap_id = snap_id


class GetSnapInfoResponse(EventResponseBase):
    default_topic = 'GetSnapInfoResponse'

    def __init__(
            self,
            request_id,
            chain_id,
            timestamp,
            es_user_id,
            policy_obj,
            protected_obj,
            status="success",
            error_desc='',
    ):
        super().__init__(
            request_id, GetSnapInfoResponse.default_topic, status, error_desc
        )
        self.chain_id = chain_id
        self.timestamp = timestamp
        self.es_user_id = es_user_id
        self.policy_obj = policy_obj
        self.protected_obj = protected_obj


class DeleteSnap(EventBase):
    default_topic = 'DeleteSnap'

    def __init__(self, request_id, snap_id, x_auth_token, response_topic=''):
        super().__init__(request_id, DeleteSnap.default_topic, response_topic)
        self.snap_id = snap_id
        self.x_auth_token = x_auth_token


class DeleteSnapResponse(EventResponseBase):
    default_topic = 'DeleteSnapResponse'

    def __init__(
            self,
            request_id,
            snap_id,
            policy_obj,
            prot_obj_str,
            status="success",
            error_desc='',
    ):
        super().__init__(
            request_id, DeleteSnapResponse.default_topic, status, error_desc
        )
        self.snap_id = snap_id
        self.policy_obj = policy_obj
        self.prot_obj = prot_obj_str


class GetBackupSnapMetaList(EventBase):
    default_topic = 'GetBackupSnapMetaList'

    def __init__(self, request_id, path='', response_topic=''):
        super().__init__(
            request_id, GetBackupSnapMetaList.default_topic, response_topic
        )
        self.path = path


class GetBackupSnapMetaListResponse(EventResponseBase):
    default_topic = 'GetBackupSnapMetaListResponse'

    def __init__(self, request_id, ret_list, status="success", error_desc=''):
        super().__init__(
            request_id, GetBackupSnapMetaListResponse.default_topic, status, error_desc
        )
        self.ret_list = ret_list
