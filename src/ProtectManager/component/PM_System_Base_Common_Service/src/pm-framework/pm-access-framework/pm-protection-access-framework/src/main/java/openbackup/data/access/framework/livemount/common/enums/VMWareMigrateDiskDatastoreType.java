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
package openbackup.data.access.framework.livemount.common.enums;

import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonValue;

/**
 * VMWare 迁移磁盘配置dataStore枚举
 *
 */
public enum VMWareMigrateDiskDatastoreType {
    /**
     * 磁盘和虚拟机配置不同的datastore
     */
    DIFFERENT_DATASTORE("different_datastore"),

    /**
     * 磁盘和虚拟机配置相同的datastore
     */
    SAME_DATASTORE("same_datastore");

    private final String name;

    VMWareMigrateDiskDatastoreType(String name) {
        this.name = name;
    }

    @JsonValue

    public String getName() {
        return name;
    }

    /**
     * get Scheduled type enum
     *
     * @param str str
     * @return RetentionUnit
     */
    public static VMWareMigrateDiskDatastoreType get(String str) {
        return EnumUtil.get(VMWareMigrateDiskDatastoreType.class, VMWareMigrateDiskDatastoreType::getName, str);
    }
}
