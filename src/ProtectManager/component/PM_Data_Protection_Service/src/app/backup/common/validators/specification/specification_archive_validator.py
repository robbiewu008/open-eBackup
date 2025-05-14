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


class SpecificationArchiveValidator:

    @staticmethod
    def do_validate(policies):
        archive_policies = [policy for policy in policies
                            if policy.type == PolicyTypeEnum.archiving.value]

        if len(archive_policies) == 0:
            return

        SpecificationArchiveValidator._check_archive_policies_count_lt_limit(archive_policies)
        SpecificationArchiveValidator._check_archive_policies_name_not_repeat(archive_policies)
        SpecificationArchiveValidator._check_archive_policies_storage_id_not_repeat(archive_policies)

    @staticmethod
    def _check_archive_policies_count_lt_limit(archive_policies):
        if len(archive_policies) > settings.ARCHIVE_POLICY_COUNT_LIMIT:
            raise EmeiStorBizException(
                ProtectionErrorCodes.ARCHIVE_POLICY_COUNT_OVER_LIMIT, settings.ARCHIVE_POLICY_COUNT_LIMIT)

    @staticmethod
    def _check_archive_policies_name_not_repeat(archive_policies):
        if len(archive_policies) > 1:
            name_list = [policy.name for policy in archive_policies]
            name_set = set(name_list)
            if len(name_list) != len(name_set):
                raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                            ["policy.name"],
                                            "archive policy name repeated")

    @staticmethod
    def _check_archive_policies_storage_id_not_repeat(archive_policies):
        if len(archive_policies) > 1:
            storage_id_list = [policy.ext_parameters.storage_id for policy in archive_policies]
            storage_id_set = set(storage_id_list)
            if len(storage_id_list) != len(storage_id_set):
                raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                            ["policy.ext_parameter.storage_id"],
                                            "archive policy storage_id repeated")
