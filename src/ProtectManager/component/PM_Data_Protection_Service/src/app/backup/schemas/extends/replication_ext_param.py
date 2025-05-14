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
from pydantic import Field, validator

from app.backup.client.replication_client import ReplicationClient
from app.backup.schemas.base_ext_param import BaseExtendParam
from app.common.enums.sla_enum import PolicyTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import IllegalParamException


class ReplicationExtendParam(BaseExtendParam):
    """
    复制扩展参数
    """

    external_system_id: str = Field(description="外部系统id")

    @staticmethod
    def is_support(application, policy_type, action) -> bool:
        return policy_type is PolicyTypeEnum.replication

    @validator("external_system_id", pre=True)
    def validate_external_system_id(cls, external_system_id):
        """
        校验外部系统id是否正确，并存在
        :param external_system_id: 外部系统id
        :return: 外部系统id
        """
        external_system_objects = ReplicationClient.query_external_system()
        external_system_ids = [external_system_object.get("clusterId") for external_system_object in
                               external_system_objects]
        if external_system_id not in external_system_ids:
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                        ["external_system_id"])
        return external_system_id
