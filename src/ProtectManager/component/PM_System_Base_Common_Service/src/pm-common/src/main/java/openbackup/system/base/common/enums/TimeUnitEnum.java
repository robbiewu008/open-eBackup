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
package openbackup.system.base.common.enums;

import lombok.Getter;

import java.util.Arrays;

/**
 * 周期类型时间枚举定义
 *
 **/
@Getter
public enum TimeUnitEnum {
    MINUTES("m"),
    HOURS("h"),
    DAYS("d"),
    WEEKS("w"),
    MONTHS("MO"),
    YEARS("y");

    private final String unit;

    TimeUnitEnum(String unit) {
        this.unit = unit;
    }

    /**
     * 根据时间单位缩写获取对应枚举类
     *
     * @param unit 时间单位缩写
     * @return TimeUnitEnum
     */
    public static TimeUnitEnum getByUnit(String unit) {
        return Arrays.stream(TimeUnitEnum.values())
            .filter(timeUnit -> timeUnit.unit.equals(unit))
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }
}
