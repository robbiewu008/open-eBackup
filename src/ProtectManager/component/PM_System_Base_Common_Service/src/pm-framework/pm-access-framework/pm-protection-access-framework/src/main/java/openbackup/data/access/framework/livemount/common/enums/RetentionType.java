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

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

/**
 * 副本保留策略枚举定义
 *
 * @author h30003246
 * @since 2020-09-22
 */
public enum RetentionType {
    /**
     * 永久保留
     */
    PERMANENT("permanent"),

    /**
     * 固定时间
     */
    FIXED_TIME("fixed_time"),

    /**
     * 最后一个
     */
    LATEST_ONE("latest_one");

    private final String name;

    RetentionType(String name) {
        this.name = name;
    }

    @JsonValue
    public String getName() {
        return name;
    }

    /**
     * get retention type enum
     *
     * @param str str
     * @return RetentionUnit
     */
    @JsonCreator
    public static RetentionType get(String str) {
        return EnumUtil.get(RetentionType.class, RetentionType::getName, str);
    }
}
