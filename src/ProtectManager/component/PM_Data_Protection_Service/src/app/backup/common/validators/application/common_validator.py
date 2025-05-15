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
from app.common.enums.sla_enum import PolicyActionEnum, PolicyTypeEnum

from app.backup.common.validators.validator import Validator


class CommonValidator(Validator):
    """ 对common的策略进行校验

    common不支持差异和日志备份
          不支持归档和复制策略
          支持且仅支持一个全量备份和增量备份，至少要有一个全量备份
    """

    @staticmethod
    def is_support(sub_type) -> bool:
        return sub_type == ResourceSubTypeEnum.Common

    @staticmethod
    def do_validate(policies):
        sla_config = SlaSpecificationConfig.Builder()\
            .config(PolicyTypeEnum.backup, PolicyActionEnum.full, settings.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT)\
            .config(PolicyTypeEnum.backup, PolicyActionEnum.difference_increment,
                    settings.DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT)\
            .build()
        UnifyValidator(sla_config, policies).validate()
