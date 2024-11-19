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
package com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.annotation.JsonValue;

import java.util.HashMap;
import java.util.Map;

/**
 * 路由表类型
 *
 */
public enum TableType {
    /**
     * 默认路由表和策略路由表
     */
    ALL("1"),
    /**
     * 策略路由表
     */
    STRATEGY("2"),

    /**
     * 默认路由表
     */
    DEFAULT("3");

    private static final Map<String, TableType> VALUE_ENUM_MAP = new HashMap<>(TableType.values().length);


    static {
        for (TableType each : TableType.values()) {
            VALUE_ENUM_MAP.put(each.getTableType(), each);
        }
    }

    /**
     * 路由表类型
     */
    private final String tableType;

    TableType(String tableType) {
        this.tableType = tableType;
    }

    public String getTableType() {
        return this.tableType;
    }

    @JsonValue
    @Override
    public String toString() {
        return getTableType();
    }

    /**
     * 和需要开启的（字符串）对比
     *
     * @param otherStoragePoolTableType 被比较路由表类型
     * @return 是否相同
     */
    public boolean equalsTableType(String otherStoragePoolTableType) {
        return getTableType().equals(otherStoragePoolTableType);
    }

    /**
     * 创建
     *
     * @param tableType 路由表类型
     * @return 当前路由表类型
     */
    @JsonCreator(mode = JsonCreator.Mode.DELEGATING)
    public static TableType forValues(@JsonProperty("TABLETYPE") String tableType) {
        return VALUE_ENUM_MAP.get(tableType);
    }
}
