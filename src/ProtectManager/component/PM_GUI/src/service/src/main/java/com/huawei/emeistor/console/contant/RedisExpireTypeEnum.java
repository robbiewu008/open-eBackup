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
package com.huawei.emeistor.console.contant;

import com.huawei.emeistor.console.exception.LegoCheckedException;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

/**
 * 手动监听redis过期类型
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/5/24
 */
public enum RedisExpireTypeEnum {
    SESSION_TYPE("0"),
    AUTO_UNLOCK_TYPE("1");
    private final String type;

    RedisExpireTypeEnum(String type) {
        this.type = type;
    }

    @JsonValue
    public String getType() {
        return type;
    }

    @Override
    public String toString() {
        return getType();
    }

    /**
     * 获取redis键过期类型对应的枚举
     *
     * @param type redis键过期类型
     * @return redis键过期类型枚举
     */
    @JsonCreator(mode = JsonCreator.Mode.DELEGATING)
    public static RedisExpireTypeEnum forValues(String type) {
        for (RedisExpireTypeEnum v : RedisExpireTypeEnum.values()) {
            if (v.getType().equals(type)) {
                return v;
            }
        }
        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Illegal enumeration types.");
    }
}
