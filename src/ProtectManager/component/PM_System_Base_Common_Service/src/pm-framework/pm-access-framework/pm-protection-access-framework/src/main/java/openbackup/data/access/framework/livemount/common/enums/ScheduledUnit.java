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
package openbackup.data.access.framework.livemount.common.enums;

import com.fasterxml.jackson.annotation.JsonValue;

import openbackup.system.base.util.EnumUtil;

/**
 * 策略调度周期, 按间隔时间的枚举单位定义
 *
 */
public enum ScheduledUnit {
    /**
     * 小时
     */
    HOUR("h"),

    /**
     * 天
     */
    DAY("d"),

    /**
     * 周
     */
    WEEK("w");

    private final String name;

    ScheduledUnit(String name) {
        this.name = name;
    }

    @JsonValue
    public String getName() {
        return name;
    }

    /**
     * get schedule unit enum
     *
     * @param str str
     * @return ScheduledUnit
     */
    public static ScheduledUnit get(String str) {
        return EnumUtil.get(ScheduledUnit.class, ScheduledUnit::getName, str);
    }
}
