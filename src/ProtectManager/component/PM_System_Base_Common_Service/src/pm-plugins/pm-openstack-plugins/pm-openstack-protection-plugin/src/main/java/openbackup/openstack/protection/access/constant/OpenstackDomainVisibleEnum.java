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
package openbackup.openstack.protection.access.constant;

/**
 * Openstack 域可见性枚举类
 *
 * @author c30016231
 * @since 2023-03-13
 */
public enum OpenstackDomainVisibleEnum {
    /**
     * 可见
     */
    VISIBLE("1"),
    /**
     * 不可见
     */
    INVISIBLE("0"),
    /**
     * 其他
     */
    OTHER("OTHER");

    private final String code;

    OpenstackDomainVisibleEnum(String code) {
        this.code = code;
    }

    public String getCode() {
        return code;
    }
}
