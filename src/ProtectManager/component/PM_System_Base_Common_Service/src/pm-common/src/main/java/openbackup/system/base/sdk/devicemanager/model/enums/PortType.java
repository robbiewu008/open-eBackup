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
 * 端口类型
 *
 */
public enum PortType {
    /**
     * 以太网口
     */
    ETHERNET("1"),
    /**
     * 绑定端口
     */
    BINDING("7"),

    /**
     * 逻辑端口
     */
    LOGICAL("9"),

    /**
     * sip端口
     */
    SIP("26");

    private static final Map<String, PortType> VALUE_ENUM_MAP = new HashMap<>(PortType.values().length);

    /**
     * 端口类型
     */
    private final String portType;

    static {
        for (PortType each : PortType.values()) {
            VALUE_ENUM_MAP.put(each.getPortType(), each);
        }
    }

    PortType(String portType) {
        this.portType = portType;
    }

    public String getPortType() {
        return this.portType;
    }

    @JsonValue
    @Override
    public String toString() {
        return getPortType();
    }

    /**
     * 和需要开启的（字符串）对比
     *
     * @param otherStoragePoolPortType 被比较端口类型
     * @return 是否相同
     */
    public boolean equalsPortType(String otherStoragePoolPortType) {
        return getPortType().equals(otherStoragePoolPortType);
    }

    /**
     * 创建
     *
     * @param portType 端口类型
     * @return 当前端口类型
     */
    @JsonCreator(mode = JsonCreator.Mode.DELEGATING)
    public static PortType forValues(@JsonProperty("PORTTYPE") String portType) {
        return VALUE_ENUM_MAP.get(portType);
    }
}
