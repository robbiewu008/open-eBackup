/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.model.repository.tape;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 保留时长单位
 *
 * @author z30006621
 * @since 2021-08-24
 */
@Getter
@AllArgsConstructor
public enum TapeTimeUnit {
    INVALID("INVALID", 0),

    DAY("DAY", 1),

    MONTH("MONTH", 2),

    YEAR("YEAR", 3);

    private final String value;

    private final int dmeKey;

    @JsonValue
    public String getValue() {
        return value;
    }

    /**
     * 通过value获取RetentionTypeEnum
     *
     * @param value 值
     * @return 保留日期单位
     */
    @JsonCreator(mode = JsonCreator.Mode.DELEGATING)
    public static TapeTimeUnit getTimeUnit(String value) {
        if (value == null) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
        }
        for (TapeTimeUnit tapeTimeUnit : values()) {
            if (value.equals(tapeTimeUnit.getValue())) {
                return tapeTimeUnit;
            }
        }
        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
    }

    /**
     * 通过dmeKey获取TapeTimeUnitEnum
     *
     * @param dmeKey 值
     * @return 保留日期单位
     */
    public static TapeTimeUnit getTimeUnit(int dmeKey) {
        for (TapeTimeUnit tapeTimeUnit : values()) {
            if (dmeKey == tapeTimeUnit.getDmeKey()) {
                return tapeTimeUnit;
            }
        }
        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
    }
}
