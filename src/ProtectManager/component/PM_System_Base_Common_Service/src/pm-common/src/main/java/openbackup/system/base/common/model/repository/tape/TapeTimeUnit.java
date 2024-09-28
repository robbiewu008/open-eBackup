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

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 保留时长单位
 *
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
