/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.saphana.protection.access.provider.resource;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.ClusterEnvironmentService;
import openbackup.saphana.protection.access.constant.SapHanaConstants;
import openbackup.saphana.protection.access.service.SapHanaResourceService;
import openbackup.saphana.protection.access.util.SapHanaUtil;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * {@link SapHanaInstanceProvider Test}
 *
 * @author wWX1013713
 * @version [DataBackup 1.5.0]
 * @since 2023-05-18
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {SapHanaUtil.class, UUIDGenerator.class})
public class SapHanaInstanceProviderTest {
    private static final long ACCESS_DB_ERROR = 1577213476L;

    private final ProviderManager providerManager = PowerMockito.mock(ProviderManager.class);

    private final PluginConfigManager pluginConfigManager = PowerMockito.mock(PluginConfigManager.class);

    private final ResourceConnectionCheckProvider connectionCheckProvider = PowerMockito.mock(
        ResourceConnectionCheckProvider.class);

    private final ClusterEnvironmentService clusterEnvironmentService = PowerMockito.mock(
        ClusterEnvironmentService.class);

    private final SapHanaResourceService hanaResourceService = PowerMockito.mock(SapHanaResourceService.class);

    private final SapHanaInstanceProvider hanaInstanceProvider = new SapHanaInstanceProvider(providerManager,
        pluginConfigManager, connectionCheckProvider, clusterEnvironmentService, hanaResourceService);

    /**
     * 用例场景：框架调applicable接口
     * 前置条件：applicable输入资源子类型
     * 检查点：SAPHANA-instance类型返回true；其他返回false
     */
    @Test
    public void applicable_sap_hana_instance_provider_success() {
        Assert.assertTrue(hanaInstanceProvider.applicable(ResourceSubTypeEnum.SAPHANA_INSTANCE.getType()));
        Assert.assertFalse(hanaInstanceProvider.applicable(ResourceSubTypeEnum.SAPHANA_DATABASE.getType()));
    }

    /**
     * 用例场景：集群实例注册
     * 前置条件：集群实例信息正确，检查连通性正常
     * 检查点: 实例有UUID，且在线
     */
    @Test
    public void should_instance_online_if_create_cluster_instance_success_when_check() throws Exception {
        PowerMockito.mockStatic(SapHanaUtil.class);
        PowerMockito.doNothing()
            .when(SapHanaUtil.class, "checkEnvironmentExtendInfoParam",
                ArgumentMatchers.any(ProtectedEnvironment.class));
        PowerMockito.when(hanaResourceService.queryEnvironments(ArgumentMatchers.anyList()))
            .thenReturn(mockClusterAgents());
        PowerMockito.doNothing().when(clusterEnvironmentService).checkClusterNodeStatus(ArgumentMatchers.anyList());
        PowerMockito.doNothing().when(clusterEnvironmentService).checkClusterNodeOsType(ArgumentMatchers.anyList());
        PowerMockito.doNothing().when(hanaResourceService).checkInstanceNumber();
        PowerMockito.doNothing()
            .when(hanaResourceService)
            .checkInstanceIsRegistered(ArgumentMatchers.any(ProtectedEnvironment.class));
        String fakeInstanceUuid = "5b899e22-d103-4040-bb93-065f6612e85b";
        PowerMockito.mockStatic(UUIDGenerator.class);
        PowerMockito.doReturn(fakeInstanceUuid).when(UUIDGenerator.class, "getUUID");
        PowerMockito.when(connectionCheckProvider.tryCheckConnection(ArgumentMatchers.any(ProtectedEnvironment.class)))
            .thenReturn(mockClusterCheckContext());
        ProtectedEnvironment createInstance = mockCreateClusterInstanceInfo();
        hanaInstanceProvider.register(createInstance);
        Assert.assertEquals(fakeInstanceUuid, createInstance.getUuid());
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), createInstance.getLinkStatus());
    }

    private ProtectedEnvironment mockCreateClusterInstanceInfo() {
        ProtectedEnvironment instance = new ProtectedEnvironment();
        instance.setSourceType(ResourceTypeEnum.DATABASE.getType());
        instance.setSubType(ResourceSubTypeEnum.SAPHANA_INSTANCE.getType());
        instance.setName("cluster-inst1");
        instance.setExtendInfoByKey(SapHanaConstants.SYSTEM_ID, "A00");
        ProtectedResource firstAgent = new ProtectedResource();
        firstAgent.setUuid("0ed8b119-7d23-475d-9ad3-4fa8e353ed0b");
        ProtectedResource secondAgent = new ProtectedResource();
        secondAgent.setUuid("cfedb495-6574-41e3-843e-e1cb2fc7afd3");
        List<ProtectedResource> agents = Arrays.asList(firstAgent, secondAgent);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.AGENTS, agents);
        instance.setDependencies(dependencies);
        return instance;
    }

    private List<ProtectedEnvironment> mockClusterAgents() {
        List<ProtectedEnvironment> envs = new ArrayList<>();
        ProtectedEnvironment firstAgent = new ProtectedEnvironment();
        firstAgent.setUuid("0ed8b119-7d23-475d-9ad3-4fa8e353ed0b");
        firstAgent.setEndpoint("10.10.10.10");
        firstAgent.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        firstAgent.setOsType("linux");
        envs.add(firstAgent);
        ProtectedEnvironment secondAgent = new ProtectedEnvironment();
        secondAgent.setUuid("cfedb495-6574-41e3-843e-e1cb2fc7afd3");
        secondAgent.setEndpoint("10.10.10.11");
        secondAgent.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        secondAgent.setOsType("linux");
        envs.add(secondAgent);
        return envs;
    }

    private ResourceCheckContext mockClusterCheckContext() {
        ResourceCheckContext checkContext = new ResourceCheckContext();
        ActionResult firResult = new ActionResult();
        firResult.setCode(DatabaseConstants.SUCCESS_CODE);
        String firMsg = "{" + "\"version\":\"2.00.020.00.1500920972\","
            + "\"landscapeId\":\"a7b5dcce-2cad-3b48-80a1-39333544845e\"," + "\"role\": \"2\"" + "}";
        firResult.setMessage(firMsg);
        ActionResult secResult = new ActionResult();
        firResult.setCode(DatabaseConstants.SUCCESS_CODE);
        String secMsg = "{" + "\"version\":\"2.00.020.00.1500920972\","
            + "\"landscapeId\":\"a7b5dcce-2cad-3b48-80a1-39333544845e\"," + "\"role\": \"1\"" + "}";
        secResult.setMessage(secMsg);
        checkContext.setActionResults(Arrays.asList(firResult, secResult));
        return checkContext;
    }

    /**
     * 用例场景：单机实例注册
     * 前置条件：单机实例信息不正确，检查连通性失败
     * 检查点: 抛出LegoCheckedException异常
     */
    @Test
    public void should_throw_ex_if_update_single_instance_fail_when_check() throws Exception {
        PowerMockito.mockStatic(SapHanaUtil.class);
        PowerMockito.doNothing()
            .when(SapHanaUtil.class, "checkEnvironmentExtendInfoParam",
                ArgumentMatchers.any(ProtectedEnvironment.class));
        PowerMockito.when(hanaResourceService.queryEnvironments(ArgumentMatchers.anyList()))
            .thenReturn(mockSingleAgent());
        PowerMockito.doNothing().when(clusterEnvironmentService).checkClusterNodeStatus(ArgumentMatchers.anyList());
        PowerMockito.doNothing().when(clusterEnvironmentService).checkClusterNodeOsType(ArgumentMatchers.anyList());
        PowerMockito.when(connectionCheckProvider.tryCheckConnection(ArgumentMatchers.any(ProtectedEnvironment.class)))
            .thenReturn(mockSingleFailCheckContext());
        ProtectedEnvironment updateInstance = mockUpdateSingleInstanceInfo();
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> hanaInstanceProvider.register(updateInstance));
        Assert.assertEquals(ACCESS_DB_ERROR, legoCheckedException.getErrorCode());
    }

    private ProtectedEnvironment mockUpdateSingleInstanceInfo() {
        ProtectedEnvironment instance = new ProtectedEnvironment();
        instance.setUuid("169b5c98-c39e-4543-9fe2-689699b48a7b ");
        instance.setSourceType(ResourceTypeEnum.DATABASE.getType());
        instance.setSubType(ResourceSubTypeEnum.SAPHANA_INSTANCE.getType());
        instance.setName("single_inst1");
        instance.setExtendInfoByKey(SapHanaConstants.SYSTEM_ID, "f00");
        ProtectedResource firstAgent = new ProtectedResource();
        firstAgent.setUuid("8c02eebe-0c83-454f-8cbf-3b7818691889");
        List<ProtectedResource> agents = Collections.singletonList(firstAgent);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.AGENTS, agents);
        instance.setDependencies(dependencies);
        return instance;
    }

    private List<ProtectedEnvironment> mockSingleAgent() {
        List<ProtectedEnvironment> envs = new ArrayList<>();
        ProtectedEnvironment firstAgent = new ProtectedEnvironment();
        firstAgent.setUuid("8c02eebe-0c83-454f-8cbf-3b7818691889");
        firstAgent.setEndpoint("10.10.10.20");
        firstAgent.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        firstAgent.setOsType("linux");
        envs.add(firstAgent);
        return envs;
    }

    private ResourceCheckContext mockSingleFailCheckContext() {
        ResourceCheckContext checkContext = new ResourceCheckContext();
        ActionResult firResult = new ActionResult();
        firResult.setCode(IsmNumberConstant.TWO_HUNDRED);
        firResult.setBodyErr(String.valueOf(ACCESS_DB_ERROR));
        checkContext.setActionResults(Collections.singletonList(firResult));
        return checkContext;
    }

    /**
     * 用例场景：实例健康检查后，返回状态信息
     * 前置条件：实例所有主机都离线
     * 检查点: 返回离线
     */
    @Test
    public void should_return_offline_if_all_hosts_offline_when_healthCheckWithResultStatus() {
        ProtectedEnvironment agent1 = new ProtectedEnvironment();
        agent1.setLinkStatus("0");
        ProtectedEnvironment agent2 = new ProtectedEnvironment();
        agent2.setLinkStatus("0");
        PowerMockito.doReturn(Arrays.asList(agent1, agent2))
            .when(hanaResourceService)
            .queryEnvironments(ArgumentMatchers.anyList());
        PowerMockito.doNothing()
            .when(hanaResourceService)
            .updateDbLinkStatusOfInstance(ArgumentMatchers.any(ProtectedEnvironment.class), ArgumentMatchers.eq("0"),
                ArgumentMatchers.eq(true), ArgumentMatchers.eq(true));
        String nodes = "[{\"uuid\":\"0ed8b119-7d23-475d-9ad3-4fa8e353ed0b\",\"linkStatus\":\"0\"},"
            + "{\"uuid\":\"cfedb495-6574-41e3-843e-e1cb2fc7afd3\",\"linkStatus\":\"0\"}]";
        ProtectedEnvironment mockInstance = mockClusterInstance(nodes, LinkStatusEnum.ONLINE.getStatus().toString());
        Optional<String> optionalStatus = hanaInstanceProvider.healthCheckWithResultStatus(mockInstance);
        Assert.assertEquals(LinkStatusEnum.OFFLINE.getStatus().toString(), optionalStatus.get());
    }

    /**
     * 用例场景：实例健康检查后，返回状态信息
     * 前置条件：实例初始状态是离线，存在在线的节点，但检查连通性实例状态离线
     * 检查点: 返回离线
     */
    @Test
    public void should_return_offline_if_inst_ori_offline_and_check_conn_offline_when_healthCheckWithResultStatus() {
        ProtectedEnvironment agent1 = new ProtectedEnvironment();
        agent1.setLinkStatus("0");
        ProtectedEnvironment agent2 = new ProtectedEnvironment();
        agent2.setLinkStatus("1");
        PowerMockito.doReturn(Arrays.asList(agent1, agent2))
            .when(hanaResourceService)
            .queryEnvironments(ArgumentMatchers.anyList());
        PowerMockito.doReturn(mockClusterInstanceCheckContext(false, false))
            .when(connectionCheckProvider)
            .tryCheckConnection(ArgumentMatchers.any(ProtectedEnvironment.class));
        PowerMockito.when(
            hanaResourceService.getInstStatusByActionResults(ArgumentMatchers.any(ProtectedEnvironment.class),
                ArgumentMatchers.anyList())).thenReturn("0");
        PowerMockito.doNothing()
            .when(hanaResourceService)
            .updateDbLinkStatusOfInstance(ArgumentMatchers.any(ProtectedEnvironment.class), ArgumentMatchers.eq("0"),
                ArgumentMatchers.eq(true), ArgumentMatchers.eq(true));
        String nodes = "[{\"uuid\":\"0ed8b119-7d23-475d-9ad3-4fa8e353ed0b\",\"linkStatus\":\"0\"},"
            + "{\"uuid\":\"cfedb495-6574-41e3-843e-e1cb2fc7afd3\",\"linkStatus\":\"1\"}]";
        ProtectedEnvironment mockInstance = mockClusterInstance(nodes, LinkStatusEnum.OFFLINE.getStatus().toString());
        Optional<String> optionalStatus = hanaInstanceProvider.healthCheckWithResultStatus(mockInstance);
        Assert.assertEquals(LinkStatusEnum.OFFLINE.getStatus().toString(), optionalStatus.get());
    }

    /**
     * 用例场景：实例健康检查后，返回状态信息
     * 前置条件：实例初始状态是离线，存在在线的节点，检查连通性实例状态在线
     * 检查点: 返回在线
     */
    @Test
    public void should_return_online_if_inst_ori_offline_and_check_conn_online_when_healthCheckWithResultStatus() {
        ProtectedEnvironment agent1 = new ProtectedEnvironment();
        agent1.setLinkStatus("0");
        ProtectedEnvironment agent2 = new ProtectedEnvironment();
        agent2.setLinkStatus("1");
        PowerMockito.doReturn(Arrays.asList(agent1, agent2))
            .when(hanaResourceService)
            .queryEnvironments(ArgumentMatchers.anyList());
        PowerMockito.doReturn(mockClusterInstanceCheckContext(false, true))
            .when(connectionCheckProvider)
            .tryCheckConnection(ArgumentMatchers.any(ProtectedEnvironment.class));
        PowerMockito.when(
            hanaResourceService.getInstStatusByActionResults(ArgumentMatchers.any(ProtectedEnvironment.class),
                ArgumentMatchers.anyList())).thenReturn("1");
        PowerMockito.doNothing()
            .when(hanaResourceService)
            .updateDbLinkStatusOfInstance(ArgumentMatchers.any(ProtectedEnvironment.class), ArgumentMatchers.eq("0"),
                ArgumentMatchers.eq(true), ArgumentMatchers.eq(false));
        PowerMockito.doReturn(new ArrayList<>())
            .when(hanaResourceService)
            .listResourcesByConditions(ArgumentMatchers.anyMap());
        String nodes = "[{\"uuid\":\"0ed8b119-7d23-475d-9ad3-4fa8e353ed0b\",\"linkStatus\":\"0\"},"
            + "{\"uuid\":\"cfedb495-6574-41e3-843e-e1cb2fc7afd3\",\"linkStatus\":\"1\"}]";
        ProtectedEnvironment mockInstance = mockClusterInstance(nodes, LinkStatusEnum.OFFLINE.getStatus().toString());
        Optional<String> optionalStatus = hanaInstanceProvider.healthCheckWithResultStatus(mockInstance);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), optionalStatus.get());
    }

    /**
     * 用例场景：实例健康检查后，返回状态信息
     * 前置条件：实例初始状态是在线，存在在线的节点，检查租户数据库状态成功
     * 检查点: 返回初始在线状态
     */
    @Test
    public void should_return_online_if_inst_ori_online_and_has_online_node_when_healthCheckWithResultStatus() {
        ProtectedEnvironment agent1 = new ProtectedEnvironment();
        agent1.setLinkStatus("0");
        ProtectedEnvironment agent2 = new ProtectedEnvironment();
        agent2.setLinkStatus("1");
        PowerMockito.doReturn(Arrays.asList(agent1, agent2))
            .when(hanaResourceService)
            .queryEnvironments(ArgumentMatchers.anyList());
        PowerMockito.doNothing()
            .when(hanaResourceService)
            .checkAndUpdateTenantDbLinkStatusOfInstance(ArgumentMatchers.any(ProtectedEnvironment.class));
        String nodes = "[{\"uuid\":\"0ed8b119-7d23-475d-9ad3-4fa8e353ed0b\",\"linkStatus\":\"0\"},"
            + "{\"uuid\":\"cfedb495-6574-41e3-843e-e1cb2fc7afd3\",\"linkStatus\":\"1\"}]";
        ProtectedEnvironment mockInstance = mockClusterInstance(nodes, LinkStatusEnum.ONLINE.getStatus().toString());
        Optional<String> optionalStatus = hanaInstanceProvider.healthCheckWithResultStatus(mockInstance);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), optionalStatus.get());
    }

    private ProtectedEnvironment mockClusterInstance(String nodes, String instStatus) {
        ProtectedEnvironment instance = new ProtectedEnvironment();
        instance.setUuid("7fb9d9a8462641c9ac505b24bb6ccd6a");
        instance.setSourceType(ResourceTypeEnum.DATABASE.getType());
        instance.setSubType(ResourceSubTypeEnum.SAPHANA_INSTANCE.getType());
        instance.setName("cluster_inst1");
        instance.setExtendInfoByKey(SapHanaConstants.NODES, nodes);
        instance.setLinkStatus(instStatus);
        return instance;
    }

    private ResourceCheckContext mockClusterInstanceCheckContext(boolean isFirSuccess, boolean isSecSuccess) {
        ResourceCheckContext checkContext = new ResourceCheckContext();
        ActionResult firResult = new ActionResult();
        if (!isFirSuccess) {
            firResult.setCode(IsmNumberConstant.TWO_HUNDRED);
            firResult.setBodyErr(String.valueOf(ACCESS_DB_ERROR));
        }
        ActionResult secResult = new ActionResult();
        if (!isSecSuccess) {
            secResult.setCode(IsmNumberConstant.TWO_HUNDRED);
            secResult.setBodyErr(String.valueOf(ACCESS_DB_ERROR));
        }
        checkContext.setActionResults(Arrays.asList(firResult, secResult));
        return checkContext;
    }
}
