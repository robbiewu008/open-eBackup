package openbackup.mysql.resources.access.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.access.framework.resource.service.provider.UnifiedConnectionCheckProvider;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.provider.DbClusterProvider;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.enums.MysqlResourceSubTypeEnum;
import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import openbackup.system.base.sdk.host.HostRestApi;
import openbackup.system.base.sdk.host.model.Host;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.ImmutableMap;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * MySQL集群实例测试类
 *
 * @author xWX950025
 * @since 2022-06-01
 */
@RunWith(PowerMockRunner.class)
public class MysqlClusterInstanceProviderTest {
    private MysqlClusterInstanceProvider mysqlClusterInstanceProvider;

    private ProviderManager providerManager;

    private DbClusterProvider dbClusterProvider;

    private MysqlBaseService mysqlBaseService;

    @Mock
    private HostRestApi hostRestApi;

    @Mock
    private AgentUnifiedService agentUnifiedService;

    @Mock
    private UnifiedConnectionCheckProvider unifiedConnectionCheckProvider;

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Before
    public void init() {
        this.dbClusterProvider = Mockito.mock(PXClusterProvider.class);
        this.providerManager = Mockito.mock(ProviderManager.class);
        PluginConfigManager pluginConfigManager = Mockito.mock(PluginConfigManager.class);
        EncryptorService encryptorService = Mockito.mock(EncryptorService.class);
        this.mysqlBaseService = Mockito.mock(MysqlBaseService.class);
        this.mysqlClusterInstanceProvider = new MysqlClusterInstanceProvider(this.providerManager, pluginConfigManager,
                encryptorService, this.mysqlBaseService);
        mysqlClusterInstanceProvider.setHostRestApi(hostRestApi);
        mysqlClusterInstanceProvider.setAgentUnifiedService(agentUnifiedService);

    }

    /**
     * 用例场景：mysql集群实例检查类provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.getType());
        Assert.assertTrue(mysqlClusterInstanceProvider.applicable(protectedResource));
    }

    /**
     * 用例场景：mysql集群实例创建
     * 前置条件：联通性检查成功、且未创建过该实例
     * 检查点：mysql实例创建成功
     */
    @Test
    public void beforeCreate_success() {
        ProtectedResource protectedResource = getProtectedResource(false);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(dbClusterProvider);
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, protectedResource))
            .thenReturn(unifiedConnectionCheckProvider);
        PowerMockito.when(unifiedConnectionCheckProvider.checkConnection(protectedResource))
            .thenReturn(new ResourceCheckContext());
        PowerMockito.when(mysqlBaseService.getEnvironmentById(any())).thenReturn(getEnvironment());
        PowerMockito.when(dbClusterProvider.checkIsCluster(any())).thenReturn(true);
        mysqlClusterInstanceProvider.beforeCreate(protectedResource);
        Assert.assertEquals(MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.getType(), protectedResource.getSubType());
    }

    /**
     * 用例场景：mysql集群实例创建
     * 前置条件：联通性检查成功、集群条件检查失败
     * 检查点：mysql实例创建失败
     */
    @Test
    public void should_throw_DataProtectionAccessException_when_beforeCreate() {
        expectedException.expect(DataProtectionAccessException.class);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.getType());
        List<ProtectedResource> protectedResources = Collections.singletonList(protectedResource);
        HashMap<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.CHILDREN, protectedResources);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.CLUSTER_TYPE, "AP");
        protectedResource.setDependencies(dependencies);
        protectedResource.setExtendInfo(extendInfo);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setEndpoint("10.44.218.91");
        protectedEnvironment.setPort(3306);
        protectedResource.setEnvironment(protectedEnvironment);
        PowerMockito.when(providerManager.findProvider(any(), any()))
            .thenReturn(dbClusterProvider);
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, protectedResource))
            .thenReturn(unifiedConnectionCheckProvider);
        PowerMockito.when(unifiedConnectionCheckProvider.checkConnection(protectedResource))
            .thenReturn(new ResourceCheckContext());
        PowerMockito.when(hostRestApi.queryHostByID(any())).thenReturn(getHostInfo());
        PowerMockito.when(agentUnifiedService.getClusterInfo(any(),any()))
                .thenReturn(getAppEnvResponse(true));
        PowerMockito.doThrow(
            new DataProtectionAccessException(CommonErrorCode.TARGET_CLUSTER_AUTH_FAILED, new String[] {"failed"}))
            .when(dbClusterProvider)
            .checkIsCluster(any());
        mysqlClusterInstanceProvider.beforeCreate(protectedResource);
    }

    /**
     * 用例场景：mysql集群实例修改
     * 前置条件：已成功注册集群实例
     * 检查点：mysql实例修改成功
     */
    @Test
    public void beforeUpdate_success() {
        ProtectedResource protectedResource = getProtectedResource(false);
        ProtectedResource mysqlSingleResource = getMysqlSingleResource();
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(dbClusterProvider);
        List<ProtectedResource> resources = protectedResource.getDependencies().get(DatabaseConstants.AGENTS);
        PowerMockito.when(providerManager.findProvider(ResourceConnectionCheckProvider.class, resources.get(0)))
            .thenReturn(unifiedConnectionCheckProvider);
        PowerMockito.when(unifiedConnectionCheckProvider.checkConnection(protectedResource))
            .thenReturn(new ResourceCheckContext());
        PowerMockito.when(dbClusterProvider.checkIsCluster(any())).thenReturn(true);
        PowerMockito.when(mysqlBaseService.getResource(any())).thenReturn(mysqlSingleResource);
        PowerMockito.when(mysqlBaseService.getEnvironmentById(any())).thenReturn(getEnvironment());
        mysqlClusterInstanceProvider.beforeUpdate(protectedResource);
        Assert.assertEquals(MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.getType(), protectedResource.getSubType());
    }

    /**
     * 用例场景：mysql集群实例修改
     * 前置条件：注册实例
     * 检查点：注册实例时第一次修改成功
     */
    @Test
    public void when_clusterInstance_creating_beforeUpdate_success() {
        ProtectedResource protectedResource = getProtectedResource(true);
        mysqlClusterInstanceProvider.beforeUpdate(protectedResource);
        Assert.assertEquals(MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.getType(), protectedResource.getSubType());
    }

    /**
     * 用例场景：mysql集群实例修改
     * 前置条件：注册成功实例
     * 检查点：mysql实例修改失败
     */
    @Test
    public void should_throw_LegoCheckedException_when_beforeUpdate() {
        expectedException.expect(LegoCheckedException.class);
        ProtectedResource protectedResource = getProtectedResource(false);
        ProtectedResource mysqlSingleResource = getMysqlSingleResource();
        Map<String, List<ProtectedResource>> dependencies = protectedResource.getDependencies();
        dependencies.remove(DatabaseConstants.CHILDREN);
        PowerMockito.when(mysqlBaseService.getResource(any())).thenReturn(mysqlSingleResource);
        mysqlClusterInstanceProvider.beforeUpdate(protectedResource);
    }

    /**
     * 用例场景：设置是否更新dependence中host配置为false
     * 前置条件：无
     * 检查点: 设置成功
     */
    @Test
    public void getResourceFeature_success() {
        Assert.assertFalse(mysqlClusterInstanceProvider.getResourceFeature().isShouldUpdateDependencyHostInfoWhenScan());
    }

    @Test
    public void test_check_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setExtendInfo(ImmutableMap.of(DatabaseConstants.CLUSTER_TYPE, MysqlConstants.EAPP));
        PowerMockito.when(dbClusterProvider.checkIsCluster(resource)).thenReturn(true);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(dbClusterProvider);
        mysqlClusterInstanceProvider.check(resource);
        Mockito.verify(dbClusterProvider, Mockito.times(1)).checkIsCluster(resource);
    }

    @Test
    public void test_check_fail_when_no_provider() {
        expectedException.expectMessage("Has no provider");
        ProtectedResource resource = new ProtectedResource();
        resource.setExtendInfo(ImmutableMap.of(DatabaseConstants.CLUSTER_TYPE, MysqlConstants.EAPP));
        mysqlClusterInstanceProvider.check(resource);
    }

    @Test
    public void test_updateCheck_when_error_cluster_type() {
        ProtectedResource resource = new ProtectedResource();
        resource.setExtendInfo(ImmutableMap.of(DatabaseConstants.CLUSTER_TYPE, MysqlConstants.AP));
        mysqlClusterInstanceProvider.updateCheck(resource);
        Mockito.verify(providerManager, Mockito.times(0)).findProvider(any(), any());
    }

    /**
     * 构建饭回资源
     * @param first 是否是第一次更新
     * @return  ProtectedResource 返回构建资源
     */
    private ProtectedResource getProtectedResource(boolean first) {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.getType());
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.CLUSTER_TYPE, "PXC");
        protectedResource.setExtendInfo(extendInfo);
        if (first){
            return protectedResource;
        }
        List<ProtectedResource> responseList = new ArrayList<>();
        ProtectedResource dependencyResource = new ProtectedResource();
        Authentication authentication = new Authentication();
        authentication.setAuthKey("root");
        authentication.setAuthPwd("123456");
        extendInfo.put(DatabaseConstants.INSTANCE_PORT, "3306");
        extendInfo.put(MysqlConstants.DEPLOY_OPERATING_SYSTEM, "centos");
        extendInfo.put(DatabaseConstants.ROLE, "1");

        authentication.setExtendInfo(extendInfo);
        dependencyResource.setUuid("111111111");
        dependencyResource.setExtendInfo(extendInfo);
        dependencyResource.setVersion("5.5.5-10.2.43-MariaDB-log");
        dependencyResource.setAuth(authentication);
        responseList.add(dependencyResource);

        Map<String, List<ProtectedResource>> dependency = new HashMap<>();
        dependency.put(DatabaseConstants.AGENTS, responseList);
        dependency.put(DatabaseConstants.CHILDREN, responseList);
        dependencyResource.setDependencies(dependency);
        protectedResource.setDependencies(dependency);
        return protectedResource;
    }

    /**
     * 构建饭回资源
     *
     * @return  ProtectedResource 返回构建资源单实例
     */
    private ProtectedResource getMysqlSingleResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType());
        Authentication auth = new Authentication();
        auth.setAuthKey("root");
        auth.setAuthPwd("123456");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.INSTANCE_PORT, "3306");
        auth.setExtendInfo(extendInfo);
        resource.setAuth(auth);
        HashMap<String, String> hashMap = new HashMap<>();
        resource.setExtendInfo(hashMap);
        return resource;
    }

    /**
     * 构建饭回资源
     *
     * @return ProtectedResource 返回构建资源单实例
     */
    private ProtectedEnvironment getEnvironment() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid("111111");
        environment.setEndpoint("192.168.1.1");
        return environment;
    }


    /**
     * 构建返回的主机
     *
     * @return  Host 返回构建主机信息
     */
    private Host getHostInfo() {
        Host host = new Host();
        host.setIp("127.0.0.1");
        host.setPort("3306");
        return host;
    }

    /**
     * 构建返回的主机
     *
     * @return  Host 返回构建主机信息
     */
    private AppEnvResponse getAppEnvResponse(Boolean isMaster) {
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setExtendInfo(new HashMap<>());
        appEnvResponse.getExtendInfo().put(DatabaseConstants.ROLE, "2");
        if (isMaster){
            appEnvResponse.getExtendInfo().put(DatabaseConstants.ROLE, "1");
        }
        appEnvResponse.getExtendInfo().put("status", "0");
        appEnvResponse.getExtendInfo().put(DatabaseConstants.DATA_DIR, "/data");
        appEnvResponse.getExtendInfo().put(DatabaseConstants.VERSION, "5.5");
        appEnvResponse.getExtendInfo().put(MysqlConstants.DEPLOY_OPERATING_SYSTEM, "red hat");
        appEnvResponse.getExtendInfo().put(MysqlConstants.MYSQL_SERVICE_NAME, "mysql.service");
        appEnvResponse.getExtendInfo().put(MysqlConstants.MYSQL_SYSTEM_SERVICE_TYPE, "systemctl");
        return appEnvResponse;
    }
}
