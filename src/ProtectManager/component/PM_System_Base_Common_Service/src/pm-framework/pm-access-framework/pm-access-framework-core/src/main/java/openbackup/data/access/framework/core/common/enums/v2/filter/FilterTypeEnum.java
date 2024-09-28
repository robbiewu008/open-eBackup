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
package openbackup.data.access.framework.core.common.enums.v2.filter;

/**
 * FilterTypeEnum
 *
 * @description: 资源过滤器中的资源类型，用于跟框架层使用
 **/
public enum FilterTypeEnum {
    /**
     * 虚拟机
     */
    VM("VM"),
    /**
     * 磁盘
     */
    DISK("Disk"),
    /**
     * 文件
     */
    FILE("File"),
    /**
     * 目录
     */
    DIR("Dir")
    ;

    FilterTypeEnum(String type) {
        this.type = type;
    }

    private String type;

    public String getType() {
        return type;
    }
}
