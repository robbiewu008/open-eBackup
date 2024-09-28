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

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * 外部集群查询参数对象
 *
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class TargetClusterRequestParm {
    private List<Integer> clusterIdList;

    private List<String> esnList;

    private Integer status;

    private Integer generatedType;

    private String remoteEsn;

    private List<Integer> roleList;

    public TargetClusterRequestParm(List<Integer> clusterIdList) {
        this.clusterIdList = clusterIdList;
    }
}
