/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
