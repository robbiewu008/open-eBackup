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
package openbackup.system.base.common.enums;

/**
 * 组件名枚举
 *
 * @author w00607005
 * @since 2023-08-04
 */
public enum ComponentEnum {
    GAUSSDB("gaussdb");

    private final String name;

    ComponentEnum(String name) {
        this.name = name;
    }

    /**
     * 获取组件名称
     *
     * @return 组件名
     */
    public String getName() {
        return name;
    }
}
