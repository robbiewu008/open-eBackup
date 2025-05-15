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
from app.common.enums.sla_enum import PolicyActionEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import IllegalParamException

from app.backup.common.validators.validator import Validator


class LogPolicyValidator(Validator):

    @staticmethod
    def is_support(sub_type) -> bool:
        return sub_type not in [ResourceSubTypeEnum.Oracle.value, ResourceSubTypeEnum.SQLServer.value,
                                ResourceSubTypeEnum.K8S_MySQL_dataset.value]

    @staticmethod
    def do_validate(policies):
        for policy_obj in policies:
            if policy_obj.name == PolicyActionEnum.log.value:
                raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["application_type"],
                                            "This sla can not have log backup policy.")
