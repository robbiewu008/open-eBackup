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

from app.resource.schemas.env_schemas import EnvironmentResourceSchema


class HdfsSchema(EnvironmentResourceSchema):
    port: int = Field(None, description="受保护环境端口号")
    link_status: int = Field(None, description="集群状态")
    cluster_type: str = Field(None, description="hdfs集群类型")
    fileset_counts: int = Field(None, description="hdfs集群文件集数量")
    create_model: str = Field(None, description="kerberos认证方式")
    access_model: str = Field(None, description="访问方式")
    kerberos_uuid: str = Field(None, description="kerberos的资源id")
