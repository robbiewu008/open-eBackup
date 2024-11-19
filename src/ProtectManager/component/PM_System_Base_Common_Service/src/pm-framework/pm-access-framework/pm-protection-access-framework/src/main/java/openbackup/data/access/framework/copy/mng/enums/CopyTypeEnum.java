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
package openbackup.data.access.framework.copy.mng.enums;

import lombok.AllArgsConstructor;
import lombok.Getter;
import openbackup.system.base.common.enums.TimeUnitEnum;

import java.util.Arrays;

/**
 * 归档副本类型
 *
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
