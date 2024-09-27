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
package openbackup.gaussdbdws.protection.access.enums;

/**
 * 备份方式枚举
 *
 * @author swx1010572
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-07
 */
public enum BackupToolEnum {
    /**
     * ROACH备份方式
     */
    ROACH("0"),

    /**
     * GDS备份方式
     */
    GDS("1");

    BackupToolEnum(String type) {
        this.type = type;
    }

    private final String type;

    public String getType() {
        return type;
    }
}
