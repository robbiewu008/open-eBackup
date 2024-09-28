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
package openbackup.data.access.framework.livemount.common.enums;

import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonValue;

/**
 * 副本保留策略, 按固定时间的枚举单位定义
 *
 */
public enum RetentionUnit {
    /**
     * 天
     */
    DAY("d"),

    /**
     * 周
     */
    WEEK("w"),

    /**
     * 月
     */
    MONTH("MO"),

    /**
     * 年
     */
    YEAR("y");

    private final String name;

    RetentionUnit(String name) {
        this.name = name;
    }

    @JsonValue
    public String getName() {
        return name;
    }

    /**
     * get retention unit enum
     *
     * @param str str
     * @return RetentionUnit
     */
    public static RetentionUnit get(String str) {
        return EnumUtil.get(RetentionUnit.class, RetentionUnit::getName, str);
    }
}
