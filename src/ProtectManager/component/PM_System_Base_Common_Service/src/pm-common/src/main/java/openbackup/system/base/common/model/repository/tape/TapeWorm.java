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
