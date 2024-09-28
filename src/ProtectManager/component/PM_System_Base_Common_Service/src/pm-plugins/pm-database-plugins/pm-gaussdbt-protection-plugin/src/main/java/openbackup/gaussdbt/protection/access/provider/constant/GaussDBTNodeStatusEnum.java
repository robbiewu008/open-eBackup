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
package openbackup.gaussdbt.protection.access.provider.constant;

/**
 * GaussDBT集群节点状态枚举
 *
 */
public enum GaussDBTNodeStatusEnum {
    /**
     * 在线状态
     */
    ONLINE("ONLINE"),

    /**
     * 离线状态
     */
    OFFLINE("OFFLINE"),

    /**
     * 停止状态
     */
    STOPPED("STOPPED");

    private final String status;

    /**
     * Constructor
     *
     * @param status 状态
     */
    GaussDBTNodeStatusEnum(String status) {
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
