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
from app.common.enums.sla_enum import RetentionTypeEnum, RetentionTimeUnit, retention_time_limits


class CyberEngineValidator:

    @staticmethod
    def validate_backup_req(backup_req):
        retention_type = backup_req.get("retention_type", None)
        retention_duration = backup_req.get("retention_duration", None)
        duration_unit = backup_req.get("duration_unit", None)
        if retention_type == RetentionTypeEnum.permanent:
            return True
        if retention_type == RetentionTypeEnum.temporary:
            if (duration_unit is None) or (retention_duration is None):
                return False
            if (duration_unit in RetentionTimeUnit.__members__.values()) and (
                    1 <= retention_duration <= retention_time_limits[duration_unit]):
                return True
        return False
