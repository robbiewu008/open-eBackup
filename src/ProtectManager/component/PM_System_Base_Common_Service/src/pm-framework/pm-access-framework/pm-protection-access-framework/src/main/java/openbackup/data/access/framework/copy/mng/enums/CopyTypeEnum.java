/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.copy.mng.enums;

import openbackup.system.base.common.enums.TimeUnitEnum;

import lombok.AllArgsConstructor;
import lombok.Getter;

import java.util.Arrays;

/**
 * 归档副本类型
 *
 * @author z30006621
 * @since 2021-09-02
 */
@Getter
@AllArgsConstructor
public enum CopyTypeEnum {
    YEAR("year", 365, TimeUnitEnum.YEARS),

    MONTH("month", 30, TimeUnitEnum.MONTHS),

    WEEK("week", 7, TimeUnitEnum.WEEKS);

    final String copyType;

    final int days;

    final TimeUnitEnum timeUnitEnum;

    /**
     * 获取副本保留时间单位
     *
     * @return copyType
     */
    public String getCopyType() {
        return copyType;
    }

    /**
     * 根据副本类型获取副本类型枚举类
     *
     * @param type 副本类型
     * @return 副本类型枚举类 {@code CopyTypeEnum}
     */
    public static CopyTypeEnum getByType(String type) {
        return Arrays.stream(CopyTypeEnum.values()).filter(copyType -> copyType.copyType.equals(type)).findFirst()
                .orElseThrow(IllegalArgumentException::new);
    }
}
