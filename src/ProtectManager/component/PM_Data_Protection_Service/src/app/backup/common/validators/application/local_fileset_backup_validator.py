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
from app.backup.common.validators.config.sla_config import SlaSpecificationConfig
from app.backup.common.validators.unify_validator import UnifyValidator
from app.common.config import settings
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.sla_enum import PolicyTypeEnum, PolicyActionEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import IllegalParamException
from app.backup.common.validators.validator import Validator


class LocalFileSystemValidator(Validator):

    @staticmethod
    def is_support(sub_type) -> bool:
        return sub_type == ResourceSubTypeEnum.CloudBackupFileSystem.value

    @staticmethod
    def do_validate(policies):
        # LocalFileSystem 除去快照备份的多个备份策略不支持向同一个云上备份
        storage_id_list = list(policy.ext_parameters.storage_id
                               for policy in policies
                               if policy.action != PolicyActionEnum.snapshot
                               )
        if len(storage_id_list) != len(set(storage_id_list)):
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                        ["storage_id is None or Duplicate field"])

        sla_config = SlaSpecificationConfig.Builder() \
            .config(PolicyTypeEnum.backup, PolicyActionEnum.difference_increment,
                    settings.DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT) \
            .config(PolicyTypeEnum.backup, PolicyActionEnum.snapshot,
                    settings.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT) \
            .check_backup_policy_extend_param_equal(False) \
            .build()
        UnifyValidator(sla_config, policies).validate()
