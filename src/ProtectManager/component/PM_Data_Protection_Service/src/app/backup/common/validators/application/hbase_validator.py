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
from app.backup.common.validators.validator import Validator
from app.common.config import settings
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.sla_enum import PolicyTypeEnum, PolicyActionEnum, RetentionTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException


class HbaseValidator(Validator):
    """对Hbase的策略进行校验

        Hbase支持日志备份、
        支持且仅支持一个全量备份（同时至少需要有一个全量备份）
        支持增量备份，但只能有一个
        支持归档和复制策略
    """

    @staticmethod
    def is_support(sub_type) -> bool:
        return sub_type == ResourceSubTypeEnum.HBaseBackupSet

    @staticmethod
    def do_validate(policies):
        sla_config = SlaSpecificationConfig.Builder() \
            .config(PolicyTypeEnum.backup, PolicyActionEnum.full,
                    settings.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT) \
            .config(PolicyTypeEnum.backup, PolicyActionEnum.difference_increment,
                    settings.DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT) \
            .config(PolicyTypeEnum.backup, PolicyActionEnum.log,
                    settings.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT) \
            .config(PolicyTypeEnum.archiving, PolicyActionEnum.archiving,
                    settings.ARCHIVE_POLICY_COUNT_LIMIT) \
            .config(PolicyTypeEnum.replication, PolicyActionEnum.replication,
                    settings.REPLICATION_POLICY_COUNT_LIMIT)\
            .check_log_time(True)\
            .check_backup_policy_extend_param_equal(False)\
            .build()
        HbaseValidator.check_retention(policies)
        UnifyValidator(sla_config, policies).validate()

    @staticmethod
    def check_retention(policies):
        for policy in policies:
            if policy.action == PolicyActionEnum.log and \
                    policy.retention.retention_type != RetentionTypeEnum.permanent:
                raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                       error_message="retention type is not permanent")
