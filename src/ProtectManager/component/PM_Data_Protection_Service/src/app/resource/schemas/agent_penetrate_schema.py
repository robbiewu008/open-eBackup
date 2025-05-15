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
from pydantic import BaseModel, Field


class AgentPenetrateUpgradeRequestSchema(BaseModel):
    ip: str = Field(..., description="主机ip", max_length=128)
    port: str = Field(..., description="主机端口", max_length=16)
    download_link: str = Field(..., alias="downloadLink", description="下载链接", max_length=2048)
    agent_id: str = Field(..., alias="agentId", description="Agent ID", min_length=10, max_length=128)
    agent_name: str = Field(..., alias="agentName", description="Agent名称", max_length=256)
    connect_need_proxy: bool = Field(..., description="是否需要代理连接")
    jobId: str = Field(..., description="任务id", max_length=128)
    cert_secret_key: str = Field(..., description="证书密钥，已经加密")
    new_package_size: int = Field(..., description="新的安装包大小(kb)")
    packageType: str = Field(..., description="安装命令", max_length=16)


class CheckAgentUpgradeStatusRequestSchema(BaseModel):
    ip: str = Field(..., description="主机ip", max_length=128)
    port: str = Field(..., description="主机端口", max_length=16)
    connect_need_proxy: bool = Field(..., description="是否需要代理连接")
