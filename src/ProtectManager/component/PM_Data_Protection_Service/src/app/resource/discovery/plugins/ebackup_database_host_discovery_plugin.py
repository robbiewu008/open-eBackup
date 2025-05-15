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
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.logger import get_logger
from app.resource.discovery.discovery_plugin import DiscoveryPlugin
from app.resource.schemas.env_schemas import ScanEnvSchema, UpdateEnvSchema
from app.resource.service.host.host_service import delete_environment

log = get_logger(__name__)

SERVICES = [ResourceSubTypeEnum.DBBackupAgent]


class DbNativeBackupAgentDiscoveryPlugin(DiscoveryPlugin):
    service_type = ResourceSubTypeEnum.DBBackupAgent

    def do_scan_env(self, params: ScanEnvSchema, is_rescan=False, is_session_connect=False):
        # oracle代理已废弃，保留文件是为了升级x8000不升级代理时不报错
        pass

    def do_modify_env(self, params: UpdateEnvSchema):
        pass

    def do_delete_env(self, params: str):
        delete_environment(params)


def create():
    return DbNativeBackupAgentDiscoveryPlugin()
