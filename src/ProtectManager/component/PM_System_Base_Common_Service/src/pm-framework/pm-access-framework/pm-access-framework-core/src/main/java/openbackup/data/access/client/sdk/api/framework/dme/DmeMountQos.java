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
package openbackup.data.access.client.sdk.api.framework.dme;

import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.Data;

/**
 * Dme Mount Qos
 *
 * @author l00272247
 * @since 2022-01-07
 */
@Data
@JsonInclude(JsonInclude.Include.NON_DEFAULT)
public class DmeMountQos {
    private int bandwidthMin;
    private int bandwidthMax;
    private int bandwidthBurst;
    private int iopsMin;
    private int iopsMax;
    private int iopsBurst;
    private int burstTime;
    private int latency;
}
