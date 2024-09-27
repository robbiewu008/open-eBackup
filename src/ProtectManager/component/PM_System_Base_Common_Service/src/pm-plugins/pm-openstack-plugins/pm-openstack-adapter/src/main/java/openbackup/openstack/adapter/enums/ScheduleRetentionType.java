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
package openbackup.openstack.adapter.enums;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

import java.util.stream.Stream;

/**
 * 云核OpenStack备份副本保留策略类型
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-05
 */
public enum ScheduleRetentionType {
    /**
     * 按数量保留
     */
    NUMBER("number"),

    /**
     * 按时间保留
     */
    TIME("time");

    private final String type;

    ScheduleRetentionType(String type) {
        this.type = type;
    }

    @JsonValue
    public String getType() {
        return type;
    }

    /**
     * 根据type获取ScheduleRetentionType枚举实例
     *
     * @param type type
     * @return ScheduleRetentionType
     */
    @JsonCreator
    public static ScheduleRetentionType create(String type) {
        return Stream.of(ScheduleRetentionType.values())
                .filter(retentionType -> retentionType.type.equals(type))
                .findFirst()
                .orElseThrow(() -> new IllegalArgumentException("Retention type is illegal."));
    }
}
