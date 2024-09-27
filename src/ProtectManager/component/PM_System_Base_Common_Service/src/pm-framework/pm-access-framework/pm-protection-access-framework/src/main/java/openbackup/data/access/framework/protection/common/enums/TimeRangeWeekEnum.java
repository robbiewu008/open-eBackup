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
package openbackup.data.access.framework.protection.common.enums;

import lombok.Getter;

import java.util.Arrays;

/**
 * 周枚举
 *
 * @since 2023-02-24
 */
@Getter
public enum TimeRangeWeekEnum {
    SUN(1, "sun"),
    MON(2, "mon"),
    TUE(3, "tue"),
    WED(4, "wed"),
    THU(5, "thu"),
    FRI(6, "fri"),
    SAT(7, "sat");

    private final Integer dayOfWeek;

    private final String value;

    TimeRangeWeekEnum(Integer dayOfWeek, String value) {
        this.dayOfWeek = dayOfWeek;
        this.value = value;
    }

    /**
     * 根据日期是所在周的第几天获取对应的枚举
     *
     * @param dayOfWeek 所在周的第几天
     * @return TimeRangeWeekEnum
     */
    public static TimeRangeWeekEnum getByDayOfWeek(int dayOfWeek) {
        return Arrays.stream(TimeRangeWeekEnum.values())
            .filter(timeRangeWeek -> timeRangeWeek.dayOfWeek.equals(dayOfWeek))
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }
}
