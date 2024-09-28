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
package openbackup.system.base.sdk.cluster.enums;

/**
 * HA证书操作类型
 *
 */
public enum HaCertOperatorEnum {
    /**
     * 更新证书
     */
    UPDATE("update"),

    /**
     * 回滚证书
     */
    ROLLBACK("rollback");

    /**
     * 操作业务类型
     */
    private final String type;

    HaCertOperatorEnum(String type) {
        this.type = type;
    }

    public String getType() {
        return type;
    }
}
