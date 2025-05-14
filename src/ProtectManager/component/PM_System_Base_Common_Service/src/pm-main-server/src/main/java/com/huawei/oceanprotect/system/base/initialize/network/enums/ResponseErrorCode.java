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
import com.fasterxml.jackson.annotation.JsonEnumDefaultValue;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.annotation.JsonValue;

import lombok.AllArgsConstructor;
import lombok.Getter;

import java.util.HashMap;
import java.util.Map;

/**
 * 错误码
 *
 * @author swx1010572
 * @since 2020-12-24
 */
@Getter
@AllArgsConstructor
public enum ResponseErrorCode {
    /**
     * 成功
     */
    SUCCESS(0),

    /**
     * 默认的异常值
     */
    @JsonEnumDefaultValue DEFAULT(Integer.MIN_VALUE);

    /**
     * 枚举常量图
     */
    private static final Map<String, ResponseErrorCode> VALUE_ENUM_MAP = new HashMap<>(
        ResponseErrorCode.values().length);

    static {
        for (ResponseErrorCode each : ResponseErrorCode.values()) {
            VALUE_ENUM_MAP.put(String.valueOf(each.getErrorCode()), each);
        }
    }

    /**
     * 错误码
     */
    private final int errorCode;

    /**
     * 获取值
     *
     * @return 错误码值
     */
    @JsonValue
    public int getErrorCode() {
        return errorCode;
    }

    @Override
    public String toString() {
        return String.valueOf(getErrorCode());
    }

    /**
     * 创建
     *
     * @param errorCode 错误码
     * @return 错误码枚举
     */
    @JsonCreator(mode = JsonCreator.Mode.PROPERTIES)
    public static ResponseErrorCode forValues(@JsonProperty("code") String errorCode) {
        return VALUE_ENUM_MAP.get(errorCode);
    }

    /**
     * 是否成功
     *
     * @return 是否成功
     */
    public boolean isSuccess() {
        return SUCCESS.equals(this);
    }
}