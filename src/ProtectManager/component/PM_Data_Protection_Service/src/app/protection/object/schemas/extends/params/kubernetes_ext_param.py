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
from typing import List, Optional

from pydantic import BaseModel, Field, root_validator, validator

from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.protection.object.common.protection_enums import SlaApplyType, ResourceFilter
from app.protection.object.schemas.extends.base_ext_param import BaseExtParam


PROTECTION_REGEX = "^(.+)[\.]{1}(sh|py){1}$"


class NamespaceExtParam(BaseExtParam):
    agents: str = Field(None, description="agents的id,多个用,隔开")
    pre_script: str = Field(None, description="前置执行脚本", regex=PROTECTION_REGEX, max_length=8192)
    post_script: str = Field(None, description="后置执行脚本", regex=PROTECTION_REGEX, max_length=8192)
    failed_script: Optional[str] = Field(None, description="备份失败运行脚本", regex=PROTECTION_REGEX, max_length=8192)
    overwrite: bool = Field(description="主机/Cluster下虚拟机已经绑定SLA时，SLA的覆盖策略")
    binding_policy: List[SlaApplyType] = Field(description="主机/Cluster下虚拟机未绑定SLA时，SLA应用策略")
    resource_filters: Optional[List[ResourceFilter]] = Field(description="批量保护过滤规则列表")
    disk_filters: Optional[List[ResourceFilter]] = Field(None, description="磁盘过滤规则列表")

    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.KubernetesNamespace]

    @validator('resource_filters', pre=True)
    def check_namespace_ext_params_resource_filters(cls, values):
        return check_resource_filters(values)

    @validator('agents', pre=True)
    def check_namespace_ext_params_agents(cls, agents):
        if not agents:
            raise EmeiStorBizException(error=CommonErrorCodes.ERR_PARAM,
                                       message="The agents value should not null in the KubernetesNamespace")
        return agents


class StatefulSetExtParam(BaseExtParam):
    """
    StatefulSet保护高级参数
    """
    agents: str = Field(None, description="agents的id,多个用,隔开")
    pre_script: Optional[str] = Field(description="前置执行脚本", regex=PROTECTION_REGEX, max_length=8192)
    post_script: Optional[str] = Field(description="后置执行脚本", regex=PROTECTION_REGEX, max_length=8192)
    failed_script: Optional[str] = Field(None, description="备份失败运行脚本", regex=PROTECTION_REGEX, max_length=8192)
    volume_names: Optional[List[str]] = Field(description="volumeNames信息")

    @staticmethod
    def support_values() -> List[object]:
        return [ResourceSubTypeEnum.KubernetesStatefulSet]

    @validator('agents', pre=True)
    def check_stateful_set_ext_params_agents(cls, agents):
        if not agents:
            raise EmeiStorBizException(error=CommonErrorCodes.ERR_PARAM,
                                       message="The agents value should not null in the StatefulSet")
        return agents


def check_resource_filters(resource_filters):
    if not resource_filters:
        return resource_filters
    for resource_filter in resource_filters:
        values = resource_filter.get("values")
        if not values:
            raise EmeiStorBizException(error=CommonErrorCodes.ERR_PARAM,
                                       message="The filters value should not null in the KubernetesNamespace")

    return resource_filters