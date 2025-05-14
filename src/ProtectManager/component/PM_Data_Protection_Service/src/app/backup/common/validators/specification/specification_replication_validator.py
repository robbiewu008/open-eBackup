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
from app.common.config import settings
from app.common.enums.sla_enum import PolicyTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.protection_error_codes import ProtectionErrorCodes
from app.common.exception.unified_exception import IllegalParamException, EmeiStorBizException


class SpecificationReplicationValidator:

    @staticmethod
    def do_validate(policies):
        replication_policies = [policy for policy in policies
                                if policy.type == PolicyTypeEnum.replication.value]

        if len(replication_policies) == 0:
            return

        SpecificationReplicationValidator._check_replication_policies_count_lt_limit(replication_policies)
        SpecificationReplicationValidator._check_replication_policies_name_not_repeat(replication_policies)
        SpecificationReplicationValidator._check_replication_policies_external_system_id_not_repeat(
            replication_policies)

    @staticmethod
    def _check_replication_policies_count_lt_limit(replication_policies):
        if len(replication_policies) > settings.REPLICATION_POLICY_COUNT_LIMIT:
            raise EmeiStorBizException(
                ProtectionErrorCodes.REPLICATION_POLICY_COUNT_OVER_LIMIT, settings.REPLICATION_POLICY_COUNT_LIMIT)

    @staticmethod
    def _check_replication_policies_name_not_repeat(replication_policies):
        if len(replication_policies) > 1:
            name_list = [policy.name for policy in replication_policies]
            name_set = set(name_list)
            if len(name_list) != len(name_set):
                raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                            ["policy.name"],
                                            "replication policy name repeated")

    @staticmethod
    def _check_replication_policies_external_system_id_not_repeat(replication_policies):
        if len(replication_policies) > 1:
            system_id_list = [policy.ext_parameters.external_system_id for policy in replication_policies]
            system_id_set = set(system_id_list)
            if len(system_id_list) != len(system_id_set):
                raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                            ["policy.ext_parameter.external_system_id"],
                                            "replication policy external_system_id repeated")
