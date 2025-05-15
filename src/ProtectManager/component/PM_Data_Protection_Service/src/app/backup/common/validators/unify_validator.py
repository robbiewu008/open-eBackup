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
from app.backup.common.validators.business.business_archive_validator import BusinessArchiveValidator
from app.backup.common.validators.business.business_backup_validator import BusinessBackupValidator
from app.backup.common.validators.business.business_log_backup_validator import BusinessLogBackupValidator
from app.backup.common.validators.business.business_policies_validator import BusinessPoliciesValidator
from app.backup.common.validators.business.business_replication_validator import BusinessReplicationValidator
from app.backup.common.validators.config.sla_config import SlaSpecificationConfig
from app.backup.common.validators.specification.specification_archive_validator import SpecificationArchiveValidator
from app.backup.common.validators.specification.specification_backup_validator import SpecificationBackupValidator
from app.backup.common.validators.specification.specification_replication_validator import \
    SpecificationReplicationValidator
from app.common import logger
from app.common.enums.sla_enum import PolicyTypeEnum, PolicyActionEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.protection_error_codes import ProtectionErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException, IllegalParamException


log = logger.get_logger(__name__)


class UnifyValidator:
    def __init__(self, sla_config: SlaSpecificationConfig, external_policies: list):
        self.sla_config = sla_config
        self.external_policies = external_policies

    def validate(self):
        if self.sla_config is None or self.external_policies is None:
            raise EmeiStorBizException(ProtectionErrorCodes.SLA_APPLICATION_NOT_SUPPORT_POLICY_ACTION,
                                       "SLA application not support this policy action.")

        if not self.sla_config.unified_validate:
            return

        self.__check_policy_action_match_support(self.sla_config.support_policies, self.external_policies)
        self.__check_policy_type_match_support(self.sla_config.support_policies, self.external_policies)
        self.__check_at_least_one_full_or_increment_backup(self.sla_config.one_full_or_increment_backup,
                                                           self.external_policies)
        self.__check_difference_cumulative_permanent_co_exist(self.sla_config.difference_cumulative_permanent_co_exist,
                                                              self.external_policies)
        self.__check_action_count_not_greater_than_support(self.sla_config.support_policies, self.external_policies)
        self.__check_specification(self.sla_config.support_policies, self.external_policies)
        self.__check_backup_policies_ext_parameters_must_equal(self.external_policies)
        self.__check_business(self.external_policies, self.sla_config.log_backup_time)

    def __check_policy_action_match_support(self, support_policies, external_policies):
        support_action = [policy.action.action_type for policy in support_policies]
        for external_policy in external_policies:
            if external_policy.action not in support_action:
                raise EmeiStorBizException(ProtectionErrorCodes.SLA_APPLICATION_NOT_SUPPORT_POLICY_ACTION,
                                           external_policy.action)

    def __check_policy_type_match_support(self, support_policies, external_policies):
        support_type = [policy.policy_type for policy in support_policies]
        for external_policy in external_policies:
            if external_policy.type not in support_type:
                raise EmeiStorBizException(ProtectionErrorCodes.SLA_APPLICATION_NOT_SUPPORT_POLICY_ACTION,
                                           external_policy.type)

    def __check_at_least_one_full_or_increment_backup(self, one_full_or_increment_backup, external_policies):
        if not one_full_or_increment_backup:
            return
        backup_actions = (PolicyActionEnum.full,
                          PolicyActionEnum.difference_increment,
                          PolicyActionEnum.cumulative_increment,
                          PolicyActionEnum.permanent_increment,
                          PolicyActionEnum.snapshot)
        backup = [policy for policy in external_policies if policy.action in backup_actions]

        if len(backup) == 0:
            raise IllegalParamException(
                error=CommonErrorCodes.ILLEGAL_PARAMS,
                parameters=["sla.policy"],
                message="backup policy is sla can not be null")

    def __check_difference_cumulative_permanent_co_exist(self, difference_cumulative_permanent_co_exist,
                                                         external_policies):
        if difference_cumulative_permanent_co_exist:
            return
        exceed_one_backup_action = len(set(policy.action for policy in external_policies
                                           if self.__check_policy_action(policy.action)))
        if exceed_one_backup_action > 1:
            raise EmeiStorBizException(ProtectionErrorCodes.SLA_APPLICATION_NOT_SUPPORT_POLICY_ACTION,
                                       "SLA application not support this policy action.")

    def __check_policy_action(self, action):
        return action == PolicyActionEnum.difference_increment or action == PolicyActionEnum.cumulative_increment \
            or action == PolicyActionEnum.permanent_increment

    def __check_action_count_not_greater_than_support(self, support_policies, external_policies):
        external_action_count = {}
        for external_policy in external_policies:
            if external_action_count.get(external_policy.action) is None:
                external_action_count.update({external_policy.action: 1})
            else:
                external_action_count.update(
                    {external_policy.action: external_action_count.get(external_policy.action) + 1})
        for support_policy in support_policies:
            count = external_action_count.get(support_policy.action.action_type)
            if count is not None and count > support_policy.action.count:
                raise EmeiStorBizException(ProtectionErrorCodes.SLA_APPLICATION_NOT_SUPPORT_POLICY_ACTION,
                                           support_policy.action)

    def __check_backup_policies_ext_parameters_must_equal(self, policies):
        backup_policies = [policy for policy in policies
                           if policy.type == PolicyTypeEnum.backup.value]
        # 是否跳过高级参数一致的情况:当前有hbase
        if not self.sla_config.equal_extend_param:
            return
        for policy in backup_policies:
            if policy.ext_parameters != backup_policies[0].ext_parameters:
                raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS, ["policy.ext_parameters"])

    def __check_specification(self, support_policies, external_policies):
        for support_policy in support_policies:
            if support_policy.policy_type == PolicyTypeEnum.backup:
                SpecificationBackupValidator.do_validate(external_policies)
            elif support_policy.policy_type == PolicyTypeEnum.archiving:
                SpecificationArchiveValidator.do_validate(external_policies)
            elif support_policy.policy_type == PolicyTypeEnum.replication:
                SpecificationReplicationValidator.do_validate(external_policies)

    def __check_business(self, external_policies, log_backup_time):
        for external_policy in external_policies:
            BusinessBackupValidator.do_validate(external_policy)
            BusinessArchiveValidator.do_validate(external_policy)
            BusinessReplicationValidator.do_validate(external_policy)
        BusinessPoliciesValidator.do_validate(external_policies)
        BusinessLogBackupValidator.check_log_interval_minutes_unit_greater_or_equal_to_five(external_policies)
        if log_backup_time:
            BusinessLogBackupValidator.check_policy_backup_start_time_before_log(external_policies)
            BusinessLogBackupValidator.check_policy_backup_schedule_interval_greater_than_log(external_policies)
