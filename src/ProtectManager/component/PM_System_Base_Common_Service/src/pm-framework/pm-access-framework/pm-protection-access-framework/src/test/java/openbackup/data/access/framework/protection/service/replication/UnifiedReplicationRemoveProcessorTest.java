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
package openbackup.data.access.framework.protection.service.replication;

import com.alibaba.fastjson.JSON;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.service.MultiClusterAuthenticationService;
import openbackup.data.access.client.sdk.api.framework.dme.replicate.DmeReplicationRestApi;
import openbackup.data.access.framework.protection.service.replication.UnifiedReplicationRemoveProcessor;
import openbackup.system.base.sdk.auth.api.AuthNativeApi;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.TargetClusterRestApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.SourceClustersParams;
import openbackup.system.base.sdk.cluster.model.StorageSystemInfo;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.cluster.model.TokenResponse;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;

import java.util.Collections;
import java.util.UUID;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

/**
 * Advance Replication Remove Processor Test
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(UnifiedReplicationRemoveProcessor.class)
@AutoConfigureMockMvc
public class UnifiedReplicationRemoveProcessorTest {
    @Mock
    private ClusterInternalApi clusterInternalApi;

    @Mock
    private DmeReplicationRestApi dmeReplicationRestApi;

    @Mock
    private MemberClusterService memberClusterService;

    @Mock
    private AuthNativeApi authNativeApi;

    @Mock
    private TargetClusterRestApi clusterRestApi;

    @Mock
    private MultiClusterAuthenticationService multiClusterAuthenticationService;

    @Mock
    private TargetClusterRestApi targetClusterRestApi;

    @InjectMocks
    private UnifiedReplicationRemoveProcessor unifiedReplicationRemoveProcessor;



    @Test
    public void test_process() {
        TargetClusterVo targetClusterVo = new TargetClusterVo();
        targetClusterVo.setEsn("ESN1234313");
        targetClusterVo.setMgrIpList(Collections.singletonList("1.1.1.1"));
        ClusterDetailInfo clusterDetailInfo = new ClusterDetailInfo();
        SourceClustersParams sourceClustersParams = new SourceClustersParams();
        sourceClustersParams.setMgrIpList(Collections.singletonList("127.0.0.1"));
        clusterDetailInfo.setSourceClusters(sourceClustersParams);
        StorageSystemInfo storageSystemInfo = new StorageSystemInfo();
        storageSystemInfo.setUsername("*******");
        storageSystemInfo.setPassword("*******");
        storageSystemInfo.setPassword("25081");
        clusterDetailInfo.setStorageSystem(storageSystemInfo);
        PowerMockito.when(clusterInternalApi.queryClusterDetails()).thenReturn(clusterDetailInfo);
        TokenResponse tokenResponse = new TokenResponse();
        tokenResponse.setToken("123");
        PowerMockito.when(multiClusterAuthenticationService.getTokenByTargetClusterVo(any())).thenReturn(tokenResponse);
        PowerMockito.when(targetClusterRestApi.getTargetClusterInfo(any(),anyString())).thenReturn(clusterDetailInfo);
        Whitebox.setInternalState(unifiedReplicationRemoveProcessor, "targetClusterRestApi", targetClusterRestApi);
        unifiedReplicationRemoveProcessor.process(mockResourceEntity(), targetClusterVo);
        unifiedReplicationRemoveProcessor.process(mockOracleResourceEntity(), targetClusterVo);
        Mockito.verify(memberClusterService, Mockito.times(2)).consumeInAllMembers(any(), any());
    }

    private NasDistributionStorageDetail mockDetail(){
        String json = "{\n" + "    \"uuid\": \"2fb924e6-2f02-4092-86b4-4f83e60e6d87\",\n"
            + "    \"name\": \"test11\",\n" + "    \"description\": \"dd\",\n" + "    \"type\": \"nas\",\n"
            + "    \"storageStrategyType\": null,\n" + "    \"timeoutPeriod\": 15,\n"
            + "    \"hasEnableParallelStorage\": true,\n" + "    \"clusterList\": [\n" + "        {\n"
            + "            \"clusterId\": 7,\n" + "            \"clusterName\": \"member39_auto_added\",\n"
            + "            \"status\": 27,\n" + "            \"ip\": \"8.40.102.39\",\n"
            + "            \"clusterType\": 2,\n" + "            \"port\": 8088,\n"
            + "            \"username\": \"admin\",\n" + "            \"role\": 3,\n"
            + "            \"clusterIp\": \"8.40.102.39,8.40.102.40\",\n"
            + "            \"usedCapacity\": 8.46655183E8,\n" + "            \"capacity\": 1.6515456722E10,\n"
            + "            \"storageEsn\": \"2102353GTD10L9000001\",\n"
            + "            \"availableCapacityRatio\": 90,\n" + "            \"generatedType\": 2,\n"
            + "            \"strategyOrder\": null,\n" + "            \"deployType\": \"d0\",\n"
            + "            \"controllerCount\": 2,\n" + "            \"pmPort\": 25081,\n"
            + "            \"pmStatus\": 27\n" + "        },\n" + "        {\n" + "            \"clusterId\": 2,\n"
            + "            \"clusterName\": \"BackupStorage\",\n" + "            \"status\": 27,\n"
            + "            \"ip\": \"8.40.129.164\",\n" + "            \"clusterType\": 2,\n"
            + "            \"port\": 8088,\n" + "            \"username\": \"dataprotect_admin\",\n"
            + "            \"role\": 3,\n" + "            \"clusterIp\": \"8.40.129.164,8.40.129.165\",\n"
            + "            \"usedCapacity\": 6.85515527E8,\n" + "            \"capacity\": 2.6798991714E10,\n"
            + "            \"storageEsn\": \"2102354SXX10N8100004\",\n"
            + "            \"availableCapacityRatio\": 90,\n" + "            \"generatedType\": 2,\n"
            + "            \"strategyOrder\": null,\n" + "            \"deployType\": \"d1\",\n"
            + "            \"controllerCount\": 2,\n" + "            \"pmPort\": 25081,\n"
            + "            \"pmStatus\": 27\n" + "        }\n" + "    ]\n" + "}";
        return JSON.parseObject(json, NasDistributionStorageDetail.class);
    }

    @Test
    public void test_process_intra() {
        ClusterDetailInfo clusterDetailInfo = new ClusterDetailInfo();
        SourceClustersParams sourceClustersParams = new SourceClustersParams();
        sourceClustersParams.setMgrIpList(Collections.singletonList("127.0.0.1"));
        clusterDetailInfo.setSourceClusters(sourceClustersParams);
        StorageSystemInfo storageSystemInfo = new StorageSystemInfo();
        storageSystemInfo.setUsername("*******");
        storageSystemInfo.setPassword("*******");
        storageSystemInfo.setPassword("25081");
        clusterDetailInfo.setStorageSystem(storageSystemInfo);
        Whitebox.setInternalState(unifiedReplicationRemoveProcessor, "targetClusterRestApi", targetClusterRestApi);
        PowerMockito.when(clusterInternalApi.queryClusterDetails()).thenReturn(clusterDetailInfo);
        PowerMockito.when(authNativeApi.generateClusterAdminToken()).thenReturn("test");
        unifiedReplicationRemoveProcessor.process(mockResourceEntity(), mockDetail());
        Mockito.verify(targetClusterRestApi, Mockito.times(2))
                .dispatchRemoveReplicationPair(any(), any(), any());
    }


    @Test
    public void test_applicable() {
        Assert.assertTrue(unifiedReplicationRemoveProcessor.applicable(ResourceSubTypeEnum.VMWARE.getType()));
    }

    private ResourceEntity mockResourceEntity() {
        ResourceEntity resourceEntity = new ResourceEntity();
        resourceEntity.setSubType(ResourceSubTypeEnum.IMPORT_COPY.getType());
        resourceEntity.setUuid(UUID.randomUUID().toString());
        return resourceEntity;
    }

    private ResourceEntity mockOracleResourceEntity() {
        ResourceEntity resourceEntity = new ResourceEntity();
        resourceEntity.setSubType(ResourceSubTypeEnum.ORACLE.getType());
        resourceEntity.setUuid(UUID.randomUUID().toString());
        return resourceEntity;
    }
}
