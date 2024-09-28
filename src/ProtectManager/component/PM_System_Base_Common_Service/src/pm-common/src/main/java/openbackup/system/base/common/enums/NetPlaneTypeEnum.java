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

import lombok.Getter;

/**
 * 任务类型枚举
 *
 */
@Getter
public enum NetPlaneTypeEnum {
    /**
     * 备份
     */
    BACKUP("backup"),
    /**
     * 复制
     */
    REPLICATION("replication"),
    /**
     * 归档
     */
    ARCHIVE("archive");

    /**
     * action名称
     */
    private final String name;

    NetPlaneTypeEnum(String name) {
        this.name = name;
    }
}
