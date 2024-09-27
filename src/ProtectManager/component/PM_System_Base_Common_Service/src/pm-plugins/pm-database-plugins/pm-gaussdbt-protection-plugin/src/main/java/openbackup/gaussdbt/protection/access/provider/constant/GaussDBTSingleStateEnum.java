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
 * GaussDBT单机版状态枚举
 *
 * @author lWX776769
 * @version [DataBackup 1.5.0]
 * @since 2023/7/21
 */
public enum GaussDBTSingleStateEnum {
    /**
     * 正常
     */
    NORMAL("Normal"),

    /**
     * 离线
     */
    OFFLINE("Offline");

    private final String state;

    /**
     * GaussDBTSingleStateEnum
     *
     * @param state 状态类型
     */
    GaussDBTSingleStateEnum(String state) {
        this.state = state;
    }

    /**
     * getter
     *
     * @return 类型
     */
    public String getState() {
        return state;
    }
}
