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
package openbackup.system.base.sdk.lock;

import com.fasterxml.jackson.annotation.JsonValue;

import java.util.Arrays;
import java.util.Objects;

/**
 * 资源锁类型
 *
 **/
public enum LockTypeEnum {
    READ("r"),
    WRITE("w");

    private final String type;

    LockTypeEnum(String type) {
        this.type = type;
    }

    /**
     * 获取资源锁类型
     *
     * @return 资源锁类型
     */
    @JsonValue
    public String getType() {
        return type;
    }

    /**
     * 根据锁类型获取锁类型枚举
     *
     * @param type 锁类型
     * @return LockTypeEnum
     */
    public static LockTypeEnum getByType(String type) {
        return Arrays.stream(LockTypeEnum.values())
            .filter(item -> Objects.equals(item.type, type))
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }
}
