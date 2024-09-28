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
package openbackup.database.base.plugin.enums;

/**
 * 启用禁用枚举类
 *
 */
public enum LockedValueEnum {
    /**
     * 置灰
     */
    OPTIONAL("true"),

    /**
     * 不置灰
     */
    NO_OPTIONAL("false");

    private final String locked;

    /**
     * 构造方法
     *
     * @param locked 节点类型
     */
    LockedValueEnum(String locked) {
        this.locked = locked;
    }

    /**
     * 获取具体值方法
     *
     * @return 具体值
     */
    public String getLocked() {
        return locked;
    }
}
