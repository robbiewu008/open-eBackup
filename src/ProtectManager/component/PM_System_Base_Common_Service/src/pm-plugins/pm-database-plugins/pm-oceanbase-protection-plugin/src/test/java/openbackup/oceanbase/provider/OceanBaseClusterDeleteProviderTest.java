/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.oceanbase.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.oceanbase.common.dto.OBClusterInfo;
import openbackup.oceanbase.common.util.OceanBaseUtils;
import openbackup.oceanbase.service.OceanBaseService;

import com.alibaba.fastjson.JSON;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Optional;

@RunWith(PowerMockRunner.class)
public class OceanBaseClusterDeleteProviderTest {

    private OceanBaseService oceanBaseService;

    private OceanBaseClusterDeleteProvider oceanBaseClusterDeleteProvider;

    @Before
    public void init() {
        oceanBaseService = Mockito.mock(OceanBaseService.class);
        oceanBaseClusterDeleteProvider = new OceanBaseClusterDeleteProvider(oceanBaseService);
    }

    /**
     * 用例场景：删除资源时解除持续挂载成功
     * 前置条件：传入注册信息正确
     * 检查点：生成endpoints/version/uuid正确
     */
    @Test
    public void preHandleDelete_success() {
        String param
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"123456\",\"authType\":2},\"dependencies\":{\"clientAgents\":[{\"uuid\":\"3d6544b6-23e9-44bc-bcab-de72a3d21682\"}],\"serverAgents\":[{\"uuid\":\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\"}]},\"extendInfo\":{\"clusterInfo\":\"{\\\"obServerAgents\\\":[{\\\"parentUuid\\\":\\\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\\\",\\\"ip\\\":\\\"8.40.129.26\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\"}],\\\"obClientAgents\\\":[{\\\"parentUuid\\\":\\\"3d6544b6-23e9-44bc-bcab-de72a3d21682\\\",\\\"nodeType\\\":\\\"OBClient\\\"}]}\"},\"name\":\"集群名称adfasgdsa\",\"port\":0,\"scanInterval\":3600,\"sourceType\":\"register\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\"}";
        ProtectedResource protectedResource = JSON.parseObject(param, ProtectedResource.class);
        OBClusterInfo clusterInfo = OceanBaseUtils.readExtendClusterInfo(protectedResource);
        String obClientUuid = clusterInfo.getObClientAgents().get(0).getParentUuid();
        String obServerUuid = clusterInfo.getObServerAgents().get(0).getParentUuid();
        mockAgent(obClientUuid, obServerUuid);

        ProtectedResource dbRecord = new ProtectedResource();
        dbRecord.setParentUuid(protectedResource.getParentUuid());
        PowerMockito.when(oceanBaseService.getResourceById(protectedResource.getUuid()))
            .thenReturn(Optional.of(dbRecord));

        PowerMockito.doNothing().when(oceanBaseService).removeDataRepoWhiteListOfResource(any());
        PowerMockito.doNothing().when(oceanBaseService).umountDataRepo(any(), any());
        oceanBaseClusterDeleteProvider.preHandleDelete(protectedResource);
    }

    private void mockAgent(String obClientUuid, String obServerUuid) {
        ProtectedEnvironment agent1 = new ProtectedEnvironment();
        agent1.setEndpoint("192.168.129.12");
        agent1.setUuid(obClientUuid);
        PowerMockito.when(oceanBaseService.getEnvironmentById(any())).thenReturn(agent1);

        ProtectedEnvironment agent2 = new ProtectedEnvironment();
        agent2.setEndpoint("192.168.129.26");
        agent2.setUuid(obServerUuid);
        PowerMockito.when(oceanBaseService.getEnvironmentById(any())).thenReturn(agent2);
    }
}
