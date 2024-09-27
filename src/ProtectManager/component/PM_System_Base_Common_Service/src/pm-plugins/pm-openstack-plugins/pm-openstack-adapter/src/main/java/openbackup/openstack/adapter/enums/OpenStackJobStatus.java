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
package openbackup.openstack.adapter.enums;

import com.fasterxml.jackson.annotation.JsonValue;

/**
 * 云核OpenStack任务状态
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-05
 */
public enum OpenStackJobStatus {
    /**
     * 正在调度中
     */
    SCHEDULING("scheduling"),

    /**
     * 正在执行中
     */
    RUNNING("running"),

    /**
     * 已停止
     */
    STOP("stop"),

    /**
     * 已结束
     */
    COMPLETED("completed");

    private final String status;

    OpenStackJobStatus(String status) {
        this.status = status;
    }

    @JsonValue
    public String getStatus() {
        return status;
    }
}
