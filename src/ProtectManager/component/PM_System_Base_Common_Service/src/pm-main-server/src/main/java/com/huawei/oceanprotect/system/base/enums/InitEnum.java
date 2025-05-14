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
package com.huawei.oceanprotect.system.base.enums;

import lombok.Getter;

/**
 * 初始化相关的枚举
 *
 */
public class InitEnum {
    /**
     * E6000业务网络类型枚举
     */
    @Getter
    public enum E6000NetworkTypeEnum {
        /**
         * 备份
         */
        BACKUP("1"),
        /**
         * 归档
         */
        ARCHIVE("2");

        private final String value;

        E6000NetworkTypeEnum(String value) {
            this.value = value;
        }
    }
}
