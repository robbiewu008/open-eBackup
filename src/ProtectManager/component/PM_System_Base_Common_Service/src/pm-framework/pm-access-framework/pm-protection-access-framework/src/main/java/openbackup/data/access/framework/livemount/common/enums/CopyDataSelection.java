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

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

/**
 * 副本选择策略枚举定义
 *
 */
public enum CopyDataSelection {
    /**
     * 最近一小时
     */
    LAST_HOUR("last_hour", "h"),

    /**
     * 最近一天
     */
    LAST_DAY("last_day", "d"),

    /**
     * 最近一周
     */
    LAST_WEEK("last_week", "w"),

    /**
     * 最近一月
     */
    LAST_MONTH("last_month", "MO"),

    /**
     * 总是最新
     */
    LATEST("latest", null);

    private String name;

    private String unit;

    CopyDataSelection(String name, String unit) {
        this.name = name;
        this.unit = unit;
    }

    @JsonValue
    public String getName() {
        return name;
    }

    public String getUnit() {
        return unit;
    }

    /**
     * get copy data selection unit enum
     *
     * @param str str
     * @return CopyDataSelection
     */
    @JsonCreator
    public static CopyDataSelection get(String str) {
        for (CopyDataSelection typeEnum : CopyDataSelection.values()) {
            if (typeEnum.getName().equals(str)) {
                return typeEnum;
            }
        }
        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
    }
}
