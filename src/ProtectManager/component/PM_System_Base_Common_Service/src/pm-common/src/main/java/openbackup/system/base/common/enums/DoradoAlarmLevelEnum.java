/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.system.base.common.enums;

import lombok.Getter;

/**
 * Dorado告警枚举
 *
 */
@Getter
public enum DoradoAlarmLevelEnum {
    /**
     * 提示
     */
    INFO(2),
    /**
     * 警告
     */
    WARNING(3),
    /**
     * 重要
     */
    MAJOR(5),
    /**
     * 紧急
     */
    CRITICAL(6);

    private final Integer alarmLevel;

    DoradoAlarmLevelEnum(Integer alarmLevel) {
        this.alarmLevel = alarmLevel;
    }

    /**
     * 获取告警级别String
     *
     * @param value value
     * @return 枚举值
     */
    public static DoradoAlarmLevelEnum getSeverity(int value) {
        for (DoradoAlarmLevelEnum severity : DoradoAlarmLevelEnum.values()) {
            if (value == severity.alarmLevel) {
                return severity;
            }
        }

        return DoradoAlarmLevelEnum.INFO;
    }
}
