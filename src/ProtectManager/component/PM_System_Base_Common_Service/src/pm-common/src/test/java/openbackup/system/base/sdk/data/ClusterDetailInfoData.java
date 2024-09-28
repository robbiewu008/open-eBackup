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
package openbackup.system.base.sdk.data;

import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.SourceClustersParams;
import openbackup.system.base.sdk.cluster.model.StorageSystemInfo;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * ClusterDetailInfoData test
 *
 */
public class ClusterDetailInfoData {

    public static final String uername = "adminTest";
    public static final String password = "adminTest";
    public static final String esn = "1232AFAWEZVeafwefawefaf";
    public static final int port = 8088;
    public static final String ipv4 = "192.16.1.1";

    public static ClusterDetailInfo getClusterDetailInfo() {
        // Local cluster detail info
        ClusterDetailInfo clusterDetailInfo = new ClusterDetailInfo();

        // Local cluster params
        SourceClustersParams sourceClusters = new SourceClustersParams();
        sourceClusters.setAddedCount(1);
        sourceClusters.setClusterStatus(27);
        List<String> list = new ArrayList<>();
        list.add("8.1.1.101");
        list.add("8.1.1.102");
        sourceClusters.setMgrIpList(list);

        clusterDetailInfo.setSourceClusters(sourceClusters);

        // storage system info
        StorageSystemInfo systemInfo = new StorageSystemInfo();
        systemInfo.setStorageEsn(esn);
        systemInfo.setStoragePort(port);
        systemInfo.setUsername(uername);
        systemInfo.setPassword(password);
        systemInfo.setNetplaneInfo(Collections.emptyList());
        clusterDetailInfo.setStorageSystem(systemInfo);


        return clusterDetailInfo;

    }

}
