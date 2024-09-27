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
package openbackup.system.base.sdk.anti.model;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * 安全卡信息
 *
 * @author j00619968
 * @since 2024-01-23
 */
@Getter
@Setter
public class SecureCardInfo {
    /**
     * 健康状态
     */
    @JsonIgnore
    public static final String HEALTH_STATUS_OK = "1";

    /**
     * 运行状态
     */
    @JsonIgnore
    public static final String RUNNING_STATUS_OK = "2";

    /**
     * 控制器ID，例如"0A"
     */
    @JsonProperty("controllerID")
    String controllerID;

    /**
     * 健康状态，0:未知;1:正常;2:故障
     */
    @JsonProperty("healthStatus")
    String healthStatus;

    /**
     * 硬件ID
     */
    @JsonProperty("id")
    String id;

    /**
     * 硬件运行状态，0:未知;1:正常;2:运行;12:正在上电;13:已下电;27:在线;28:离线;103:上电失败
     */
    @JsonProperty("runningStatus")
    String runningStatus;
}
