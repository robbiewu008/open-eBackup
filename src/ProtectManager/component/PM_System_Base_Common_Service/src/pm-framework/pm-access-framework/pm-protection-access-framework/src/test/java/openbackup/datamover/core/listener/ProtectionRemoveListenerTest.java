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
package openbackup.datamover.core.listener;

import com.alibaba.fastjson.JSON;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import openbackup.data.access.framework.core.dao.ProtectedObjectMapper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.listener.v1.backup.ProtectionRemoveListener;
import openbackup.data.access.framework.protection.listener.v1.replication.ReplicationProtectionRemoveProcessor;
import openbackup.data.protection.access.provider.sdk.backup.BackupProvider;
import com.huawei.oceanprotect.repository.service.impl.NasDistributionStorageServiceImpl;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.protection.SlaRestApi;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.SlaBo;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.ResourceRestApi;
import openbackup.system.base.sdk.resource.model.FileSetEntity;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.kafka.support.Acknowledgment;

import java.util.Collections;
import java.util.List;
import java.util.UUID;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;

/**
 * ProtectionBackupListener LLT
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(ProtectionRemoveListener.class)
@AutoConfigureMockMvc
public class ProtectionRemoveListenerTest {
    @Mock
    private ProviderManager registry;

    @Mock
    private ResourceRestApi resourceRestApi;

    @Mock
    private SlaRestApi slaRestApi;

    @Mock
    private ClusterInternalApi clusterInternalApi;

    @Mock
    private RedissonClient redissonClient;

    @Mock
    private ProtectObjectRestApi protectObjectRestApi;

    @InjectMocks
    private ProtectionRemoveListener protectionRemoveListener;

    @Mock
    private NasDistributionStorageServiceImpl nasDistributionStorageService;

    @Mock
    private ProtectedObjectMapper protectedObjectMapper;

    @Test
    public void test_remove_protection() throws JsonProcessingException {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();

        JSONObject data = new JSONObject();
        data.set("request_id", UUID.randomUUID().toString());
        data.set("resource_id", UUID.randomUUID().toString());
        data.set("sla_id", UUID.randomUUID().toString());
        FileSetEntity fileSetEntity = PowerMockito.mock(FileSetEntity.class);
        PowerMockito.when(resourceRestApi.queryResource(ArgumentMatchers.anyString())).thenReturn(fileSetEntity);
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(
                redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
            .thenReturn(map);
        ReplicationProtectionRemoveProcessor processor = PowerMockito.mock(ReplicationProtectionRemoveProcessor.class);
        PowerMockito.when(
                registry.findProviderOrDefault(ArgumentMatchers.any(), ArgumentMatchers.any(), ArgumentMatchers.any()))
            .thenReturn(processor);
        SlaBo slaBo = new SlaBo();
        PolicyBo policyBo = new PolicyBo();
        policyBo.setType("replication");
        JSONObject jsonObject = new JSONObject();
        jsonObject.set("external_system_id", 1);
        ObjectMapper mapper = new ObjectMapper();
        JsonNode node = mapper.readTree(jsonObject.toString());
        policyBo.setExtParameters(node);
        policyBo.setUuid(UUID.randomUUID().toString());
        List<PolicyBo> policyBos = Collections.singletonList(policyBo);
        slaBo.setPolicyList(policyBos);
        PowerMockito.when(slaRestApi.querySlaById(ArgumentMatchers.anyString())).thenReturn(slaBo);
        PowerMockito.when(map.put(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(null);
        TargetClusterVo targetClusterVo = new TargetClusterVo();
        targetClusterVo.setClusterId("1");
        PowerMockito.when(clusterInternalApi.queryTargetClusterDetailsByClusterId(anyInt()))
            .thenReturn(targetClusterVo);
        BackupProvider provider = PowerMockito.mock(BackupProvider.class);
        PowerMockito.when(registry.findProvider(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(provider);
        PowerMockito.when(protectedObjectMapper.countBySlaId(any())).thenReturn(1);
        protectionRemoveListener.removeProtection(data.toString(), acknowledgment);
        protectionRemoveListener.changeProtection(data.toString(), acknowledgment);
        Mockito.verify(slaRestApi, Mockito.times(3)).querySlaById(any());
    }

    @Test
    public void test_remove_protection_intra() throws JsonProcessingException {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
        PowerMockito.when(protectedObjectMapper.countBySlaId(any())).thenReturn(1);
        JSONObject data = new JSONObject();
        data.set("request_id", UUID.randomUUID().toString());
        data.set("resource_id", UUID.randomUUID().toString());
        data.set("sla_id", UUID.randomUUID().toString());
        FileSetEntity fileSetEntity = PowerMockito.mock(FileSetEntity.class);
        PowerMockito.when(resourceRestApi.queryResource(ArgumentMatchers.anyString())).thenReturn(fileSetEntity);
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(
                redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
            .thenReturn(map);
        ReplicationProtectionRemoveProcessor processor = PowerMockito.mock(ReplicationProtectionRemoveProcessor.class);
        PowerMockito.when(
                registry.findProviderOrDefault(ArgumentMatchers.any(), ArgumentMatchers.any(), ArgumentMatchers.any()))
            .thenReturn(processor);
        SlaBo slaBo = new SlaBo();
        PolicyBo policyBo = new PolicyBo();
        policyBo.setType("replication");
        JSONObject jsonObject = new JSONObject();
        jsonObject.set("replication_target_mode", 2);
        jsonObject.set("external_storage_id", "test111");
        ObjectMapper mapper = new ObjectMapper();
        JsonNode node = mapper.readTree(jsonObject.toString());
        policyBo.setExtParameters(node);
        policyBo.setUuid(UUID.randomUUID().toString());
        List<PolicyBo> policyBos = Collections.singletonList(policyBo);
        slaBo.setPolicyList(policyBos);
        PowerMockito.when(slaRestApi.querySlaById(ArgumentMatchers.anyString())).thenReturn(slaBo);
        PowerMockito.when(map.put(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(null);
        TargetClusterVo targetClusterVo = new TargetClusterVo();
        targetClusterVo.setClusterId("1");
        PowerMockito.when(clusterInternalApi.queryTargetClusterDetailsByClusterId(anyInt()))
            .thenReturn(targetClusterVo);

        PowerMockito.when(nasDistributionStorageService.getDetail(anyString()))
            .thenReturn(mockDetail());
        BackupProvider provider = PowerMockito.mock(BackupProvider.class);
        PowerMockito.when(registry.findProvider(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(provider);

        protectionRemoveListener.removeProtection(data.toString(), acknowledgment);
        protectionRemoveListener.changeProtection(data.toString(), acknowledgment);
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
    public void test_on_sla_changed() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();

        JSONObject data = new JSONObject();
        SlaBo oldSla = new SlaBo();
        SlaBo newSla = new SlaBo();
        PolicyBo policyBo = new PolicyBo();
        policyBo.setUuid(UUID.randomUUID().toString());
        List<PolicyBo> policyBos = Collections.singletonList(policyBo);
        oldSla.setPolicyList(policyBos);
        TargetClusterVo targetClusterVo = new TargetClusterVo();
        targetClusterVo.setClusterId("1");
        PowerMockito.when(clusterInternalApi.queryTargetClusterDetailsByClusterId(anyInt()))
            .thenReturn(targetClusterVo);
        data.set("old_sla", oldSla);
        data.set("new_sla", newSla);

        protectionRemoveListener.onSlaChanged(data.toString(), acknowledgment);
        Mockito.verify(acknowledgment, Mockito.times(1)).acknowledge();
    }
}
