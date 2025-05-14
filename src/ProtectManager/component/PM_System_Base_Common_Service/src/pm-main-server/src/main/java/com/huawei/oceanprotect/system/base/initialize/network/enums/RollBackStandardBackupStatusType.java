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
 * @author l00347293
 * @since 2020-12-19
 */
public enum RollBackStandardBackupStatusType {
    /**
     * 回滚中
     */
    ROLL_BACK("rollbacking"),
    /**
     * 回滚失败
     */
    FAILED("failed"),
    /**
     * 回滚成功
     */
    READY("ready-init"),
    /**
     * 未回滚
     */
    UNROLL_BACK("ready");

    private String value;

    /**
     * 默认构造函数
     *
     * @param type 类型
     */
    RollBackStandardBackupStatusType(String type) {
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