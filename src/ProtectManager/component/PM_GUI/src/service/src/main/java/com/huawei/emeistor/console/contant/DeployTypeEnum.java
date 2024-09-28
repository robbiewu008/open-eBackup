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

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 部署场景枚举
 *
 */
@Getter
@AllArgsConstructor
public enum DeployTypeEnum {
    /**
     * A8000
     */
    A8000("a8000"),

    /**
     * CLOUD_BACKUP_OLD
     */
    CLOUD_BACKUP_OLD("cloudbackup"),

    /**
     * X8000
     */
    X8000("d0"),

    /**
     * X6000
     */
    X6000("d1"),

    /**
     * X3000
     */
    X3000("d2"),

    /**
     * CLOUD_BACKUP
     */
    CLOUD_BACKUP("d3"),

    /**
     * HYPER_DETECT
     */
    HYPER_DETECT("d4"),

    /**
     * OCEAN_CYBER
     */
    OCEAN_CYBER("d5"),

    /**
     * X9000
     */
    X9000("d6"),

    /**
     * E6000
     */
    E6000("d7"),

    /**
     * E1000
     */
    E1000("d8");

    private final String value;

    /**
     * 根据value获取部署类型枚举
     *
     * @param value value
     * @return deployType
     */
    public static DeployTypeEnum getByValue(String value) {
        for (DeployTypeEnum deployTypeEnum : DeployTypeEnum.values()) {
            if (deployTypeEnum.value.equals(value)) {
                return deployTypeEnum;
            }
        }
        throw new IllegalArgumentException("Illegal deploy type value.");
    }
}
