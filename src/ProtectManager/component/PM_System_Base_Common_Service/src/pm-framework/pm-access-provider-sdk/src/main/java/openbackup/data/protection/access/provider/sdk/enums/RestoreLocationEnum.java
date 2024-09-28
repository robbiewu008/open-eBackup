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
package openbackup.data.protection.access.provider.sdk.enums;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

import java.util.Arrays;

/**
 * 恢复目标类型枚举定义
 *
 */
public enum RestoreLocationEnum {
    /**
     * 新位置恢复
     */
    NEW("new"),
    /**
     * 原位置恢复
     */
    ORIGINAL("original"),
    /**
     * 本机恢复
     */
    NATIVE("native");

    private final String location;

    RestoreLocationEnum(String location) {
        this.location = location;
    }

    @JsonValue
    public String getLocation() {
        return location;
    }

    /**
     * 根据恢复位置获取恢复位置枚举对象
     *
     * @param location 恢复类型
     * @return 恢复任务类型 {@code RestoreLocationEnum}
     */
    @JsonCreator
    public static RestoreLocationEnum getByLocation(String location) {
        return Arrays.stream(RestoreLocationEnum.values())
            .filter(restoreLocation -> restoreLocation.location.equals(location))
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }
}
