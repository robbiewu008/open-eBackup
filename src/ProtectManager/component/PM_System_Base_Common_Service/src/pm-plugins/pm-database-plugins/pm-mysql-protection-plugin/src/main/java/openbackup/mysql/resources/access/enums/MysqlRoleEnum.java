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
package openbackup.mysql.resources.access.enums;

/**
 * mysql节点role枚举
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/10/10
 */
public enum MysqlRoleEnum {
    /**
     * MySQL主实例、集群实例
     */
    MASTER("1"),

    /**
     * MySQL备实例，普通实例
     */
    SLAVE("2");

    /**
     * role
     */
    private final String role;

    /**
     * 构造方法
     *
     * @param role role
     */
    MysqlRoleEnum(String role) {
        this.role = role;
    }

    /**
     * get role
     *
     * @return role
     */
    public String getRole() {
        return role;
    }
}
