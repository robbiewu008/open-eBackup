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
package com.huawei.oceanprotect.system.base.initialize.network.enums;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.collect.ImmutableList;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 配置平面网络类型枚举类
 *
 */
public enum NetPlaneType {
    /**
     * 备份
     */
    BACKUP("1"),
    /**
     * 归档
     */
    ARCHIVE("3"),
    /**
     * 复制
     */
    COPY("2");

    private static final Map<String, NetPlaneType> VALUE_ENUM_MAP = new HashMap<>(NetPlaneType.values().length);

    /**
     * 网络平面类型
     */
    public static final List<String> NET_PLANE_TYPES = ImmutableList.of(BACKUP.getType(),
            ARCHIVE.getType(), COPY.getType());

    private final String value;

    static {
        for (NetPlaneType each : NetPlaneType.values()) {
            VALUE_ENUM_MAP.put(each.getType(), each);
        }
    }

    /**
     * 默认构造函数
     *
     * @param type 类型
     */
    NetPlaneType(String type) {
        value = type;
    }

    /**
     * 获取IP类型时ipv4还是ipv6
     *
     * @return ipType
     */
    public String getType() {
        return value;
    }

    /**
     * 创建
     *
     * @param netPlaneType 网络平面类型值
     * @return 对应网络平面类型
     */
    @JsonCreator(mode = JsonCreator.Mode.DELEGATING)
    public static NetPlaneType forValues(@JsonProperty("NETPLANETYPE") String netPlaneType) {
        return VALUE_ENUM_MAP.get(netPlaneType);
    }
}
