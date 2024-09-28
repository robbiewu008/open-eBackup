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
package openbackup.tdsql.resources.access.provider;

/**
 * 功能描述
 *
 */

import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.tdsql.resources.access.service.TdsqlService;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

@RunWith(MockitoJUnitRunner.class)
public class TdsqlClusterGroupConnectionCheckerTest {
    @Mock
    private ResourceService resourceService;

    @Mock
    private ResourceService mockResourceService;

    @Mock
    private ProtectedEnvironmentRetrievalsService mockEnvironmentRetrievalsService;

    @Mock
    private AgentUnifiedService mockAgentUnifiedService;

    @Mock
    private TdsqlService mockTdsqlService;

    @Mock
    private ProviderManager providerManager;

    private TdsqlClusterGroupConnectionChecker tdsqlClusterGroupConnectionChecker;

    @Before
    public void setUp() {
        tdsqlClusterGroupConnectionChecker = new TdsqlClusterGroupConnectionChecker(mockEnvironmentRetrievalsService,
            mockAgentUnifiedService, mockTdsqlService, resourceService, providerManager);
    }

    /**
     * 用例场景：策略模式策略识别-TDSQL
     * 前置条件：类型参数为TDSQL-clusterGroup
     * 检查点：识别成功
     */
    @Test
    public void test_applicable_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.TDSQL_CLUSTERGROUP.getType());
        boolean result = tdsqlClusterGroupConnectionChecker.applicable(resource);
        assertTrue(result);
    }

    /**
     * 用例场景：获取集群节点及其对应的主机
     * 前置条件：集群已注册成功，获取环境信息成功
     * 检查点：获取集群节点及其对应的主机成功
     */
    @Test
    public void test_collect_collectable_resources() {
        ResourceConnectionCheckProvider connectionCheckProvider = PowerMockito.mock(ResourceConnectionCheckProvider.class);
        ResourceCheckContext context = new ResourceCheckContext();
        List<ActionResult> results = new ArrayList<>();
        results.add(new ActionResult());
        context.setActionResults(results);
        String clusterId = "96590445-0df7-31f4-806b-9fb9e4ed548d";
        when(mockTdsqlService.getEnvironmentById(clusterId)).thenReturn(getClusterEnvironment());
        PowerMockito.when(connectionCheckProvider.tryCheckConnection(any())).thenReturn(context);
        String agentId = "7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7";
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setEndpoint("9.9.9.9");
        agentEnvironment.setPort(9999);
        when(mockTdsqlService.getEnvironmentById(agentId)).thenReturn(agentEnvironment);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(connectionCheckProvider);
        tdsqlClusterGroupConnectionChecker.collectConnectableResources(getEnvironment());
    }

    private Map<ProtectedResource, List<ProtectedEnvironment>> mockCollectConnectableResources() {
        Map<ProtectedResource, List<ProtectedEnvironment>> map = new HashMap<>();
        List<ProtectedEnvironment> list = new ArrayList<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("1");
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8088);
        list.add(protectedEnvironment);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
        protectedResource.setUuid("test1");
        map.put(protectedResource, list);
        return map;
    }

    private ProtectedEnvironment getClusterEnvironment() {
        String json
            = "{\"uuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"name\":\"testzyl\",\"type\":\"Database\",\"subType\":\"TDSQL-cluster\",\"createdTime\":\"2023-05-31 20:10:22.084\",\"rootUuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"sourceType\":\"register\",\"protectionStatus\":0,\"extendInfo\":{\"linkStatus\":\"1\",\"clusterInfo\":\"{\\\"ossNodes\\\":[{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\\\",\\\"ip\\\":\\\"8.40.147.38\\\",\\\"port\\\":\\\"8080\\\"},{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\\\",\\\"ip\\\":\\\"8.40.147.39\\\",\\\"port\\\":\\\"8080\\\"},{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"16f74c9f-915c-4af6-91f6-40c643f13fd5\\\",\\\"ip\\\":\\\"8.40.147.40\\\",\\\"port\\\":\\\"8080\\\"}],\\\"schedulerNodes\\\":[{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\\\",\\\"ip\\\":\\\"8.40.147.38\\\"},{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\\\",\\\"ip\\\":\\\"8.40.147.39\\\"},{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"16f74c9f-915c-4af6-91f6-40c643f13fd5\\\",\\\"ip\\\":\\\"8.40.147.40\\\"}]}\"},\"endpoint\":\"192.168.147.38,192.168.147.39,192.168.147.40,192.168.147.38,192.168.147.39,192.168.147.40\",\"port\":0,\"auth\":{\"authType\":2,\"authKey\":\"DES\",\"authPwd\":\"DES\",\"extendInfo\":{}},\"dependencies\":{\"agents\":[{\"uuid\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"name\":\"tdsql-h63\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.40,8.40.147.40,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"$citations_agents_96e73f8ccba74641bc75d44c16b7d97e\":\"0fc6cb490c73476bb90aa69e40f3c931\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_5ce5b61e6fed4c618a6131ad28ef2e48\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\"},\"endpoint\":\"192.168.147.40\",\"port\":59531,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false},{\"uuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"name\":\"host-8-40-147-32\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.38,8.40.147.38,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_8503624548bf45d68368c26cf12027dc\":\"0fc6cb490c73476bb90aa69e40f3c931\",\"$citations_agents_435d0f7267f14c40879d86094149ed51\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\"},\"endpoint\":\"192.168.147.38\",\"port\":59530,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false},{\"uuid\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"name\":\"host-8-40-147-33\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.39,8.40.147.39,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"$citations_agents_dea1dc850efa4568bc13d815ba0be3d7\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_156c0a940f72494bbfb802a7db1e34d2\":\"0fc6cb490c73476bb90aa69e40f3c931\"},\"endpoint\":\"192.168.147.39\",\"port\":59522,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false}]},\"linkStatus\":\"0\",\"scanInterval\":3600,\"cluster\":false}";
        return JsonUtil.read(json, ProtectedEnvironment.class);
    }

    private ProtectedEnvironment getEnvironment() {
        String json  =  "{\n" + "  \"uuid\": \"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\n"
            + "  \"name\": \"group_1698889827_3\",\n" + "  \"parentUuid\": \"96590445-0df7-31f4-806b-9fb9e4ed548d\",\n"
            + "  \"type\": \"Database\",\n" + "  \"subType\": \"TDSQL-clusterGroup\",\n" + "  \"extendInfo\": {\n"
            + "    \"clusterGroupInfo\": \"{\\\"id\\\":\\\"group_1698889827_3\\\",\\\"name\\\":\\\"group_1698889827_3\\\",\\\"cluster\\\":\\\"9e68a8f1-7ad4-3eef-a808-dce3b2062120\\\",\\\"type\\\":\\\"1\\\",\\\"group\\\":{\\\"groupId\\\":\\\"group_1698889827_3\\\",\\\"setIds\\\":[\\\"set_1698890174_3\\\",\\\"set_1698890103_1\\\"],\\\"dataNodes\\\":[{\\\"ip\\\":\\\"8.40.168.190\\\",\\\"parentUuid\\\":\\\"dd15b622-fd7c-4a7c-9841-b0fb45b4201f\\\"},{\\\"ip\\\":\\\"8.40.168.191\\\",\\\"parentUuid\\\":\\\"ce56a464-3b7b-4016-88f2-58ae12fb6d1d\\\"},{\\\"ip\\\":\\\"8.40.168.192\\\",\\\"parentUuid\\\":\\\"c75146f7-7e2a-41d6-b110-28d0e22245ee\\\"}]}}\"\n"
            + "  }\n" + "}";
        return JsonUtil.read(json, ProtectedEnvironment.class);
    }
}
