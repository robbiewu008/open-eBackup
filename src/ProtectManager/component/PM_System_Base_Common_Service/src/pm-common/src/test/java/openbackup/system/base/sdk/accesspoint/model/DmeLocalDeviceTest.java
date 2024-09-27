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

import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.data.ClusterDetailInfoData;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

/**
 * DmeLocalDevice test
 *
 * @author jwx701567
 * @since 2021-03-15
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = DmeLocalDevice.class)
public class DmeLocalDeviceTest {
    @MockBean
    private ClusterInternalApi clusterInternalApi;

    @Test
    public void build_dme_local_device_success() {
        ClusterDetailInfo clusterDetailInfo = ClusterDetailInfoData.getClusterDetailInfo();
        Mockito.when(clusterInternalApi.queryClusterDetails()).thenReturn(clusterDetailInfo);

        DmeLocalDevice dmeLocalDevice = DmeLocalDevice.build(clusterInternalApi);
        Assert.assertNotNull(ClusterDetailInfoData.port + "", String.valueOf(dmeLocalDevice.getPort()));
        Assert.assertNotNull(ClusterDetailInfoData.uername, dmeLocalDevice.getUserName());
        Assert.assertNotNull(ClusterDetailInfoData.password, dmeLocalDevice.getPassword());
        Assert.assertNotNull("2", String.valueOf(dmeLocalDevice.getMgrIp().size()));
    }

}
