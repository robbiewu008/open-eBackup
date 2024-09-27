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
package openbackup.oceanbase.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.inOrder;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;

import openbackup.access.framework.resource.service.provider.UnifiedConnectionCheckProvider;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.oceanbase.common.constants.OBConstants;
import openbackup.oceanbase.common.constants.OBErrorCodeConstants;
import openbackup.oceanbase.common.dto.OBClusterInfo;
import openbackup.oceanbase.common.dto.OBTenantInfo;
import openbackup.oceanbase.common.util.OceanBaseUtils;
import openbackup.oceanbase.service.OceanBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.service.DeployTypeService;
import com.huawei.oceanprotect.system.base.user.common.constant.UserErrorCode;

import com.alibaba.fastjson.JSON;
import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InOrder;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-05
 */
@RunWith(PowerMockRunner.class)
public class OceanBaseClusterProviderTest {
    private static final String ONLINE = LinkStatusEnum.ONLINE.getStatus().toString();

    private static final String OFFLINE = LinkStatusEnum.OFFLINE.getStatus().toString();

    private UnifiedConnectionCheckProvider unifiedConnectionCheckProvider;

    private OceanBaseService oceanBaseService;

    private ProviderManager providerManager;

    private OceanBaseClusterProvider oceanBaseClusterProvider;

    private DeployTypeService deployTypeService;

    @Before
    public void init() {
        oceanBaseService = Mockito.mock(OceanBaseService.class);
        providerManager = Mockito.mock(ProviderManager.class);
        unifiedConnectionCheckProvider = mock(UnifiedConnectionCheckProvider.class);
        deployTypeService = mock(DeployTypeService.class);
        PluginConfigManager pluginConfigManager = Mockito.mock(PluginConfigManager.class);
        oceanBaseClusterProvider = new OceanBaseClusterProvider(providerManager, pluginConfigManager, oceanBaseService,
            deployTypeService);
    }

    /**
     * 用例场景：注册集群成功
     * 前置条件：传入注册信息正确
     * 检查点：生成endpoints/version/uuid正确
     */
    @Test
    @Ignore
    public void register_check_success() {
        String param
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"123456\",\"authType\":2},\"dependencies\":{\"clientAgents\":[{\"uuid\":\"3d6544b6-23e9-44bc-bcab-de72a3d21682\"}],\"serverAgents\":[{\"uuid\":\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\"}]},\"extendInfo\":{\"clusterInfo\":\"{\\\"obServerAgents\\\":[{\\\"parentUuid\\\":\\\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\\\",\\\"ip\\\":\\\"8.40.129.26\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\"}],\\\"obClientAgents\\\":[{\\\"parentUuid\\\":\\\"3d6544b6-23e9-44bc-bcab-de72a3d21682\\\",\\\"nodeType\\\":\\\"OBClient\\\"}]}\"},\"name\":\"集群名称adfasgdsa\",\"port\":0,\"scanInterval\":3600,\"sourceType\":\"register\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\"}";
        ProtectedEnvironment environment = JSON.parseObject(param, ProtectedEnvironment.class);
        OBClusterInfo clusterInfo = OceanBaseUtils.readExtendClusterInfo(environment);
        String obClientUuid = clusterInfo.getObClientAgents().get(0).getParentUuid();
        String obServerUuid = clusterInfo.getObServerAgents().get(0).getParentUuid();
        mockAgent(obClientUuid, obServerUuid);

        PowerMockito.when(oceanBaseService.queryClusterInfo(any())).thenReturn(generateVersionResult());
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(unifiedConnectionCheckProvider);
        ResourceCheckContext checkContext = getResourceCheckContext();
        PowerMockito.when(unifiedConnectionCheckProvider.checkConnection(any())).thenReturn(checkContext);
        oceanBaseClusterProvider.register(environment);
        InOrder inOrder = inOrder(oceanBaseService, unifiedConnectionCheckProvider);

        inOrder.verify(oceanBaseService).getEnvironmentById(obServerUuid);
        inOrder.verify(oceanBaseService).getExistingOceanBaseCluster(null);
        inOrder.verify(oceanBaseService).getEnvironmentById(obClientUuid);
        inOrder.verify(unifiedConnectionCheckProvider).checkConnection(any());
        inOrder.verify(oceanBaseService).queryClusterInfo(any());
        Assert.assertEquals(environment.getEndpoint(), "192.168.129.12,192.168.129.26");
        Assert.assertEquals(environment.getVersion(), "3.2.3");
        Assert.assertEquals(environment.getUuid(), "50f3bbcb-dff5-31d8-857c-2879f321a132");
        Assert.assertEquals(environment.getLinkStatus(), ONLINE);

        clusterInfo = OceanBaseUtils.readExtendClusterInfo(environment);
        Assert.assertTrue(
            clusterInfo.getObServerAgents().stream().noneMatch(item -> Objects.equals(item.getLinkStatus(), OFFLINE)));
    }

    /**
     * 用例场景：连通性检查失败，注册失败
     * 前置条件：传入注册信息正确
     * 检查点：生成endpoints/version/uuid正确
     */
    @Test
    @Ignore
    public void register_check_failed_when_check_connect_error() {
        String param
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"123456\",\"authType\":2},\"extendInfo\":{\"clusterInfo\":\"{\\\"obServerAgents\\\":[{\\\"parentUuid\\\":\\\"11111\\\",\\\"ip\\\":\\\"8.40.129.26\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\"},{\\\"parentUuid\\\":\\\"22222\\\",\\\"ip\\\":\\\"8.40.129.27\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\"}],\\\"obClientAgents\\\":[{\\\"parentUuid\\\":\\\"33333\\\",\\\"nodeType\\\":\\\"OBClient\\\"},{\\\"parentUuid\\\":\\\"44444\\\",\\\"nodeType\\\":\\\"OBClient\\\"}]}\"},\"name\":\"集群名称adfasgdsa\",\"port\":0,\"scanInterval\":3600,\"sourceType\":\"register\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\"}";
        ProtectedEnvironment environment = JSON.parseObject(param, ProtectedEnvironment.class);
        OBClusterInfo clusterInfo = OceanBaseUtils.readExtendClusterInfo(environment);
        String obClientUuid = clusterInfo.getObClientAgents().get(0).getParentUuid();
        String obServerUuid = clusterInfo.getObServerAgents().get(0).getParentUuid();
        String obClientUuid2 = clusterInfo.getObClientAgents().get(1).getParentUuid();
        String obServerUuid2 = clusterInfo.getObServerAgents().get(1).getParentUuid();
        mockAgent(obClientUuid, obServerUuid);
        mockAgent2(obClientUuid2, obServerUuid2);

        PowerMockito.when(oceanBaseService.queryClusterInfo(any())).thenReturn(generateVersionResult());
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(unifiedConnectionCheckProvider);
        PowerMockito.when(unifiedConnectionCheckProvider.checkConnection(any()))
            .thenThrow(new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR));
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> oceanBaseClusterProvider.register(environment));
        InOrder inOrder = inOrder(oceanBaseService, unifiedConnectionCheckProvider);

        inOrder.verify(oceanBaseService).getEnvironmentById(obServerUuid);
        inOrder.verify(oceanBaseService).getEnvironmentById(obServerUuid2);
        inOrder.verify(oceanBaseService).getExistingOceanBaseCluster(null);
        inOrder.verify(oceanBaseService).getEnvironmentById(obClientUuid);
        inOrder.verify(oceanBaseService).getEnvironmentById(obClientUuid2);
        inOrder.verify(unifiedConnectionCheckProvider).checkConnection(any());
        inOrder.verify(oceanBaseService, never()).queryClusterInfo(any());
        Assert.assertEquals(environment.getEndpoint(), "192.168.129.12,192.168.129.13,192.168.129.26,192.168.129.27");
        Assert.assertEquals(environment.getUuid(), "d568bd5a-8d93-3ad6-8e67-a7875c8c315e");
        Assert.assertEquals(legoCheckedException.getErrorCode(), CommonErrorCode.AGENT_NETWORK_ERROR);
    }

    private void mockAgent(String obClientUuid, String obServerUuid) {
        ProtectedEnvironment agent1 = new ProtectedEnvironment();
        agent1.setEndpoint("192.168.129.12");
        agent1.setUuid(obClientUuid);
        PowerMockito.when(oceanBaseService.getEnvironmentById(obClientUuid)).thenReturn(agent1);

        ProtectedEnvironment agent2 = new ProtectedEnvironment();
        agent2.setEndpoint("192.168.129.26");
        agent2.setUuid(obServerUuid);
        PowerMockito.when(oceanBaseService.getEnvironmentById(obServerUuid)).thenReturn(agent2);
    }

    private void mockAgent2(String obClientUuid, String obServerUuid) {
        ProtectedEnvironment agent1 = new ProtectedEnvironment();
        agent1.setEndpoint("192.168.129.13");
        agent1.setUuid(obClientUuid);
        PowerMockito.when(oceanBaseService.getEnvironmentById(obClientUuid)).thenReturn(agent1);

        ProtectedEnvironment agent2 = new ProtectedEnvironment();
        agent2.setEndpoint("192.168.129.27");
        agent2.setUuid(obServerUuid);
        PowerMockito.when(oceanBaseService.getEnvironmentById(obServerUuid)).thenReturn(agent2);
    }

    /**
     * 用例场景：修改集群成功
     * 前置条件：传入注册信息正确
     * 检查点：生成endpoints/version/uuid正确
     */
    @Test
    public void update_check_success() {
        String param
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"123456\",\"authType\":2,\"extendInfo\":{}},\"createdTime\":\"2023-07-11 15:45:48.93\",\"dependencies\":{\"clientAgents\":[{\"uuid\":\"3d6544b6-23e9-44bc-bcab-de72a3d21682\"}],\"serverAgents\":[{\"uuid\":\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\"}]},\"extendInfo\":{\"clusterInfo\":\"{\\\"obServerAgents\\\":[{\\\"parentUuid\\\":\\\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\\\",\\\"ip\\\":\\\"8.40.129.26\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\"}],\\\"obClientAgents\\\":[{\\\"parentUuid\\\":\\\"3d6544b6-23e9-44bc-bcab-de72a3d21682\\\",\\\"nodeType\\\":\\\"OBClient\\\"}]}\"},\"linkStatus\":\"1\",\"name\":\"test修改名称2\",\"port\":0,\"protectionStatus\":0,\"rootUuid\":\"50f3bbcb-dff5-31d8-857c-2879f321a132\",\"scanInterval\":3600,\"sourceType\":\"register\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\",\"uuid\":\"50f3bbcb-dff5-31d8-857c-2879f321a132\",\"version\":\"3.2.4\"}";
        ProtectedEnvironment environment = JSON.parseObject(param, ProtectedEnvironment.class);
        OBClusterInfo clusterInfo = OceanBaseUtils.readExtendClusterInfo(environment);
        String obClientUuid = clusterInfo.getObClientAgents().get(0).getParentUuid();
        String obServerUuid = clusterInfo.getObServerAgents().get(0).getParentUuid();
        mockAgent(obClientUuid, obServerUuid);

        PowerMockito.when(oceanBaseService.queryClusterInfo(any())).thenReturn(generateVersionResult());
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(unifiedConnectionCheckProvider);

        ResourceCheckContext checkContext = getResourceCheckContext();
        PowerMockito.when(unifiedConnectionCheckProvider.checkConnection(any())).thenReturn(checkContext);

        oceanBaseClusterProvider.register(environment);
        InOrder inOrder = inOrder(oceanBaseService, unifiedConnectionCheckProvider);
        inOrder.verify(oceanBaseService).getEnvironmentById(obServerUuid);
        inOrder.verify(oceanBaseService).getExistingOceanBaseCluster(environment.getUuid());
        inOrder.verify(oceanBaseService).getEnvironmentById(obClientUuid);
        inOrder.verify(unifiedConnectionCheckProvider).checkConnection(any());
        inOrder.verify(oceanBaseService).queryClusterInfo(any());

        Assert.assertEquals(environment.getEndpoint(), "192.168.129.12,192.168.129.26");
        Assert.assertEquals(environment.getVersion(), "3.2.3");
        Assert.assertEquals(environment.getUuid(), "50f3bbcb-dff5-31d8-857c-2879f321a132");
    }

    private static ResourceCheckContext getResourceCheckContext() {
        Map<String, Object> map = new HashMap<>();
        map.put(OBConstants.CONTENT_KEY_CONNECT_RESULT,
            "[{\"results\":[{\"environment\":{\"uuid\":\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\",\"extendInfo\":{\"checkType\":\"check_observer\"},\"endpoint\":\"192.168.160.251\",\"port\":59535,\"linkStatus\":\"1\"},\"results\":{\"code\":0,\"bodyErr\":\"0\",\"message\":\"Check connection success!\"}},{\"environment\":{\"uuid\":\"3d6544b6-23e9-44bc-bcab-de72a3d21682\",\"extendInfo\":{\"checkType\":\"check_obclient\"},\"endpoint\":\"192.168.160.251\",\"port\":59535,\"linkStatus\":\"1\"},\"results\":{\"code\":0,\"bodyErr\":\"0\",\"message\":\"Check connection success!\"}}]}]");

        ResourceCheckContext checkContext = new ResourceCheckContext();
        checkContext.setContext(map);
        return checkContext;
    }

    /**
     * 用例场景：注册集群参数，server节点有重复
     * 前置条件：传入参数server节点重复
     * 检查点：返回参数错误异常
     */
    @Test
    public void register_check_duplicate_server_node_error() {
        String param
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"123456\",\"authType\":2},\"extendInfo\":{\"clusterInfo\":\"{\\\"obServerAgents\\\":[{\\\"parentUuid\\\":\\\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\\\",\\\"ip\\\":\\\"8.40.129.26\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\"},{\\\"parentUuid\\\":\\\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\\\",\\\"ip\\\":\\\"8.40.129.26\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\"}],\\\"obClientAgents\\\":[{\\\"parentUuid\\\":\\\"3d6544b6-23e9-44bc-bcab-de72a3d21682\\\",\\\"nodeType\\\":\\\"OBClient\\\"}]}\"},\"name\":\"集群名称adfasgdsa\",\"port\":0,\"scanInterval\":3600,\"sourceType\":\"register\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\"}";
        ProtectedEnvironment environment = JSON.parseObject(param, ProtectedEnvironment.class);
        OBClusterInfo clusterInfo = OceanBaseUtils.readExtendClusterInfo(environment);
        String obClientUuid = clusterInfo.getObClientAgents().get(0).getParentUuid();
        String obServerUuid = clusterInfo.getObServerAgents().get(0).getParentUuid();
        mockAgent(obClientUuid, obServerUuid);

        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> oceanBaseClusterProvider.register(environment));

        Assert.assertEquals("obServer agents are duplicate", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：注册集群参数，server节点为空
     * 前置条件：传入参数server节点为空
     * 检查点：返回参数错误异常
     */
    @Test
    public void register_check_empty_server_node_error() {
        // OBServer节点不存在
        String param
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"123456\",\"authType\":2},\"extendInfo\":{\"clusterInfo\":\"{\\\"obClientAgents\\\":[{\\\"parentUuid\\\":\\\"3d6544b6-23e9-44bc-bcab-de72a3d21682\\\",\\\"nodeType\\\":\\\"OBClient\\\"}]}\"},\"name\":\"集群名称adfasgdsa\",\"port\":0,\"scanInterval\":3600,\"sourceType\":\"register\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\"}";
        ProtectedEnvironment environment = JSON.parseObject(param, ProtectedEnvironment.class);
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> oceanBaseClusterProvider.register(environment));

        Assert.assertEquals("OBServer node is empty", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, legoCheckedException.getErrorCode());

        // OBServer节点中内容为空
        param
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"123456\",\"authType\":2},\"extendInfo\":{\"clusterInfo\":\"{\\\"obServerAgents\\\":[],\\\"obClientAgents\\\":[{\\\"parentUuid\\\":\\\"3d6544b6-23e9-44bc-bcab-de72a3d21682\\\",\\\"nodeType\\\":\\\"OBClient\\\"}]}\"},\"name\":\"集群名称adfasgdsa\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\"}";
        ProtectedEnvironment environment2 = JSON.parseObject(param, ProtectedEnvironment.class);
        legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> oceanBaseClusterProvider.register(environment2));

        Assert.assertEquals("OBServer node is empty", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：注册集群参数，client节点有重复
     * 前置条件：传入参数client节点重复
     * 检查点：返回参数错误异常
     */
    @Test
    public void register_check_duplicate_client_node_error() {
        String param
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"HUAwei@@123\",\"authType\":2},\"extendInfo\":{\"clusterInfo\":\"{\\\"obServerAgents\\\":[{\\\"parentUuid\\\":\\\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\\\",\\\"ip\\\":\\\"8.40.129.26\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\"}],\\\"obClientAgents\\\":[{\\\"parentUuid\\\":\\\"3d6544b6-23e9-44bc-bcab-de72a3d21682\\\",\\\"nodeType\\\":\\\"OBClient\\\"},{\\\"parentUuid\\\":\\\"3d6544b6-23e9-44bc-bcab-de72a3d21682\\\",\\\"nodeType\\\":\\\"OBClient\\\"}]}\"},\"name\":\"集群名称adfasgdsa\",\"port\":0,\"scanInterval\":3600,\"sourceType\":\"register\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\"}";
        ProtectedEnvironment environment = JSON.parseObject(param, ProtectedEnvironment.class);
        OBClusterInfo clusterInfo = OceanBaseUtils.readExtendClusterInfo(environment);
        String obClientUuid = clusterInfo.getObClientAgents().get(0).getParentUuid();
        String obServerUuid = clusterInfo.getObServerAgents().get(0).getParentUuid();
        mockAgent(obClientUuid, obServerUuid);

        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> oceanBaseClusterProvider.register(environment));

        Assert.assertEquals("obServer agents are duplicate", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：注册集群参数，server节点中有节点是其他注册集群中已经注册过的节点
     * 前置条件：传入参数server节点中有节点是其他注册集群中已经注册过的节点
     * 检查点：返回参数错误异常
     */
    @Test
    public void register_check_server_node_is_used_error() {
        String param
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"123456\",\"authType\":2},\"extendInfo\":{\"clusterInfo\":\"{\\\"obServerAgents\\\":[{\\\"parentUuid\\\":\\\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\\\",\\\"ip\\\":\\\"8.40.129.26\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\"}],\\\"obClientAgents\\\":[{\\\"parentUuid\\\":\\\"3d6544b6-23e9-44bc-bcab-de72a3d21682\\\",\\\"nodeType\\\":\\\"OBClient\\\"}]}\"},\"name\":\"集群名称adfasgdsa\",\"port\":0,\"scanInterval\":3600,\"sourceType\":\"register\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\"}";
        ProtectedEnvironment environment = JSON.parseObject(param, ProtectedEnvironment.class);
        OBClusterInfo clusterInfo = OceanBaseUtils.readExtendClusterInfo(environment);
        String obClientUuid = clusterInfo.getObClientAgents().get(0).getParentUuid();
        String obServerUuid = clusterInfo.getObServerAgents().get(0).getParentUuid();
        mockAgent(obClientUuid, obServerUuid);

        PowerMockito.when(oceanBaseService.getExistingOceanBaseCluster(environment.getUuid()))
            .thenReturn(Collections.singletonList(obServerUuid));

        PowerMockito.when(oceanBaseService.queryClusterInfo(any())).thenReturn(generateVersionResult());

        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> oceanBaseClusterProvider.register(environment));

        Assert.assertEquals("The OBServer agent id: 8796bfa6-e9ad-41e3-91f2-93af637ebf98 already exist.",
            legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.CLUSTER_NODE_IS_REGISTERED, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：注册集群参数，server节点中IP不符合格式
     * 前置条件：传入参数server节点IP不符合格式
     * 检查点：返回参数错误异常
     */
    @Test
    public void register_check_server_node_ip_invalid_error() {
        String param
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"HUAwei@@123\",\"authType\":2},\"extendInfo\":{\"clusterInfo\":\"{\\\"obServerAgents\\\":[{\\\"parentUuid\\\":\\\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\\\",\\\"ip\\\":\\\"8.40.129.300\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\"}],\\\"obClientAgents\\\":[{\\\"parentUuid\\\":\\\"3d6544b6-23e9-44bc-bcab-de72a3d21682\\\",\\\"nodeType\\\":\\\"OBClient\\\"}]}\"},\"name\":\"集群名称adfasgdsa\",\"port\":0,\"scanInterval\":3600,\"sourceType\":\"register\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\"}";
        ProtectedEnvironment environment = JSON.parseObject(param, ProtectedEnvironment.class);

        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> oceanBaseClusterProvider.register(environment));

        Assert.assertEquals(UserErrorCode.IP_OR_IP_SEGMENT_INVALID, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：注册集群参数，client节点为空
     * 前置条件：传入参数client节点为空
     * 检查点：返回参数错误异常
     */
    @Test
    public void register_check_empty_client_node_error() {
        // OBClient节点整个没有
        String param
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"123456\",\"authType\":2},\"extendInfo\":{\"clusterInfo\":\"{\\\"obServerAgents\\\":[{\\\"parentUuid\\\":\\\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\\\",\\\"ip\\\":\\\"8.40.129.26\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\"}]}\"},\"name\":\"集群名称adfasgdsa\",\"port\":0,\"scanInterval\":3600,\"sourceType\":\"register\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\"}";
        ProtectedEnvironment environment = JSON.parseObject(param, ProtectedEnvironment.class);
        OBClusterInfo clusterInfo = OceanBaseUtils.readExtendClusterInfo(environment);
        String obServerUuid = clusterInfo.getObServerAgents().get(0).getParentUuid();

        ProtectedEnvironment agent2 = new ProtectedEnvironment();
        agent2.setEndpoint("192.168.129.26");
        agent2.setUuid(obServerUuid);
        PowerMockito.when(oceanBaseService.getEnvironmentById(obServerUuid)).thenReturn(agent2);

        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> oceanBaseClusterProvider.register(environment));

        Assert.assertEquals("OBClient node is empty", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, legoCheckedException.getErrorCode());

        // OBClient节点中的各项数据为空
        param
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"123456\",\"authType\":2,\"extendInfo\":{}},\"extendInfo\":{\"clusterInfo\":\"{\\\"obServerAgents\\\":[{\\\"parentUuid\\\":\\\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\\\",\\\"ip\\\":\\\"8.40.129.26\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\"}],\\\"obClientAgents\\\":[]}\"},\"name\":\"test修改名称2\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\"}";
        ProtectedEnvironment environment2 = JSON.parseObject(param, ProtectedEnvironment.class);
        legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> oceanBaseClusterProvider.register(environment2));

        Assert.assertEquals("OBClient node is empty", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：查询集群的租户列表成功
     * 前置条件：集群存在租户，集群运行正常
     * 检查点：返回租户列表
     */
    @Test
    public void query_tenants_success() {
        String param
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"123456\",\"authType\":2,\"extendInfo\":{}},\"cluster\":false,\"createdTime\":\"2023-07-12 09:59:28.102\",\"endpoint\":\"192.168.129.12,192.168.129.26\",\"extendInfo\":{\"clusterInfo\":\"{\\\"obServerAgents\\\":[{\\\"parentUuid\\\":\\\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\\\",\\\"ip\\\":\\\"8.40.129.26\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\"}],\\\"obClientAgents\\\":[{\\\"parentUuid\\\":\\\"3d6544b6-23e9-44bc-bcab-de72a3d21682\\\",\\\"nodeType\\\":\\\"OBClient\\\"}]}\"},\"linkStatus\":\"1\",\"name\":\"集群名称adfasgdsa\",\"port\":0,\"protectionStatus\":0,\"rootUuid\":\"50f3bbcb-dff5-31d8-857c-2879f321a132\",\"scanInterval\":3600,\"sourceType\":\"register\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\",\"uuid\":\"50f3bbcb-dff5-31d8-857c-2879f321a132\",\"version\":\"3.2.4\"}";
        ProtectedEnvironment environment = JSON.parseObject(param, ProtectedEnvironment.class);
        String conditionStr
            = "{\"envId\":\"50f3bbcb-dff5-31d8-857c-2879f321a132\",\"pageNo\":0,\"pageSize\":2000,\"resourceType\":\"OceanBase-tenant\"}";
        BrowseEnvironmentResourceConditions conditions = JSON.parseObject(conditionStr,
            BrowseEnvironmentResourceConditions.class);
        PowerMockito.when(oceanBaseService.getExistingOceanBaseTenant(environment.getUuid()))
            .thenReturn(Collections.singletonList("tenant3"));

        PowerMockito.when(oceanBaseService.queryClusterInfo(any())).thenReturn(generateVersionResult());

        PageListResponse<ProtectedResource> response = oceanBaseClusterProvider.browse(environment, conditions);
        Assert.assertEquals(2, response.getRecords().size());
        Assert.assertEquals("tenant1", response.getRecords().get(0).getName());
        Assert.assertEquals("tenant3", response.getRecords().get(1).getName());
        Assert.assertEquals("false", response.getRecords().get(0).getExtendInfo().get("isUsed"));
        Assert.assertEquals("true", response.getRecords().get(1).getExtendInfo().get("isUsed"));
    }

    /**
     * 用例场景：用户名或密码为空
     * 前置条件：OB服务运行正常
     * 检查点：返回异常
     */
    @Test
    public void register_check_username_empty_error() {
        String param
            = "{\"auth\":{\"authKey\":\"\",\"authPwd\":\"123456\",\"authType\":2},\"extendInfo\":{\"clusterInfo\":\"{\\\"obServerAgents\\\":[{\\\"parentUuid\\\":\\\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\\\",\\\"ip\\\":\\\"8.40.129.26\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\"}],\\\"obClientAgents\\\":[{\\\"parentUuid\\\":\\\"3d6544b6-23e9-44bc-bcab-de72a3d21682\\\",\\\"nodeType\\\":\\\"OBClient\\\"}]}\"},\"name\":\"集群名称adfasgdsa\",\"port\":0,\"scanInterval\":3600,\"sourceType\":\"register\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\"}";
        ProtectedEnvironment environment = JSON.parseObject(param, ProtectedEnvironment.class);
        OBClusterInfo clusterInfo = OceanBaseUtils.readExtendClusterInfo(environment);
        String obClientUuid = clusterInfo.getObClientAgents().get(0).getParentUuid();
        String obServerUuid = clusterInfo.getObServerAgents().get(0).getParentUuid();
        mockAgent(obClientUuid, obServerUuid);

        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> oceanBaseClusterProvider.register(environment));

        Assert.assertEquals("Auth param is empty", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, legoCheckedException.getErrorCode());
    }

    private OBClusterInfo generateVersionResult() {
        OBClusterInfo appEnvResponse = new OBClusterInfo();
        appEnvResponse.setVersion("3.2.3");
        appEnvResponse.setTenantInfos(Lists.newArrayList(new OBTenantInfo("tenant1"), new OBTenantInfo("tenant3")));
        return appEnvResponse;
    }

    /**
     * 用例场景：健康检查成功
     * 前置条件：OB服务运行正常
     * 检查点：返回正常
     */
    @Test
    public void check_health_success() {
        String param
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"123456\",\"authType\":2,\"extendInfo\":{}},\"cluster\":false,\"createdTime\":\"2023-07-14 12:09:12.246\",\"endpoint\":\"192.168.129.12,192.168.129.26\",\"extendInfo\":{\"clusterInfo\":\"{\\\"obServerAgents\\\":[{\\\"parentUuid\\\":\\\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\\\",\\\"ip\\\":\\\"8.40.129.26\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\",\\\"linkStatus\\\":\\\"1\\\"}],\\\"obClientAgents\\\":[{\\\"parentUuid\\\":\\\"3d6544b6-23e9-44bc-bcab-de72a3d21682\\\",\\\"nodeType\\\":\\\"OBClient\\\",\\\"linkStatus\\\":\\\"1\\\"}]}\"},\"linkStatus\":\"1\",\"name\":\"cluster_name\",\"port\":0,\"protectionStatus\":0,\"rootUuid\":\"50f3bbcb-dff5-31d8-857c-2879f321a132\",\"scanInterval\":3600,\"sourceType\":\"register\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\",\"uuid\":\"50f3bbcb-dff5-31d8-857c-2879f321a132\",\"version\":\"3.2.4\"}";
        ProtectedEnvironment environment = JSON.parseObject(param, ProtectedEnvironment.class);
        ActionResult actionResult1 = new ActionResult(ActionResult.SUCCESS_CODE, "");
        ActionResult actionResult2 = new ActionResult(ActionResult.SUCCESS_CODE, "");

        List<ActionResult> actionResultList = Lists.newArrayList(actionResult1, actionResult2);
        ResourceCheckContext context = new ResourceCheckContext();
        context.setActionResults(actionResultList);

        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(unifiedConnectionCheckProvider);
        PowerMockito.when(unifiedConnectionCheckProvider.tryCheckConnection(any())).thenReturn(context);

        ProtectedResource child = new ProtectedResource();
        child.setUuid("11111");
        child.setExtendInfoByKey(OBConstants.KEY_CLUSTER_INFO,
            "{\\\"tenantInfos\\\":[{\\\"name\\\":\\\"tenant1\\\"},{\\\"name\\\":\\\"tenant3\\\"}]}");

        List<ProtectedResource> children = Lists.newArrayList(child);
        PowerMockito.when(oceanBaseService.getProtectedEnvironments(any(), any())).thenReturn(children);

        String result = oceanBaseClusterProvider.healthCheckWithResultStatus(environment).get();

        InOrder inOrder = inOrder(oceanBaseService, unifiedConnectionCheckProvider);
        inOrder.verify(unifiedConnectionCheckProvider).tryCheckConnection(any());
        inOrder.verify(oceanBaseService).getProtectedEnvironments(any(), any());
        inOrder.verify(oceanBaseService).checkTenantSetConnect(child);
        inOrder.verify(oceanBaseService).updateExtendInfo(children);
        Assert.assertEquals(result, LinkStatusEnum.ONLINE.getStatus().toString());
    }

    /**
     * 用例场景：健康检查时， 连通性检查失败了
     * 前置条件：OBserver节点或ObClient节点进程异常
     * 检查点：返回异常
     */
    @Test
    public void check_health_connect_failed() {
        String param
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"123456\",\"authType\":2,\"extendInfo\":{}},\"cluster\":false,\"createdTime\":\"2023-07-14 12:09:12.246\",\"endpoint\":\"192.168.129.12,192.168.129.26\",\"extendInfo\":{\"clusterInfo\":\"{\\\"obServerAgents\\\":[{\\\"parentUuid\\\":\\\"8796bfa6-e9ad-41e3-91f2-93af637ebf98\\\",\\\"ip\\\":\\\"8.40.129.26\\\",\\\"port\\\":\\\"2881\\\",\\\"nodeType\\\":\\\"OBServer\\\",\\\"linkStatus\\\":\\\"1\\\"}],\\\"obClientAgents\\\":[{\\\"parentUuid\\\":\\\"3d6544b6-23e9-44bc-bcab-de72a3d21682\\\",\\\"nodeType\\\":\\\"OBClient\\\",\\\"linkStatus\\\":\\\"1\\\"}]}\"},\"linkStatus\":\"1\",\"name\":\"cluster_name\",\"port\":0,\"protectionStatus\":0,\"rootUuid\":\"50f3bbcb-dff5-31d8-857c-2879f321a132\",\"scanInterval\":3600,\"sourceType\":\"register\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\",\"uuid\":\"50f3bbcb-dff5-31d8-857c-2879f321a132\",\"version\":\"3.2.4\"}";
        ProtectedEnvironment environment = JSON.parseObject(param, ProtectedEnvironment.class);
        ActionResult actionResult1 = new ActionResult(ActionResult.SUCCESS_CODE, "");
        ActionResult actionResult2 = new ActionResult(OBErrorCodeConstants.AUTH_ERROR, "");

        List<ActionResult> actionResultList = Lists.newArrayList(actionResult1, actionResult2);
        ResourceCheckContext context = new ResourceCheckContext();
        context.setActionResults(actionResultList);

        PowerMockito.when(unifiedConnectionCheckProvider.tryCheckConnection(any())).thenReturn(context);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(unifiedConnectionCheckProvider);

        String result = oceanBaseClusterProvider.healthCheckWithResultStatus(environment).get();

        verify(unifiedConnectionCheckProvider).tryCheckConnection(any());
        verify(oceanBaseService, never()).getProtectedEnvironments(any(), any());
        verify(oceanBaseService, never()).checkTenantSetConnect(any());
        verify(oceanBaseService, never()).updateExtendInfo(any());
        Assert.assertEquals(result, LinkStatusEnum.OFFLINE.getStatus().toString());
    }
}
