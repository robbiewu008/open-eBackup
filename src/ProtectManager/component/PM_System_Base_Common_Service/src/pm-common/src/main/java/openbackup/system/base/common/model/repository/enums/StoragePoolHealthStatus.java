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
package openbackup.system.base.common.model.repository.enums;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonEnumDefaultValue;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.annotation.JsonValue;

import lombok.AllArgsConstructor;
import lombok.Getter;

import java.util.HashMap;
import java.util.Map;

/**
 * 存储池健康状态
 *
 */
@Getter
@AllArgsConstructor
public enum StoragePoolHealthStatus {
    /**
     * 正常
     */
    NORMAL("1"),

    /**
     * 故障
     */
    @JsonEnumDefaultValue FAULT("2"),

    /**
     * 降级
     */
    DEGRADED("5");

    /**
     * 常量图
     */
    private static final Map<String, StoragePoolHealthStatus> VALUE_ENUM_MAP = new HashMap<>(
        StoragePoolHealthStatus.values().length);

    static {
        for (StoragePoolHealthStatus each : StoragePoolHealthStatus.values()) {
            VALUE_ENUM_MAP.put(each.getHealthStatus(), each);
        }
    }

    /**
     * 存储池健康状态
     */
    private final String healthStatus;

    @JsonValue
    @Override
    public String toString() {
        return getHealthStatus();
    }

    /**
     * 和其他存储池健康状态（字符串）对比
     *
     * @param otherStoragePoolHealthStatus 存储池健康状态
     * @return 是否相同
     */
    public boolean equalsStatus(String otherStoragePoolHealthStatus) {
        return getHealthStatus().equals(otherStoragePoolHealthStatus);
    }

    /**
     * 创建
     *
     * @param healthStatus 存储池健康状态
     * @return 存储池健康状态
     */
    @JsonCreator(mode = JsonCreator.Mode.PROPERTIES)
    public static StoragePoolHealthStatus forValues(@JsonProperty("HEALTHSTATUS") String healthStatus) {
        return VALUE_ENUM_MAP.get(healthStatus);
    }
}