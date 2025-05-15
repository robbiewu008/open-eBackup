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
from enum import Enum

from app.common.enums.resource_enum import ResourceSubTypeEnum


class AgentSubeTypeEnum(str, Enum):
    VMBackupAgent = "VMBackupAgent"
    # Oracleԭ����ʽ
    DBBackupAgent = "DBBackupAgent"


class FilesetTemplateOsTypeEnum(str, Enum):
    # Windows
    WINDOWS = "windows"
    # 非Windows
    NON_WINDOWS = "non-windows"


class HostMigrationAgentType(str, Enum):
    HOST_AGENT_ORACLE = "HOST_AGENT_ORACLE"
    REMOTE_AGENT_VMWARE = "REMOTE_AGENT_VMWARE"
    REMOTE_AGENT = "REMOTE_AGENT"
    SAN_CLIENT_AGENT = "SAN_CLIENT_AGENT"


class HostMigrationOsType(str, Enum):
    LINUX = "LINUX"
    UNIX = "UNIX"
    WINDOWS = "WINDOWS"


class HostMigrationIpType(str, Enum):
    IPV4 = "IPV4"
    IPV6 = "IPV6"


class MigrateType(str, Enum):
    HOST_MIGRATE = '0'


class TargetClusterStatus(str, Enum):
    ON_LINE = 27
    OFF_LINE = 28


class ProxyHostTypeEnum(Enum):
    DBBackupAgent = HostMigrationAgentType.HOST_AGENT_ORACLE.value
    VMBackupAgent = HostMigrationAgentType.REMOTE_AGENT_VMWARE.value
    DWSBackupAgent = HostMigrationAgentType.REMOTE_AGENT.value
    UBackupAgent = HostMigrationAgentType.REMOTE_AGENT.value
    SBackupAgent = HostMigrationAgentType.SAN_CLIENT_AGENT.value

    @staticmethod
    def get_type_by_proxy(proxy):
        return ProxyHostTypeEnum.__dict__.get(proxy).value
