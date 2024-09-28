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
package openbackup.system.base.sdk.storage.enums;

import com.fasterxml.jackson.annotation.JsonValue;

/**
 * dorado设备信息
 *
 */
public enum DoradoRunningStatus {
    /**
     * 正常
     */
    NORMAL(1),
    /**
     * 未运行
     */
    NOT_RUNNING(3),
    /**
     * 正在上电
     */
    POWERING_ON(12),
    /**
     * 正在下电
     */
    POWERING_OFF(47),
    /**
     * 正在升级
     */
    UPGRADING(51);

    private int type;

    DoradoRunningStatus(int type) {
        this.type = type;
    }

    @JsonValue
    public int getType() {
        return type;
    }
}
