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

import lombok.Data;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.SourceClustersParams;
import openbackup.system.base.sdk.cluster.model.StorageSystemInfo;

import java.util.List;

/**
 * Dme Local Device
 *
 */
@Data
public class DmeLocalDevice {
    private int port;

    private String userName;

    private String password;

    private String cert;

    private List<String> mgrIp;

    private String esn;

    private String localStorageType;

    /**
     * build dme local device by ClusterInternalApi
     *
     * @param clusterInternalApi clusterInternalApi
     * @return DmeLocalDevice
     */
    public static DmeLocalDevice build(ClusterInternalApi clusterInternalApi) {
        ClusterDetailInfo clusterDetail = clusterInternalApi.queryClusterDetails();
        DmeLocalDevice localDevice = new DmeLocalDevice();
        SourceClustersParams sourceClustersParams = clusterDetail.getSourceClusters();
        localDevice.setMgrIp(sourceClustersParams.getMgrIpList());
        StorageSystemInfo storage = clusterDetail.getStorageSystem();
        localDevice.setUserName(storage.getUsername());
        localDevice.setPassword(storage.getPassword());
        localDevice.setPort(storage.getStoragePort());
        localDevice.setEsn(storage.getStorageEsn());
        return localDevice;
    }
}
