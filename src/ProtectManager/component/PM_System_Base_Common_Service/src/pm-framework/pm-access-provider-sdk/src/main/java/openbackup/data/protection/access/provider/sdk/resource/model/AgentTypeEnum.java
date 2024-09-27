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
package openbackup.data.protection.access.provider.sdk.resource.model;

/**
 * agent类型枚举类
 *
 * @author w00616953
 * @since 2022-03-22
 */
public enum AgentTypeEnum {
    /**
     * 外置代理
     */
    EXTERNAL_AGENT("0"),

    /**
     * 内置代理
     */
    INTERNAL_AGENT("1");

    private final String value;

    AgentTypeEnum(String value) {
        this.value = value;
    }

    /**
     * 获取枚举值
     *
     * @return 枚举值
     */
    public String getValue() {
        return value;
    }
}
