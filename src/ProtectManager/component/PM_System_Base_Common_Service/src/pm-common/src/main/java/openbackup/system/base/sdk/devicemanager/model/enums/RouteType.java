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
import com.google.common.collect.ImmutableList;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 读写权限
 *
 */
public enum RouteType {
    /**
     * 网段路由
     */
    NETWORK("0"),
    /**
     * 主机路由
     */
    MASTER("1"),

    /**
     * 默认路由
     */
    DEFAULT("2");

    private static final Map<String, RouteType> VALUE_ENUM_MAP = new HashMap<>(RouteType.values().length);

    /**
     * 路由类型
     */
    public static final List<String> ROUTE_TYPES = ImmutableList.of(NETWORK.getRouteType(),
            MASTER.getRouteType(), DEFAULT.getRouteType());

    static {
        for (RouteType each : RouteType.values()) {
            VALUE_ENUM_MAP.put(each.getRouteType(), each);
        }
    }

    /**
     * 路由类型
     */
    private final String routeType;

    RouteType(String routeType) {
        this.routeType = routeType;
    }

    public String getRouteType() {
        return routeType;
    }

    @JsonValue
    @Override
    public String toString() {
        return getRouteType();
    }

    /**
     * 和需要开启的（字符串）对比
     *
     * @param otherStoragePoolRouteType 被比较路由类型
     * @return 是否相同
     */
    public boolean equalsRouteType(String otherStoragePoolRouteType) {
        return getRouteType().equals(otherStoragePoolRouteType);
    }

    /**
     * 创建
     *
     * @param routeType 路由类型
     * @return 当前路由类型
     */
    @JsonCreator(mode = JsonCreator.Mode.DELEGATING)
    public static RouteType forValues(@JsonProperty("TYPE") String routeType) {
        return VALUE_ENUM_MAP.get(routeType);
    }
}
