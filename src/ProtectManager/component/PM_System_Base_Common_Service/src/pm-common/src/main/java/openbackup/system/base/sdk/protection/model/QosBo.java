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
package openbackup.system.base.sdk.protection.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * replication qos返回类
 *
 */
@Data
public class QosBo {
    /**
     * qos的uuid
     */
    private String uuid;

    /**
     * qos的名字
     */
    private String name;

    /**
     * qos的限速带宽
     */
    @JsonProperty("speed_limit")
    private int speedLimit;

    /**
     * qos的详情描述
     */
    private String description;

    /**
     * get qos bo speed limit
     *
     * @param qosBo qos bo
     * @return speed limit
     */
    public static int getQosBoSpeedLimit(QosBo qosBo) {
        return qosBo != null ? qosBo.getSpeedLimit() : 0;
    }
}
