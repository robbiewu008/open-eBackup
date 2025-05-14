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
from app.common.enums.sla_enum import PolicyTypeEnum, PolicyActionEnum


class ReplicaCopyValidator(Validator):
    """ 对Replica的策略进行校验

    Replica只支持归档策略
    """

    @staticmethod
    def is_support(sub_type) -> bool:
        return sub_type == ResourceSubTypeEnum.Replica

    @staticmethod
    def do_validate(policies):
        sla_config = SlaSpecificationConfig.Builder()\
            .config(PolicyTypeEnum.archiving, PolicyActionEnum.archiving, settings.ARCHIVE_POLICY_COUNT_LIMIT)\
            .require_one_full_or_increment_backup(False).build()
        UnifyValidator(sla_config, policies).validate()
