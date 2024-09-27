/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.model.repository.tape;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 磁带驱动器压缩状态
 *
 * @author g30003063
 * @since 2021-08-10
 */
@Getter
@AllArgsConstructor
public enum TapeDriveCompressionStatus {
    /**
     * 禁用的
     */
    DISABLE(1),

    /**
     * 启用的
     */
    ENABLE(2);

    private final int value;

    /**
     * 通过value获取TapeLibraryStatus
     *
     * @param value 值
     * @return 目标告警对象
     */
    public static TapeDriveCompressionStatus getTapeLibraryStatus(int value) {
        for (TapeDriveCompressionStatus tapeDriveCompressionStatus : values()) {
            if (value == tapeDriveCompressionStatus.getValue()) {
                return tapeDriveCompressionStatus;
            }
        }
        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
    }
}
