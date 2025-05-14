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

from app.common.config import settings
from app.common.enums.sla_enum import PolicyTypeEnum, PolicyActionEnum


class ActionConfig(BaseModel):
    action_type: PolicyActionEnum
    count: int = Field(description="SLA支持的action数量", gt=0)


class PolicyConfig(BaseModel):
    policy_type: PolicyTypeEnum
    action: ActionConfig


class SlaSpecificationConfig:

    def __init__(self, builder):
        self.__support_policies = builder.support_policies
        self.__unified_validate = builder.unified_validate
        self.__log_backup_time = builder.log_backup_time
        self.__equal_extend_param = builder.equal_extend_param
        self.__one_full_or_increment_backup = builder.one_full_or_increment_backup
        self.__difference_cumulative_permanent_co_exist = builder.difference_cumulative_permanent_co_exist

    @property
    def support_policies(self):
        return self.__support_policies

    @property
    def unified_validate(self):
        return self.__unified_validate

    @property
    def log_backup_time(self):
        return self.__log_backup_time

    @property
    def one_full_or_increment_backup(self):
        return self.__one_full_or_increment_backup

    @property
    def equal_extend_param(self) -> bool:
        return self.__equal_extend_param

    @property
    def difference_cumulative_permanent_co_exist(self):
        return self.__difference_cumulative_permanent_co_exist

    class Builder:
        def __init__(self):
            self.support_policies = []
            self.one_full_or_increment_backup = True
            self.unified_validate = True
            self.log_backup_time = False
            self.equal_extend_param = True
            self.difference_cumulative_permanent_co_exist = False

        def config(self, policy_type: PolicyTypeEnum, action_type: PolicyActionEnum, action_count: int):
            action = ActionConfig(action_type=action_type, count=action_count)
            policy = PolicyConfig(policy_type=policy_type, action=action)
            self.support_policies.append(policy)
            return self

        def unify_validate(self, unified_validate: bool):
            self.unified_validate = unified_validate
            return self

        def check_log_time(self, log_backup_time):
            self.log_backup_time = log_backup_time
            return self

        def check_backup_policy_extend_param_equal(self, equal_extend_param):
            self.equal_extend_param = equal_extend_param
            return self

        def require_one_full_or_increment_backup(self, one_full_or_increment_backup):
            self.one_full_or_increment_backup = one_full_or_increment_backup
            return self

        def require_difference_cumulative_permanent_co_exist(self, difference_cumulative_permanent_co_exist):
            self.difference_cumulative_permanent_co_exist = difference_cumulative_permanent_co_exist
            return self

        def build(self):
            return SlaSpecificationConfig(self)

        def default(self):
            full_action = ActionConfig(action_type=PolicyActionEnum.full,
                                       count=settings.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT)
            full_backup = PolicyConfig(policy_type=PolicyTypeEnum.backup, action=full_action)

            difference_action = ActionConfig(action_type=PolicyActionEnum.difference_increment,
                                             count=settings.DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT)
            difference_backup = PolicyConfig(policy_type=PolicyTypeEnum.backup, action=difference_action)

            replication_action = ActionConfig(action_type=PolicyActionEnum.replication,
                                              count=settings.REPLICATION_POLICY_COUNT_LIMIT)
            replication = PolicyConfig(policy_type=PolicyTypeEnum.replication, action=replication_action)

            self.support_policies = [full_backup, difference_backup, replication]
            return SlaSpecificationConfig(self)
