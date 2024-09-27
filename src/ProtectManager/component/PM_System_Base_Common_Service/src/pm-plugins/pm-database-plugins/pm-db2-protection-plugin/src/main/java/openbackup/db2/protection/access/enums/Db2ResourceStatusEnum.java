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
package openbackup.db2.protection.access.enums;

import java.util.Arrays;

/**
 * db2资源状态
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-02-20
 */
public enum Db2ResourceStatusEnum {
    NORMAL("Normal"),

    BACKUPING("Backuping"),

    RESTORING("Restoring");

    Db2ResourceStatusEnum(String status) {
        this.status = status;
    }

    private final String status;

    public String getStatus() {
        return status;
    }

    /**
     * 根据status获取到对应的枚举
     *
     * @param status 枚举值
     * @return enum
     */
    public static Db2ResourceStatusEnum getByStatus(String status) {
        return Arrays.stream(Db2ResourceStatusEnum.values())
            .filter(location -> location.status.equals(status))
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }
}
