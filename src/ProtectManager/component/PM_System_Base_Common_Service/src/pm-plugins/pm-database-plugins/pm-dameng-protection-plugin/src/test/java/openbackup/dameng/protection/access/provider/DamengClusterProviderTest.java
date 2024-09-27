package openbackup.dameng.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.dameng.protection.access.DamengTestDataUtil;
import openbackup.dameng.protection.access.constant.DamengConstant;
import openbackup.dameng.protection.access.service.DamengService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * dameng集群注册测试类
 *
 * @author lWX1100347
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-25
 */
@RunWith(PowerMockRunner.class)
public class DamengClusterProviderTest {
    private final ProviderManager providerManager = PowerMockito.mock(ProviderManager.class);

    private final PluginConfigManager pluginConfigManager = PowerMockito.mock(PluginConfigManager.class);

    private final DamengService damengService = PowerMockito.mock(DamengService.class);

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final ResourceConnectionCheckProvider resourceConnectionCheckProvider = PowerMockito.mock(
        ResourceConnectionCheckProvider.class);

    private final ProtectedResourceChecker protectedResourceChecker = PowerMockito.mock(ProtectedResourceChecker.class);

    private final DamengClusterProvider damengClusterProvider = new DamengClusterProvider(providerManager,
        pluginConfigManager, damengService, resourceService, protectedResourceChecker);

    /**
     * 用例场景：dameng环境检查类过滤
     * 前置条件：无
     * 检查点：过滤成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(damengClusterProvider.applicable(ResourceSubTypeEnum.DAMENG_CLUSTER.getType()));
    }

    /**
     * 用例场景：dameng集群创建
     * 前置条件：联通性检查成功、且未创建过
     * 检查点：dameng集群创建成功
     */
    @Test
    public void register_check_success() {
        create_mockito_condition();
        ProtectedEnvironment environment = DamengTestDataUtil.buildProtectedEnvironment("",
            ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setUuid("uuid");
        PowerMockito.when(damengService.queryAgentEnvironment(any())).thenReturn(agent);
        damengClusterProvider.register(environment);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), environment.getLinkStatus());
    }

    /**
     * 用例场景：dameng集群修改
     * 前置条件：联通性检查成功、且已经注册
     * 检查点：dameng集群修改成功
     */
    @Test
    public void update_check_success() {
        create_mockito_condition();
        ProtectedEnvironment environment = DamengTestDataUtil.buildProtectedEnvironment(UUIDGenerator.getUUID(),
            ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("uuid");
        environment.getDependencies().put(DatabaseConstants.DELETE_CHILDREN, Collections.singletonList(resource));
        PowerMockito.when(damengService.getEnvironmentById(anyString())).thenReturn(environment);
        PowerMockito.when(damengService.queryAgentEnvironment(any())).thenReturn(environment);
        damengClusterProvider.register(environment);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), environment.getLinkStatus());
    }

    /**
     * 用例场景：dameng集群修改
     * 前置条件：联通性检查成功、且已经注册，删除的节点不属于本集群的节点
     * 检查点：dameng集群修改成功
     */
    @Test
    public void should_throw_LegoCheckedException_when_update_check_delete_children_is_not_instance() {
        create_mockito_condition();
        ProtectedEnvironment environment = DamengTestDataUtil.buildProtectedEnvironment(UUIDGenerator.getUUID(),
            ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("not_instance_uuid");
        environment.getDependencies().put(DatabaseConstants.DELETE_CHILDREN, Collections.singletonList(resource));
        PowerMockito.when(damengService.getEnvironmentById(anyString())).thenReturn(environment);
        PowerMockito.when(damengService.queryAgentEnvironment(any())).thenReturn(environment);
        Assert.assertThrows(LegoCheckedException.class, () -> damengClusterProvider.register(environment));
    }

    /**
     * 用例场景：健康检查
     * 前置条件：无
     * 检查点: 检查成功
     */
    @Test
    public void health_check_success() {
        create_mockito_condition();
        ProtectedEnvironment environment = DamengTestDataUtil.buildProtectedEnvironment(UUIDGenerator.getUUID(),
            ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        PowerMockito.when(damengService.queryAgentEnvironment(any())).thenReturn(environment);
        PowerMockito.when(providerManager.findProviderOrDefault(ProtectedResourceChecker.class, environment,
            this.protectedResourceChecker)).thenReturn(protectedResourceChecker);
        PowerMockito.when(protectedResourceChecker.collectConnectableResources(environment))
            .thenReturn(getProtectedResourceListMap());
        CheckResult checkResult = new CheckResult();
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(0L);
        checkResult.setResults(actionResult);
        PowerMockito.when(protectedResourceChecker.generateCheckResult(any())).thenReturn(checkResult);
        PowerMockito.when(damengService.getNodeInfoFromNodes(any())).thenReturn(DamengTestDataUtil.buildNodeInfo());
        damengClusterProvider.healthCheckWithResultStatus(environment);
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), environment.getLinkStatus());
    }

    /**
     * 用例场景：注册修改集群时，状态离线抛异常
     * 前置条件：实例离线
     * 检查点: 报错
     */
    @Test
    public void should_throw_LegoCheckedException_if_linkStatus_is_offline() {
        AppEnvResponse response = new AppEnvResponse();
        NodeInfo nodeInfo = DamengTestDataUtil.buildNodeInfo().get(0);
        nodeInfo.getExtendInfo().put(DamengConstant.INSTANCESTATUS, "0");
        response.setNodes(Collections.singletonList(nodeInfo));
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DamengConstant.VERSION, "V8");
        response.setExtendInfo(extendInfo);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        PowerMockito.when(resourceConnectionCheckProvider.checkConnection(any()))
            .thenReturn(new ResourceCheckContext());
        PowerMockito.when(damengService.check(any())).thenReturn(Collections.singletonList(response));
        ProtectedEnvironment environment = DamengTestDataUtil.buildProtectedEnvironment("uuid",
            ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        PowerMockito.when(damengService.queryAgentEnvironment(any())).thenReturn(environment);
        Assert.assertThrows(LegoCheckedException.class, () -> damengClusterProvider.register(environment));
    }

    @Test
    public void test_scan(){
        PowerMockito.when(damengService.queryClusterInfo(any())).thenReturn(buildAppEnvResponse());
        PowerMockito.when(damengService.getNodeInfoFromNodes(any())).thenReturn(DamengTestDataUtil.buildNodeInfo());
        PowerMockito.doNothing().when(resourceService).updateSourceDirectly(any(List.class));
        damengClusterProvider.scan(DamengTestDataUtil.buildProtectedEnvironment("",
            ResourceSubTypeEnum.DAMENG_CLUSTER.getType()));
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：注册集群时，存在已经注册的实例
     * 前置条件：存在已经注册的实例
     * 检查点: 报错
     */
    @Test
    public void should_throw_LegoCheckedException_if_instance_has_registered() {
        ProtectedEnvironment environment = DamengTestDataUtil.buildProtectedEnvironment("",
            ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setUuid("uuid");
        PowerMockito.when(damengService.queryAgentEnvironment(any())).thenReturn(agent);
        Set<String> uuidAndPortSet = new HashSet<>();
        uuidAndPortSet.add("uuid_5236");
        PowerMockito.when(damengService.getExistingUuidAndPort(any())).thenReturn(uuidAndPortSet);
        PowerMockito.when(damengService.connectUuidAndPort(any(), any())).thenReturn("uuid_5236");
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> damengClusterProvider.register(environment));
        Assert.assertEquals(CommonErrorCode.CLUSTER_NODE_IS_REGISTERED, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：注册集群时，下发参数中存在相同实例
     * 前置条件：下发参数中存在相同实例
     * 检查点: 报错
     */
    @Test
    public void should_throw_LegoCheckedException_if_params_has_same_instance() {
        ProtectedEnvironment environment = DamengTestDataUtil.buildProtectedEnvironment("",
            ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        List<ProtectedResource> subProtectedResourceList = new ArrayList<>();
        subProtectedResourceList.addAll(DamengTestDataUtil.getSubProtectedResourceList());
        subProtectedResourceList.addAll(DamengTestDataUtil.getSubProtectedResourceList());
        environment.getDependencies().put(DatabaseConstants.CHILDREN, subProtectedResourceList);
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setUuid("uuid");
        PowerMockito.when(damengService.queryAgentEnvironment(any())).thenReturn(agent);
        PowerMockito.when(damengService.connectUuidAndPort(any(), any())).thenReturn("uuid_5236");
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> damengClusterProvider.register(environment));
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, legoCheckedException.getErrorCode());
    }

    private Map<ProtectedResource, List<ProtectedEnvironment>> getProtectedResourceListMap() {
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap = new HashMap<>();
        ProtectedEnvironment protectedEnvironment = DamengTestDataUtil.buildProtectedEnvironment("",
            ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        protectedResourceMap.put(DamengTestDataUtil.getSubProtectedResourceList().get(0),
            Collections.singletonList(protectedEnvironment));
        return protectedResourceMap;
    }

    private void create_mockito_condition() {
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        PowerMockito.when(resourceConnectionCheckProvider.checkConnection(any()))
            .thenReturn(new ResourceCheckContext());
        PowerMockito.when(damengService.check(any())).thenReturn(buildAppEnvResponse());
        PowerMockito.when(damengService.queryClusterInfo(any())).thenReturn(buildAppEnvResponse());
    }

    private List<AppEnvResponse> buildAppEnvResponse() {
        AppEnvResponse response = new AppEnvResponse();
        response.setNodes(DamengTestDataUtil.buildNodeInfo());
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DamengConstant.VERSION, "V8");
        response.setExtendInfo(extendInfo);
        return Collections.singletonList(response);
    }
}
