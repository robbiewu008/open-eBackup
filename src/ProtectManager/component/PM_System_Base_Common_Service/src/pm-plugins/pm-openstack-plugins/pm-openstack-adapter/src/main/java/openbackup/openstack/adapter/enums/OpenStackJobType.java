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
 * 云核OpenStack备份对象
 *
 */
public enum OpenStackJobType {
    /**
     * 虚拟机备份
     */
    SERVER("server"),

    /**
     * 卷备份
     */
    VOLUME("volume");

    private final String type;

    OpenStackJobType(String type) {
        this.type = type;
    }

    @JsonValue
    public String getType() {
        return type;
    }

    /**
     * 通过type获取BackupJobType枚举实例
     *
     * @param type type
     * @return OpenStackJobType
     */
    @JsonCreator
    public static OpenStackJobType create(String type) {
        return Stream.of(OpenStackJobType.values())
                .filter(openStackJobType -> openStackJobType.type.equals(type))
                .findFirst()
                .orElseThrow(() -> new IllegalArgumentException("Backup type is illegal."));
    }
}
