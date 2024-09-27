/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.model.repository.tape;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 磁带类型
 *
 * @author w50021188
 * @since 2021-08-11
 **/
@Getter
@AllArgsConstructor
public enum TapeWorm {
    /**
     * Unknown
     */
    UNKNOWN(1),

    /**
     * 一次写一次读
     */
    RW(2),

    /**
     * 一次写多次读
     */
    WORM(3);

    private final int value;

    /**
     * 通过value获取TapeWorm
     *
     * @param value 值
     * @return 目标告警对象
     */
    public static TapeWorm getTapeWorm(int value) {
        for (TapeWorm tapeWorm : values()) {
            if (value == tapeWorm.getValue()) {
                return tapeWorm;
            }
        }
        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
    }
}
