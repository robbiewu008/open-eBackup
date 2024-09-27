/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.livemount.common.enums;

import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonValue;

/**
 * 策略调度周期, 按间隔时间的枚举单位定义
 *
 * @author h30003246
 * @since 2020-09-22
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
