/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.model.repository.tape;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import com.google.common.collect.ImmutableSet;

import lombok.AllArgsConstructor;
import lombok.Getter;

import java.util.Set;

/**
 * 磁带写状态
 *
 * @author w50021188
 * @since 2021-08-11
 **/
@Getter
@AllArgsConstructor
public enum TapeWriteStatus {
    /**
     * 未知
     */
    UNKNOWN(1),

    /**
     * 空
     */
    EMPTY(2),

    /**
     * 已写
     */
    WRITTEN(3),

    /**
     * 已满
     */
    FULL(4),

    /**
     * 错误
     */
    ERROR(5);

    /**
     * 可以合法操作的状态
     */
    public static final Set<TapeWriteStatus> WORM_LEGAL_ADD_OPERATION_WRITE_STATUS_SET =
            ImmutableSet.of(EMPTY, WRITTEN, FULL, UNKNOWN);

    /**
     * RW类型可以合法操作的状态
     */
    public static final Set<TapeWriteStatus> RW_LEGAL_ADD_OPERATION_WRITE_STATUS_SET =
            ImmutableSet.of(EMPTY, WRITTEN, FULL);

    /**
     * 可以移除的状态
     */
    public static final Set<TapeWriteStatus> LEGAL_DELETE_WRITE_STATUS_SET = ImmutableSet.of(EMPTY, UNKNOWN, ERROR);

    /**
     * 可用磁带写状态
     */
    public static final Set<TapeWriteStatus> AVAILABLE_WRITE_STATUS_SET = ImmutableSet.of(EMPTY, WRITTEN);

    private final int value;

    /**
     * 通过value获取TapeWriteStatus
     *
     * @param value 值
     * @return 目标告警对象
     */
    public static TapeWriteStatus getTapeWriteStatus(int value) {
        for (TapeWriteStatus tapeWriteStatus : values()) {
            if (value == tapeWriteStatus.getValue()) {
                return tapeWriteStatus;
            }
        }
        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
    }
}
