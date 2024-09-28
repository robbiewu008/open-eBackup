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
package openbackup.system.base.sdk.protection.emuns;

import lombok.Getter;

import java.util.Arrays;

/**
 * SLA 备份策略类型
 *
 */
@Getter
public enum SlaPolicyTypeEnum {
    /**
     * 备份
     */
    BACKUP("backup"),
    /**
     * 归档
     */
    ARCHIVING("archiving"),
    /**
     * 复制
     */
    REPLICATION("replication");

    /**
     * action名称
     */
    private final String name;

    SlaPolicyTypeEnum(String name) {
        this.name = name;
    }

    /**
     * 根据Type名称获取Type枚举
     *
     * @param name type名称
     * @return SlaPolicyTypeEnum
     */
    public static SlaPolicyTypeEnum getTypeByName(String name) {
        return Arrays.stream(SlaPolicyTypeEnum.values())
                .filter(type -> type.name.equals(name))
                .findFirst()
                .orElseThrow(IllegalArgumentException::new);
    }
}
