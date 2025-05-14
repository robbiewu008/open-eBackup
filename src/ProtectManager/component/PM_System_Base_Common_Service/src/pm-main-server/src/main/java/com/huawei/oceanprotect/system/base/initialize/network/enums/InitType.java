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

/**
 * 初始化类型枚举
 *
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/4/5
 */
public enum InitType {
    /**
     * 手动初始化
     */
    MANUAL_CONFIGURATION("0"),
    /**
     * LLD初始化
     */
    LLD_TEMPLATE_CONFIGURATION("1");

    private final String value;

    InitType(String value) {
        this.value = value;
    }

    /**
     * 获取初始化配置类型是手动还是LLD
     *
     * @return initType
     */
    public String getType() {
        return value;
    }

    /**
     * 是否是手动初始化
     *
     * @return true：是，false：不是
     */
    public boolean isManualInit() {
        return InitType.MANUAL_CONFIGURATION.getType().equals(this.value);
    }

    /**
     * 是否是lld初始化
     *
     * @return true：是，false：不是
     */
    public boolean isLldInit() {
        return InitType.LLD_TEMPLATE_CONFIGURATION.getType().equals(this.value);
    }
}
