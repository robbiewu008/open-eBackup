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
package openbackup.system.base.sdk.resource.enums;

/**
 * 反向注册时前端显示agent操作系统类型和environments表中osType字段值
 *
 */
public enum EnvironmentsOsTypeEnum {
    /**
     * Linux
     */
    LINUX("linux"),

    /**
     * Linux_AIX
     */
    AIX("aix"),

    /**
     * Linux_SOLARIS
     */
    SOLARIS("solaris");

    private final String osType;

    EnvironmentsOsTypeEnum(String osType) {
        this.osType = osType;
    }

    /**
     * 操作系统
     *
     * @return osType
     */
    public String getOsType() {
        return osType;
    }
}