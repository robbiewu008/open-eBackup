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
 * SLA 备份策略action
 *
 * @author y00559272
 * @version [A8000 1.0.0]
 * @since 2021-01-03
 */
@Getter
public enum SlaPolicyActionEnum {
    /**
     * 全量备份
     */
    FULL("full"),
    /**
     * 日志备份
     */
    LOG("log"),
    /**
     * 累积增量
     */
    CUMULATIVE_INCREMENT("cumulative_increment"),
    /**
     * 差异增量
     */
    DIFFERENCE_INCREMENT("difference_increment"),
    /**
     * 永久增量
     */
    PERMANENT_INCREMENT("permanent_increment");

    /**
     * action名称
     */
    private final String name;

    SlaPolicyActionEnum(String name) {
        this.name = name;
    }

    /**
     * 根据Action名称获取Action枚举
     *
     * @param name action名称
     * @return SlaPolicyActionEnum
     */
    public static SlaPolicyActionEnum getActionByName(String name) {
        return Arrays.stream(SlaPolicyActionEnum.values())
                .filter(action -> action.name.equals(name))
                .findFirst()
                .orElseThrow(IllegalArgumentException::new);
    }
}
