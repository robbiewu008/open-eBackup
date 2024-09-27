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
