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
 * 保留策略
 *
 * @author z30006621
 * @since 2021-08-24
 */
@Getter
@AllArgsConstructor
public enum TapeRetentionType {
    /**
     * 立刻过期
     */
    IMMEDIATE(1),

    /**
     * 永不过期
     */
    PERMANENT(2),

    /**
     * 临时
     */
    TEMPORARY(3);

    private final int value;

    /**
     * 通过value获取RetentionTypeEnum
     *
     * @param value 值
     * @return 保留类型
     */
    public static TapeRetentionType getRetentionType(int value) {
        for (TapeRetentionType tapeRetentionType : values()) {
            if (value == tapeRetentionType.getValue()) {
                return tapeRetentionType;
            }
        }
        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
    }
}
