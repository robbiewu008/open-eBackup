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
package openbackup.datamover.core.listener.replication;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.protection.listener.v1.replication.ReplicateContext;
import openbackup.data.protection.access.provider.sdk.backup.BackupObject;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.TargetClusterRestApi;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.service.ApplicationContextService;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.Arrays;

/**
 * Replicate Context Test
 *
 * @author l00272247
 * @since 2021-01-03
 */
@RunWith(SpringRunner.class)
@SpringBootTest
@ContextConfiguration(classes = ApplicationContextService.class)
public class ReplicateContextTest {
    @MockBean(name = "targetClusterApiWithDmaProxy")
    private TargetClusterRestApi targetClusterApiWithDmaProxy;

    @MockBean(name = "targetClusterApiWithRoute")
    private TargetClusterRestApi targetClusterApiWithRoute;

    @MockBean
    private ClusterInternalApi clusterInternalApi;

    @Autowired
    private ApplicationContextService applicationContextService;

    private ClusterNativeApi clusterNativeApi;

    private TargetClusterVo targetClusterVo;

    @Before
    public void init() {
        this.clusterNativeApi = Mockito.mock(ClusterNativeApi.class);
        this.targetClusterVo = Mockito.mock(TargetClusterVo.class);
    }

    @Test
    public void testAutowire() {
        ReplicateContext replicateContext = applicationContextService.autowired(ReplicateContext.builder().build(),
            Arrays.asList(targetClusterApiWithDmaProxy, targetClusterApiWithRoute,
                clusterNativeApi, Mockito.mock(EncryptorService.class),
                Mockito.mock(BackupObject.class), Mockito.mock(ResourceEntity.class), Mockito.mock(PolicyBo.class),
                targetClusterVo));
        Assert.assertNotNull(replicateContext.getBackupObject());
        Assert.assertNotNull(replicateContext.getResourceEntity());
        Assert.assertNotNull(replicateContext.getPolicy());
        Assert.assertNotNull(replicateContext.getTargetCluster());
    }
}
