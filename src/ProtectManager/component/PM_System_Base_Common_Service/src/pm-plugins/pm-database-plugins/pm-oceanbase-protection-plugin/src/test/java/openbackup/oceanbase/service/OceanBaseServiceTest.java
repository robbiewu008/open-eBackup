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
package openbackup.oceanbase.service;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.argThat;
import static org.mockito.Mockito.verify;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.oceanbase.common.constants.OBConstants;
import openbackup.oceanbase.common.dto.OBClusterInfo;
import openbackup.oceanbase.common.dto.OBTenantInfo;
import openbackup.oceanbase.common.util.OceanBaseUtils;
import openbackup.system.base.common.exception.LegoCheckedException;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.NfsServiceApi;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import com.alibaba.fastjson.JSON;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatcher;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
public class OceanBaseServiceTest {
    private ResourceService resourceService;

    private AgentUnifiedService agentUnifiedService;

    private OceanBaseService oceanBaseService;

    private DmeUnifiedRestApi dmeUnifiedRestApi;

    private ClusterBasicService clusterBasicService;

    private NfsServiceApi nfsServiceApi;

    private JobService jobService;

    @Before
    public void init() {
        resourceService = Mockito.mock(ResourceService.class);
        agentUnifiedService = Mockito.mock(AgentUnifiedService.class);
        dmeUnifiedRestApi = Mockito.mock(DmeUnifiedRestApi.class);
        clusterBasicService = Mockito.mock(ClusterBasicService.class);
        nfsServiceApi = Mockito.mock(NfsServiceApi.class);
        jobService = Mockito.mock(JobService.class);
        oceanBaseService = new OceanBaseServiceImpl(jobService, resourceService, agentUnifiedService, dmeUnifiedRestApi,
            nfsServiceApi, clusterBasicService);
    }

    /**
     * 用例场景：查询资源信息成功
     * 前置条件：Pm环境正常
     * 检查点：返回资源信息
     */
    @Test
    public void get_environment_by_id_success() {
        String uuid = "3d6544b6-23e9-44bc-bcab-de72a3d21682";
        String resource
            = "{\"cluster\":false,\"createdTime\":\"2023-07-08 15:09:41.0\",\"endpoint\":\"192.168.129.12\",\"extendInfo\":{\"agent_last_update_time\":\"1689127212483\",\"agentIpList\":\"192.168.129.12,8.40.129.12,192.168.122.1,172.17.0.1,fe80::5f66:16a8:fbf9:591e,fe80::1649:562a:f5cf:7fe6\",\"agent_domain_available_ip\":\"protectengine-1.protectengine.dpa.svc.cluster.local,protectengine-0.protectengine.dpa.svc.cluster.local\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_960581fbd7244a1e94b6cdad7159c02c\":\"50f3bbcb-dff5-31d8-857c-2879f321a132\"},\"linkStatus\":\"1\",\"name\":\"localhost.localdomain\",\"osName\":\"linux\",\"osType\":\"linux\",\"port\":59527,\"protectionStatus\":0,\"rootUuid\":\"3d6544b6-23e9-44bc-bcab-de72a3d21682\",\"scanInterval\":3600,\"subType\":\"UBackupAgent\",\"type\":\"Host\",\"username\":\"\",\"uuid\":\"3d6544b6-23e9-44bc-bcab-de72a3d21682\",\"version\":\"1.5.RC1.086\"}";
        ProtectedEnvironment protectedResource = JSON.parseObject(resource, ProtectedEnvironment.class);
        PowerMockito.when(resourceService.getResourceById(uuid)).thenReturn(Optional.ofNullable(protectedResource));
        ProtectedEnvironment environment = oceanBaseService.getEnvironmentById(uuid);
        Assert.assertEquals(uuid, environment.getUuid());
        Assert.assertEquals("192.168.129.12", environment.getEndpoint());
    }

    /**
     * 用例场景：查询集群信息成功
     * 前置条件：Pm环境正常
     * 检查点：返回资源信息
     */
    @Test
    public void query_cluster_info_success() {
        String environmentStr
            = "{\"linkStatus\":\"1\",\"name\":\"集群名称adfasgdsa\",\"rootUuid\":\"50f3bbcb-dff5-31d8-857c-2879f321a132\",\"sourceType\":\"register\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\",\"uuid\":\"50f3bbcb-dff5-31d8-857c-2879f321a132\",\"version\":\"3.2.4\",\"extendInfo\":{\"clusterInfo\":\"{\\\"obServerAgents\\\":[{\\\"parentUuid\\\":\\\"8796bfa6-e9ad-41e3-91f2-111111\\\",\\\"ip\\\":\\\"8.40.129.26\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\",\\\"linkStatus\\\":\\\"1\\\"}],\\\"obClientAgents\\\":[{\\\"parentUuid\\\":\\\"3d6544b6-23e9-44bc-bcab-222222\\\",\\\"nodeType\\\":\\\"OBClient\\\",\\\"linkStatus\\\":\\\"1\\\"}]}\"}}";
        ProtectedEnvironment environment = JSON.parseObject(environmentStr, ProtectedEnvironment.class);

        String agentUuid = "3d6544b6-23e9-44bc-bcab-222222";
        String resource
            = "{\"cluster\":false,\"createdTime\":\"2023-07-08 15:09:41.0\",\"endpoint\":\"192.168.129.12\",\"extendInfo\":{\"agent_last_update_time\":\"1689127212483\",\"agentIpList\":\"192.168.129.12,8.40.129.12,192.168.122.1,172.17.0.1,fe80::5f66:16a8:fbf9:591e,fe80::1649:562a:f5cf:7fe6\",\"agent_domain_available_ip\":\"protectengine-1.protectengine.dpa.svc.cluster.local,protectengine-0.protectengine.dpa.svc.cluster.local\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_960581fbd7244a1e94b6cdad7159c02c\":\"50f3bbcb-dff5-31d8-857c-2879f321a132\"},\"linkStatus\":\"1\",\"name\":\"localhost.localdomain\",\"osName\":\"linux\",\"osType\":\"linux\",\"port\":59527,\"protectionStatus\":0,\"rootUuid\":\"3d6544b6-23e9-44bc-bcab-de72a3d21682\",\"scanInterval\":3600,\"subType\":\"UBackupAgent\",\"type\":\"Host\",\"username\":\"\",\"uuid\":\"3d6544b6-23e9-44bc-bcab-de72a3d21682\",\"version\":\"1.5.RC1.086\"}";
        ProtectedEnvironment protectedResource = JSON.parseObject(resource, ProtectedEnvironment.class);
        PowerMockito.when(resourceService.getResourceById(agentUuid))
            .thenReturn(Optional.ofNullable(protectedResource));

        OBClusterInfo info = mockGetDetailPageList();

        OBClusterInfo clusterInfo = oceanBaseService.queryClusterInfo(environment);

        Assert.assertEquals(info.getTenantInfos(), clusterInfo.getTenantInfos());
        Assert.assertEquals(info.getVersion(), clusterInfo.getVersion());
    }

    private OBClusterInfo mockGetDetailPageList() {
        ProtectedResource getDetail = new ProtectedResource();
        Map<String, String> extendInfo = new HashMap<>();
        OBClusterInfo info = new OBClusterInfo();
        info.setTenantInfos(Lists.newArrayList(new OBTenantInfo("tenant1"), new OBTenantInfo("tenant3")));
        info.setVersion("3.2.4");
        extendInfo.put("clusterInfo", JSON.toJSONString(info));
        getDetail.setExtendInfo(extendInfo);
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(Lists.newArrayList(getDetail));

        PowerMockito.when(agentUnifiedService.getDetailPageListNoRetry(any(), any(), any(), any(), false))
            .thenReturn(response);
        return info;
    }

    /**
     * 用例场景：查询已注册集群成功
     * 前置条件：Pm环境正常
     * 检查点：返回资源信息
     */
    @Test
    public void get_exist_cluster_success() {
        String excludeUuid = "3d6544b6-23e9-44bc-bcab-de72a3d21682";
        Map<String, String> map = Maps.newHashMap();
        map.put("clusterInfo",
            "{\"obServerAgents\":[{\"parentUuid\":\"8796bfa6-e9ad-41e3-91f2-111111\",\"ip\":\"8.40.129.26\",\"port\":\"2881\",\"nodeType\":\"OBServer\"}],\"obClientAgents\":[{\"parentUuid\":\"3d6544b6-23e9-44bc-bcab-222222\",\"nodeType\":\"OBClient\"}]}");

        ProtectedEnvironment environment1 = new ProtectedEnvironment();
        environment1.setUuid(excludeUuid);
        environment1.setExtendInfo(map);

        Map<String, String> map2 = Maps.newHashMap();
        map2.put("clusterInfo",
            "{\"obServerAgents\":[{\"parentUuid\":\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\",\"ip\":\"8.40.129.26\",\"port\":\"2881\",\"nodeType\":\"OBServer\"}],\"obClientAgents\":[{\"parentUuid\":\"3d6544b6-23e9-44bc-bcab-de72a3d21682\",\"nodeType\":\"OBClient\"}]}");

        ProtectedEnvironment environment2 = new ProtectedEnvironment();
        environment2.setUuid("50f3bbcb-dff5-31d8-857c-2879f321a132");
        environment2.setExtendInfo(map2);
        List<ProtectedResource> existingResources = Lists.newArrayList(environment1, environment2);

        Map<String, Object> filter = new HashMap<>();
        filter.put(DatabaseConstants.RESOURCE_TYPE, ResourceTypeEnum.DATABASE.getType());
        filter.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.OCEAN_BASE_CLUSTER.getType());

        PageListResponse<ProtectedResource> pageList = new PageListResponse<>();
        pageList.setRecords(existingResources);

        PowerMockito.when(resourceService.query(0, OBConstants.OB_CLUSTER_MAX_COUNT, filter)).thenReturn(pageList);

        List<String> list = oceanBaseService.getExistingOceanBaseCluster(excludeUuid);

        Assert.assertEquals(Lists.newArrayList("8796bfa6-e9ad-41e3-91f2-93af637ebf98"), list);
    }

    /**
     * 用例场景：查询已注册的租戶成功。 数据库已有租户集1、租户集2，查询租户时， 排除租户集1。
     * 前置条件：Pm环境正常
     * 检查点：返回资源信息
     */
    @Test
    public void get_exist_tenant_success() {
        OBClusterInfo info = new OBClusterInfo();
        info.setTenantInfos(Lists.newArrayList(new OBTenantInfo("tenant1"), new OBTenantInfo("tenant3")));

        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("clusterInfo", JSON.toJSONString(info));

        ProtectedResource resource1 = new ProtectedResource();
        resource1.setName("租户集1");
        resource1.setUuid("1111111111111111111");
        resource1.setExtendInfo(extendInfo);

        OBClusterInfo info2 = new OBClusterInfo();
        info2.setTenantInfos(Lists.newArrayList(new OBTenantInfo("tenant2")));

        Map<String, String> extendInfo2 = new HashMap<>();
        extendInfo2.put("clusterInfo", JSON.toJSONString(info2));

        ProtectedResource resource2 = new ProtectedResource();
        resource1.setName("租户集2");
        resource2.setUuid("222222222222222222");
        resource2.setExtendInfo(extendInfo2);

        List<ProtectedResource> existingResources = Lists.newArrayList(resource1, resource2);
        PageListResponse<ProtectedResource> pageList = new PageListResponse<>();
        pageList.setRecords(existingResources);

        Map<String, Object> filter = new HashMap<>();
        filter.put(DatabaseConstants.RESOURCE_TYPE, ResourceTypeEnum.DATABASE.getType());
        filter.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.OCEAN_BASE_TENANT.getType());
        String parentUuid = "3d6544b6-23e9-44bc-bcab-de72a3d21682";
        filter.put(DatabaseConstants.PARENT_UUID, parentUuid);

        PowerMockito.when(resourceService.query(0, OBConstants.OB_CLUSTER_MAX_COUNT, filter)).thenReturn(pageList);

        List<String> list = oceanBaseService.getExistingOceanBaseTenant(parentUuid, "1111111111111111111");
        Assert.assertEquals(Lists.newArrayList("tenant2"), list);
    }

    /**
     * 用例场景：更新extendInfo成功
     * 前置条件：Pm环境正常
     * 检查点：无异常
     */
    @Test
    public void check_update_extend_info_success() {
        String resourceStr
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"123456\",\"authType\":2,\"extendInfo\":{}},\"extendInfo\":{\"clusterInfo\":\"{\\\"obServerAgents\\\":[{\\\"parentUuid\\\":\\\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\\\",\\\"ip\\\":\\\"8.40.129.26\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\",\\\"linkStatus\\\":\\\"1\\\"}],\\\"obClientAgents\\\":[{\\\"parentUuid\\\":\\\"3d6544b6-23e9-44bc-bcab-de72a3d21682\\\",\\\"nodeType\\\":\\\"OBClient\\\",\\\"linkStatus\\\":\\\"1\\\"}]}\",\"type\":\"health\"},\"name\":\"集群名称adfasgdsa\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\",\"uuid\":\"11111\"}";
        ProtectedResource resource = JSON.parseObject(resourceStr, ProtectedResource.class);

        oceanBaseService.updateExtendInfo(Lists.newArrayList(resource));

        // 校验请求更新的参数是否满足要求
        verify(resourceService).updateSourceDirectly(argThat(matchResource()));
    }

    private ArgumentMatcher<List<ProtectedResource>> matchResource() {
        return new ArgumentMatcher<List<ProtectedResource>>() {
            @Override
            public boolean matches(List<ProtectedResource> argument) {
                if (argument.size() != 1) {
                    return false;
                }
                ProtectedResource resource = argument.get(0);

                // 请求参数中应只有uuid和extendInfo
                ProtectedResource resource2 = new ProtectedResource();
                resource2.setUuid(resource.getUuid());
                resource2.setExtendInfo(resource.getExtendInfo());

                return Objects.equals(JSON.toJSONString(resource), JSON.toJSONString(resource2));
            }
        };
    }

    /**
     * 用例场景：检查租户集状态
     * 前置条件：Pm环境正常
     * 检查点：无异常
     */
    @Test
    public void check_tenant_set_connect_success() {
        String resourceStr
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"123456\",\"authType\":2,\"extendInfo\":{}},\"extendInfo\":{\"clusterInfo\":\"{\\\"obServerAgents\\\":[{\\\"parentUuid\\\":\\\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\\\",\\\"ip\\\":\\\"8.40.129.26\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\",\\\"linkStatus\\\":\\\"1\\\"}],\\\"obClientAgents\\\":[{\\\"parentUuid\\\":\\\"3d6544b6-23e9-44bc-bcab-de72a3d21682\\\",\\\"nodeType\\\":\\\"OBClient\\\",\\\"linkStatus\\\":\\\"1\\\"}]}\",\"type\":\"health\"},\"name\":\"集群名称adfasgdsa\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\",\"uuid\":\"11111\"}";
        ProtectedEnvironment environment = JSON.parseObject(resourceStr, ProtectedEnvironment.class);

        // 要检查的租户集中有3个租户， tenant1, tenant2, tenant3
        String tenantStr
            = "{\"uuid\":\"3d6544b6-23e9-44bc-bcab-de72a3d21682\",\"name\":\"test3\",\"type\":\"Database\",\"subType\":\"OceanBase-tenant\",\"parentName\":null,\"parentUuid\":\"50f3bbcb-dff5-31d8-857c-2879f321a132\",\"rootUuid\":\"50f3bbcb-dff5-31d8-857c-2879f321a132\",\"sourceType\":\"register\",\"extendInfo\":{\"linkStatus\":\"0\",\"clusterInfo\":\"{\\\"tenantInfos\\\":[{\\\"name\\\":\\\"tenant1\\\"}, {\\\"name\\\":\\\"tenant2\\\"}, {\\\"name\\\":\\\"tenant3\\\"}]}\"},\"dependencies\":null}";
        ProtectedEnvironment tenantEnv = JSON.parseObject(tenantStr, ProtectedEnvironment.class);

        PowerMockito.when(resourceService.getResourceById(tenantEnv.getParentUuid()))
            .thenReturn(Optional.ofNullable(environment));
        PowerMockito.when(resourceService.getResourceById(tenantEnv.getUuid())).thenReturn(Optional.of(tenantEnv));
        // 实际集群中只有2个租户 tenant1, tenant3
        mockGetDetailPageList();

        oceanBaseService.checkTenantSetConnect(tenantEnv);

        // tenant1, tenant3的linkStatus应为ONLINE， tenant2的linkStatus应为OFFLINE
        List<OBTenantInfo> infos = OceanBaseUtils.readExtendClusterInfo(tenantEnv).getTenantInfos();
        for (OBTenantInfo info : infos) {
            switch (info.getName()) {
                case "tenant1":
                case "tenant3":
                    Assert.assertEquals(info.getLinkStatus(), LinkStatusEnum.ONLINE.getStatus().toString());
                    break;
                case "tenant2":
                    Assert.assertEquals(info.getLinkStatus(), LinkStatusEnum.OFFLINE.getStatus().toString());
                    break;
                default:
            }
        }

        // 租户集状态为 OFFLINE
        Assert.assertEquals(tenantEnv.getExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY),
            LinkStatusEnum.OFFLINE.getStatus().toString());
    }

    /**
     * 用例场景：移除资源的数据存储仓白名单
     * 前置条件：Pm环境正常
     * 检查点：无异常
     */
    @Test
    public void removeDataRepoWhiteListOfResource_success() {
        Mockito.doNothing().when(dmeUnifiedRestApi).removeRepoWhiteListOfResource(any());
        oceanBaseService.removeDataRepoWhiteListOfResource("123");
    }

    /**
     * 用例场景：日志仓解挂载
     * 前置条件：Pm环境正常
     * 检查点：无异常
     */
    @Test
    public void test_umountDataRepo_success() {
        String param
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"123456\",\"authType\":2},\"dependencies\":{\"obClientAgents\":[{\"uuid\":\"3d6544b6-23e9-44bc-bcab-de72a3d21682\"}],\"obServerAgents\":[{\"uuid\":\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\"}]},\"extendInfo\":{\"clusterInfo\":\"{\\\"obServerAgents\\\":[{\\\"parentUuid\\\":\\\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\\\",\\\"ip\\\":\\\"8.40.129.26\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\"}],\\\"obClientAgents\\\":[{\\\"parentUuid\\\":\\\"3d6544b6-23e9-44bc-bcab-de72a3d21682\\\",\\\"nodeType\\\":\\\"OBClient\\\"}]}\"},\"name\":\"集群名称adfasgdsa\",\"port\":0,\"scanInterval\":3600,\"sourceType\":\"register\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\"}";
        ProtectedResource protectedResource = JSON.parseObject(param, ProtectedResource.class);
        OBClusterInfo obClusterInfo = OceanBaseUtils.readExtendClusterInfo(protectedResource);
        ProtectedEnvironment dbRecord = new ProtectedEnvironment();
        dbRecord.setParentUuid(protectedResource.getParentUuid());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(dbRecord));

        PowerMockito.doNothing().when(agentUnifiedService).removeProtectUnmountRepoNoRetry(any(), any(), any(), any());
        oceanBaseService.umountDataRepo(obClusterInfo, protectedResource);
    }

    /**
     * 用例场景：调DeviceManager查询NFSv4.1服务，校验成功
     * 前置条件：Pm环境正常
     * 检查点：无异常
     */
    @Test
    public void test_checkSupportNFSV41_success() {
        PowerMockito.when(clusterBasicService.getCurrentClusterEsn()).thenReturn("DeviceId");
        String object = "{\"data\":{\"SUPPORTV41\":\"true\"},\"error\":{\"code\":0,\"description\":\"\"}}";
        PowerMockito.when(nfsServiceApi.getNfsServiceConfig(anyString(), anyString())).thenReturn(object);
        oceanBaseService.checkSupportNFSV41();
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：调DeviceManager查询NFSv4.1服务，未返回结果，校验失败
     * 前置条件：Pm环境正常
     * 检查点：抛出异常
     */
    @Test
    public void test_checkSupportNFSV41_error_if_dm_response_is_empty() {
        PowerMockito.when(clusterBasicService.getCurrentClusterEsn()).thenReturn("DeviceId");
        PowerMockito.when(nfsServiceApi.getNfsServiceConfig(anyString(), anyString())).thenReturn(null);
        Assert.assertThrows(LegoCheckedException.class, () -> oceanBaseService.checkSupportNFSV41());
    }

    /**
     * 用例场景：DeviceManager未开启NFSv4.1服务，校验失败
     * 前置条件：Pm环境正常
     * 检查点：抛出异常
     */
    @Test
    public void test_checkSupportNFSV41_error_if_not_support_V41() {
        PowerMockito.when(clusterBasicService.getCurrentClusterEsn()).thenReturn("DeviceId");
        String object = "{\"data\":{\"SUPPORTV41\":\"false\"},\"error\":{\"code\":0,\"description\":\"\"}}";
        PowerMockito.when(nfsServiceApi.getNfsServiceConfig(anyString(), anyString())).thenReturn(object);
        Assert.assertThrows(LegoCheckedException.class, () -> oceanBaseService.checkSupportNFSV41());
    }
}
