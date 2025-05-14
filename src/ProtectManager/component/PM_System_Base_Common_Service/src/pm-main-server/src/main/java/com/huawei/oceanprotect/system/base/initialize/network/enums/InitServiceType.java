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
package com.huawei.oceanprotect.system.base.initialize.network.enums;

/**
 * 归档IP类型
 *
 */
public enum InitServiceType {
    /**
     * Init Network
     */
    INIT_NETWORK("INIT_NETWORK"),
    /**
     * Modify BackupNetwork
     */
    MODIFY_BACKUP_NETWORK("MODIFY_BACKUP_NETWORK"),

    /**
     * Modify ArchiveNetwork
     */
    MODIFY_ARCHIVE_NETWORK("MODIFY_ARCHIVE_NETWORK"),

    /**
     * init Standard
     */
    INIT_STANDARD("INIT_STANDARD"),

    /**
     * init Standard
     */
    EXPANSION_NETWORK("EXPANSION_NETWORK");

    private String value;

    /**
     * 默认构造函数
     *
     * @param type 类型
     */
    InitServiceType(String type) {
        value = type;
    }

    /**
     * 获取IP类型时ipv4还是ipv6
     *
     * @return ipType
     */
    public String getType() {
        return value;
    }
}