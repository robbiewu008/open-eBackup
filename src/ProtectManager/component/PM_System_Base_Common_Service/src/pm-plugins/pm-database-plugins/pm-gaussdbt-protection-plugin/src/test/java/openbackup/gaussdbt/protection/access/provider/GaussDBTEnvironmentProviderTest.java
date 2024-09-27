package openbackup.gaussdbt.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTClusterStateEnum;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTConstant;
import openbackup.gaussdbt.protection.access.provider.util.GaussDBTClusterUtil;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;

/**
 * 高斯资源健康检查测试类
 *
 * @author hwx1144169
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-31
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(GaussDBTClusterUtil.class)
public class GaussDBTEnvironmentProviderTest {
    private static final String TEST_UUID = "cfd1799e-6f09-3330-ace5-7b79938017c";
    private static final String TEST_NAME = "TestClusterName";
    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);
    private final ProviderManager providerManager = PowerMockito.mock(ProviderManager.class);
    private final PluginConfigManager pluginConfigManager = PowerMockito.mock(PluginConfigManager.class);
    private final ResourceConnectionCheckProvider checkProvider = PowerMockito.mock(ResourceConnectionCheckProvider.class);
    private final GaussDBTEnvironmentProvider gaussDBTEnvironmentProvider = new GaussDBTEnvironmentProvider(
        providerManager, pluginConfigManager, resourceService);

    @Before
    public void setUp() {
        PowerMockito.mockStatic(GaussDBTClusterUtil.class);
    }

    /**
     * 用例场景：gaussDBT类型识别
     * 前置条件：类型参数为GaussDBT
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(gaussDBTEnvironmentProvider.applicable(ResourceSubTypeEnum.GAUSSDBT.getType()));
        Assert.assertFalse(gaussDBTEnvironmentProvider.applicable(ResourceSubTypeEnum.DAMENG.getType()));
    }

    /**
     * 用例场景：检查是否根据相同的字符串生成的UUID唯一
     * 前置条件：两个相同的字符串
     * 检查点: 生成的UUID相同
     */
    @Test
    public void general_unique_uuid_success() {
        String s1 = ResourceSubTypeEnum.GAUSSDBT.getType() + TEST_UUID;
        String s2 = ResourceSubTypeEnum.GAUSSDBT.getType() + TEST_UUID;
        String uuid1 = UUID.nameUUIDFromBytes(s1.getBytes()).toString();
        String uuid2 = UUID.nameUUIDFromBytes(s2.getBytes()).toString();
        Assert.assertEquals(uuid1, uuid2);
    }

    /**
     * 用例场景：当注册的环境生成的唯一UUID已存在资源时候，说明重复注册
     * 前置条件：数据库已存在该UUID创建的资源
     * 检查点: 抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_check_duplicated_environment() {
        ProtectedResource resource = new ProtectedResource();
        Optional<ProtectedResource> resourceOptional = Optional.of(resource);
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(resourceOptional);
        Assert.assertThrows(LegoCheckedException.class,
            () -> Whitebox.invokeMethod(gaussDBTEnvironmentProvider, "checkDuplicatedEnvironment", anyString()));
    }

    /**
     * 用例场景：当agent主机已经创建过GaussDBT集群资源时，不能再创建资源
     * 前置条件：该agent主机创建了某个集群资源
     * 检查点: 抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_check_host_has_already_registered() {
        Set<String> uuids = new HashSet<>();
        uuids.add(TEST_UUID);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        String nodeString = "[{\"uuid\":\"cfd1799e-6f09-3330-ace5-7b79938017c\"}]";
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(GaussDBTConstant.NODES_KEY, nodeString);
        environment.setExtendInfo(extendInfo);
        List<ProtectedEnvironment> environments = Collections.singletonList(environment);
        Assert.assertThrows(LegoCheckedException.class,
            () -> Whitebox.invokeMethod(gaussDBTEnvironmentProvider, "checkHostRegistered", uuids, environments));
    }

    /**
     * 用例场景：创建资源的部署类型和检测到的类型进行比对
     * 前置条件：类型不一致
     * 检查点: 抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_deploy_type_is_not_match_real_deploy_type() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.DEPLOY_TYPE, "2");
        environment.setExtendInfo(extendInfo);
        Map<String, String> appExtendInfo = new HashMap<>();
        appExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, "1");
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setExtendInfo(appExtendInfo);
        Set<String> uuids = new HashSet<>();
        Assert.assertThrows(LegoCheckedException.class,
            () -> Whitebox.invokeMethod(gaussDBTEnvironmentProvider, "checkClusterInfo", environment, appEnvResponse, uuids));
    }

    /**
     * 用例场景：当连接agent的check方法失败时，抛出异常
     * 前置条件：连接agent的check方法失败时
     * 检查点: 抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_check_connection_failed() {
        ProtectedEnvironment environment = getProtectedEnvironment();
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(checkProvider);
        PowerMockito.doThrow(new LegoCheckedException("")).when(checkProvider).checkConnection(any());
        Assert.assertThrows(LegoCheckedException.class,
            () -> Whitebox.invokeMethod(gaussDBTEnvironmentProvider, "checkClusterConnection", environment));
    }

    /**
     * 用例场景：当连接agent的check方法失败时，抛出异常
     * 前置条件：连接agent的check方法失败时
     * 检查点: 抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_agents_is_not_a_same_cluster() {
        ResourceCheckContext checkContext = new ResourceCheckContext();
        Map<String, Object> map = new HashMap<>();
        String app = "[{\"uuid\":\"test_uuid\", \"nodes\": [{\"uuid\":\"123456\", \"endpoint\":\"127.0.0.1\"}, "
            + "{\"uuid\":\"123457\", \"endpoint\":\"127.0.0.2\"}], \"extendInfo\": {\"deployType\": \"1\"}}, "
            + "{\"uuid\":\"test_uuid\", \"nodes\": [{\"uuid\":\"123458\", \"endpoint\":\"127.0.0.3\"}], "
            + "\"extendInfo\": {\"deployType\": \"1\"}}]";
        map.put(GaussDBTConstant.CLUSTER_INFO_KEY, app);
        checkContext.setContext(map);
        Assert.assertThrows(LegoCheckedException.class, () -> Whitebox.invokeMethod(gaussDBTEnvironmentProvider, "getCheckResult", checkContext));
    }

    /**
     * 用例场景：注册资源前置检查成功
     * 前置条件：资源注册信息正确，生产环境功能正常
     * 检查点: 前置检查成功
     */
    @Test
    public void pre_check_success_when_register_a_GaussDBT_environment() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        String nodes = "[{\"uuid\":\"123456\", \"endpoint\":\"127.0.0.1\"}]";
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(GaussDBTConstant.NODES_KEY, nodes);
        environment.setExtendInfo(extendInfo);
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(Collections.singletonList(environment));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(response);
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.empty());
        ResourceCheckContext checkContext = new ResourceCheckContext();
        Map<String, Object> map = new HashMap<>();
        String app = "[{\"uuid\":\"test_uuid\", \"nodes\": [{\"uuid\":\"123456\", \"endpoint\":\"127.0.0.1\"}], \"extendInfo\": {\"deployType\": \"1\"}}]";
        map.put(GaussDBTConstant.CLUSTER_INFO_KEY, app);
        checkContext.setContext(map);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(checkProvider);
        PowerMockito.when(checkProvider.checkConnection(any())).thenReturn(checkContext);
        gaussDBTEnvironmentProvider.register(getProtectedEnvironment());
    }

    /**
     * 用例场景：更新的资源不存在时候抛出异常
     * 前置条件：更新的资源不存在
     * 检查点: 抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_update_a_GaussDBT_environment_not_exist() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid(TEST_UUID);
        environment.setName(TEST_NAME);
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.empty());
        Assert.assertThrows(LegoCheckedException.class, () -> gaussDBTEnvironmentProvider.register(environment));
    }

    /**
     * 用例场景：更新资源前置检查成功
     * 前置条件：资源更新信息正确，生产环境功能正常
     * 检查点: 前置检查成功
     */
    @Test
    public void pre_check_success_when_update_a_GaussDBT_environment() {
        ProtectedEnvironment newEnvironment = getProtectedEnvironment();
        newEnvironment.setUuid("test_uuid");
        String nodes = "[{\"uuid\":\"123456\", \"endpoint\":\"127.0.0.1\"}]";
        Map<String, String> info = new HashMap<>();
        info.put(GaussDBTConstant.NODES_KEY, nodes);
        info.put(DatabaseConstants.DEPLOY_TYPE, "1");
        newEnvironment.setExtendInfo(info);
        PowerMockito.when(resourceService.getResourceById("test_uuid")).thenReturn(Optional.of(newEnvironment));
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid(TEST_UUID);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(GaussDBTConstant.NODES_KEY, nodes);
        environment.setExtendInfo(extendInfo);
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(Collections.singletonList(environment));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(response);
        ResourceCheckContext checkContext = new ResourceCheckContext();
        Map<String, Object> map = new HashMap<>();
        String app = "[{\"uuid\":\"test_uuid\", \"nodes\": [{\"uuid\":\"123456\", \"endpoint\":\"127.0.0.1\"}], \"extendInfo\": {\"deployType\": \"1\"}}]";
        map.put(GaussDBTConstant.CLUSTER_INFO_KEY, app);
        checkContext.setContext(map);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(checkProvider);
        PowerMockito.when(checkProvider.checkConnection(any())).thenReturn(checkContext);
        gaussDBTEnvironmentProvider.register(newEnvironment);
    }

    /**
     * 用例场景：健康检查成功
     * 前置条件：GaussDBT生产环境和agent正常
     * 检查点: 检查成功
     */
    @Test
    public void health_check_success_and_update_environment_to_normal_status() {
        ProtectedEnvironment environment = getProtectedEnvironment();
        ResourceCheckContext checkContext = new ResourceCheckContext();
        Map<String, Object> map = new HashMap<>();
        String app = "[{\"uuid\":\"test_uuid\", \"nodes\": [{\"uuid\":\"123456\", \"endpoint\":\"127.0.0.1\"}], \"extendInfo\": {\"deployType\": \"1\"}}]";
        map.put(GaussDBTConstant.CLUSTER_INFO_KEY, app);
        checkContext.setContext(map);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(checkProvider);
        PowerMockito.when(checkProvider.checkConnection(any())).thenReturn(checkContext);
        gaussDBTEnvironmentProvider.validate(environment);
    }

    /**
     * 用例场景：健康检查成功
     * 前置条件：GaussDBT生产环境或agent连接异常
     * 检查点: 检查成功并更新集群到离线状态
     */
    @Test
    public void health_check_failed_and_update_environment_to_offline() {
        ProtectedEnvironment environment = getProtectedEnvironment();
        environment.setUuid(TEST_UUID);
        String nodeInfo = "[{\"uuid\":\"654321\",\"status\":\"ONLINE\"}]";
        Map<String, String> info = new HashMap<>();
        info.put(GaussDBTConstant.NODES_KEY, nodeInfo);
        environment.setExtendInfo(info);
        ResourceCheckContext checkContext = new ResourceCheckContext();
        Map<String, Object> map = new HashMap<>();
        map.put(GaussDBTConstant.CLUSTER_INFO_KEY, new Object());
        checkContext.setContext(map);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(checkProvider);
        PowerMockito.when(checkProvider.checkConnection(any())).thenReturn(checkContext);
        gaussDBTEnvironmentProvider.validate(environment);
        Assert.assertEquals(GaussDBTClusterStateEnum.UNAVAILABLE.getState(), environment.getExtendInfoByKey(GaussDBTConstant.CLUSTER_STATE_KEY));
    }

    private ProtectedEnvironment getProtectedEnvironment() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setName("gaussDB");
        Authentication authentication = new Authentication();
        authentication.setAuthType(1);
        authentication.setAuthKey("omm");
        protectedEnvironment.setAuth(authentication);
        protectedEnvironment.setType(ResourceTypeEnum.DATABASE.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.GAUSSDBT.getType());
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("deployType", "1");
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid(TEST_UUID);
        protectedEnvironment.setDependencies(Collections.singletonMap("agents", Collections.singletonList(resource)));
        protectedEnvironment.setExtendInfo(extendInfo);
        return protectedEnvironment;
    }
}
