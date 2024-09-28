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
 * 功能描述: 保护对象数据一致性状态枚举类型
 *
 */
public enum ConsistentStatusEnum {
    /**
     * 待检测
     */
    UNDETECTED("undetected"),

    /**
     * 一致
     */
    CONSISTENT("consistent"),

    /**
     * 不一致
     */
    INCONSISTENT("inconsistent");

    private final String status;

    ConsistentStatusEnum(String status) {
        this.status = status;
    }

    /**
     * getter
     *
     * @return status
     */
    public String getStatus() {
        return status;
    }
}
