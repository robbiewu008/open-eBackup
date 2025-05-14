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
import com.fasterxml.jackson.annotation.JsonValue;

import lombok.AllArgsConstructor;
import lombok.Getter;

import java.util.HashMap;
import java.util.Map;

/**
 * 爱数安装语言的枚举值
 *
 * @since 2021-03-26
 */
@Getter
@AllArgsConstructor
public enum InstallationLanguageType {
    /**
     * 当前没有安装语言
     */
    NULL_LANGUAGE(""),

    /**
     * 安装语言为中文
     */
    CHINA_LANGUAGE("zh-cn"),

    /**
     * 安装语言为英文
     */
    ENGLISH_LANGUAGE("en-us");

    private static final Map<String, InstallationLanguageType> VALUE_ENUM_MAP = new HashMap<>(
        InstallationLanguageType.values().length);

    static {
        for (InstallationLanguageType each : InstallationLanguageType.values()) {
            VALUE_ENUM_MAP.put(each.getValue(), each);
        }
    }

    /**
     * 策略方式
     */
    private final String value;

    @JsonValue
    @Override
    public String toString() {
        return getValue();
    }

    /**
     * 和需要开启的（字符串）对比
     *
     * @param otherInstallationLanguageType 被比较分配方式
     * @return 是否相同
     */
    public boolean equalsPolicy(String otherInstallationLanguageType) {
        return getValue().equals(otherInstallationLanguageType);
    }

    /**
     * 创建
     *
     * @param installationLanguageType 选择策略
     * @return 当前策略选择
     */
    @JsonCreator(mode = JsonCreator.Mode.DELEGATING)
    public static InstallationLanguageType forValues(
        @JsonProperty("INITIALDISTRIBUTEPOLICY") String installationLanguageType) {
        return VALUE_ENUM_MAP.get(installationLanguageType);
    }
}
