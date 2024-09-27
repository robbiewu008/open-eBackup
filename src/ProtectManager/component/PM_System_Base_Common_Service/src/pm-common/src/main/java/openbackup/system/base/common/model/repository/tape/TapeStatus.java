/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.model.repository.tape;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 磁盘状态Status
 *
 * @author w50021188
 * @since 2021-08-12
 **/
@Getter
@AllArgsConstructor
public enum TapeStatus {
    /**
     * 不在带库中
     */
    NOT_IN_LIBRARY(1),

    /**
     * 在带库中
     */
    IN_LIBRARY(2),

    /**
     * 识别中
     */
    IDENTIFYING(3),

    /**
     * 擦除中
     */
    ERASING(4),

    /**
     * 置空中
     */
    MARKING_EMPTY(5),

    /**
     * 删除中
     */
    DELETING(6),

    /**
     * 导入中
     */
    IMPORTING(7),

    /**
     * 导出中
     */
    EXPORTING(8),

    /**
     * 就绪
     */
    READY(9);

    private final int value;

    /**
     * 通过value获取TapeStatus
     *
     * @param value 值
     * @return 目标告警对象
     */
    public static TapeStatus getTapeStatus(int value) {
        for (TapeStatus tapeStatus : values()) {
            if (value == tapeStatus.getValue()) {
                return tapeStatus;
            }
        }
        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
    }
}
