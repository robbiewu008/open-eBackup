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

import java.util.List;

/**
 * cluster params
 *
 */
@Data
public class SourceClustersParams {
    // cluster added as target cluster times
    private int addedCount;

    // cluster status
    private int clusterStatus;

    // dorado controller node count
    private int nodeCount;

    // cluster node ip list
    private List<String> mgrIpList;

    // storage controller ip obj
    private String storageDisplayIps;

    private String deployType;

    // 当前节点的roleType
    private String roleType;
}
