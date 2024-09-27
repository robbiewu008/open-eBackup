/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.livemount.common.enums;

import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonValue;

/**
 * 副本保留策略, 按固定时间的枚举单位定义
 *
 * @author h30003246
 * @since 2020-09-22
 */
public enum RetentionUnit {
    /**
     * 天
     */
    DAY("d"),

    /**
     * 周
     */
    WEEK("w"),

    /**
     * 月
     */
    MONTH("MO"),

    /**
     * 年
     */
    YEAR("y");

    private final String name;

    RetentionUnit(String name) {
        this.name = name;
    }

    @JsonValue
    public String getName() {
        return name;
    }

    /**
     * get retention unit enum
     *
     * @param str str
     * @return RetentionUnit
     */
    public static RetentionUnit get(String str) {
        return EnumUtil.get(RetentionUnit.class, RetentionUnit::getName, str);
    }
}
