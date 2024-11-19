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
package openbackup.system.base.sdk.cluster.model;

import lombok.Data;
import openbackup.system.base.bean.DeviceNetworkInfo;
import openbackup.system.base.sdk.infrastructure.model.beans.NodePodInfo;

import java.math.BigDecimal;
import java.util.List;

/**
 * Storage system info
 *
 */
@Data
public class StorageSystemInfo {
    // storage esn
    private String storageEsn;

    // storage work port
    private int storagePort;

    private String username;

    private String password;

    // 集群已使用容量 kb
    private BigDecimal usedCapacity;

    // 集群容量 kb
    private BigDecimal capacity;

    private List<NodePodInfo> netplaneInfo;

    private DeviceNetworkInfo deviceNetworkInfo;
}
