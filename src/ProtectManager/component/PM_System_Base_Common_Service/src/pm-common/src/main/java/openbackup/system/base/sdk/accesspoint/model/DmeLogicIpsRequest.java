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
package openbackup.system.base.sdk.accesspoint.model;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * DME 修改逻辑端口链路请求
 *
 */
@Data
@JsonInclude(JsonInclude.Include.NON_NULL)
public class DmeLogicIpsRequest {
    /**
     * old name
     */
    @JsonProperty("originLogicIpNames")
    private List<String> originLogicIpNames;

    /**
     * new name
     */
    @JsonProperty("newLogicIpNames")
    private List<String> newLogicIpNames;

    /**
     * Dme Local Device
     */
    @JsonProperty("LocalDevice")
    private DmeLocalDevice localDevice;
}
