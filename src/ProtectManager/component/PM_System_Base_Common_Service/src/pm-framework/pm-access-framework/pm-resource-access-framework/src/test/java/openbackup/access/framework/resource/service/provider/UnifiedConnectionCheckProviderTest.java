package openbackup.access.framework.resource.service.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;

import openbackup.access.framework.resource.mock.MockResourceChecker;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceExtendInfoMapper;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceMapper;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceRepositoryImpl;
import openbackup.access.framework.resource.provider.DefaultResourceProvider;
import openbackup.access.framework.resource.service.JobScheduleService;
import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.ProtectedEnvironmentServiceImpl;
import openbackup.access.framework.resource.service.ProtectedResourceDecryptService;
import openbackup.access.framework.resource.service.ProtectedResourceDesensitizeService;
import openbackup.access.framework.resource.service.ProtectedResourceEncryptService;
import openbackup.access.framework.resource.service.ProtectedResourceMonitorService;
import openbackup.access.framework.resource.service.ProtectedResourceServiceImpl;
import openbackup.access.framework.resource.service.ProtectedResourceWatchService;
import openbackup.access.framework.resource.service.ResourceAlarmService;
import openbackup.access.framework.resource.service.ResourceScanService;
import openbackup.access.framework.resource.validator.JsonSchemaValidatorImpl;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.plugin.CollectableConfig;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfig;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.plugin.ResourceExtensionManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CyberEngineResourceService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.pack.lock.LockService;
import openbackup.system.base.query.DefaultPageQueryFieldNamingStrategy;
import openbackup.system.base.query.PageQueryService;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.query.SnakeCasePageQueryFieldNamingStrategy;
import com.huawei.oceanprotect.system.base.schedule.service.SchedulerService;
import openbackup.system.base.sdk.alarm.CommonAlarmService;
import openbackup.system.base.sdk.lock.ResourceLockRestApi;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.service.hostagent.AgentQueryService;
import openbackup.system.base.util.MessageTemplate;
import com.huawei.oceanprotect.system.sdk.dto.SystemSwitchDto;
import com.huawei.oceanprotect.system.sdk.enums.SwitchStatusEnum;
import com.huawei.oceanprotect.system.sdk.service.SystemSwitchInternalService;

import com.baomidou.mybatisplus.autoconfigure.MybatisPlusAutoConfiguration;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.mybatis.spring.annotation.MapperScan;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration;
import org.springframework.boot.autoconfigure.sql.init.SqlInitializationAutoConfiguration;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.annotation.ComponentScan;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Optional;
import java.util.UUID;

import javax.annotation.Resource;

/**
 * The UnifiedConnectionCheckProviderTest
 *
 * @author g30003063
 * @since 2022/6/15
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@ComponentScan(basePackages = {"openbackup.access.framework.resource.persistence.dao"})
@MapperScan(basePackages = {"openbackup.access.framework.resource.persistence.dao",
        "openbackup.data.access.framework.core.dao"})
@SpringBootTest(classes = {
    DataSourceAutoConfiguration.class, MybatisPlusAutoConfiguration.class, SqlInitializationAutoConfiguration.class, ProtectedResourceServiceImpl.class,
    ProtectedResourceRepositoryImpl.class, JsonSchemaValidatorImpl.class, PageQueryService.class,
    DefaultPageQueryFieldNamingStrategy.class, SnakeCasePageQueryFieldNamingStrategy.class, ProviderManager.class,
    ProtectedResourceWatchService.class, ProtectedResourceEncryptService.class,
    ProtectedResourceDesensitizeService.class, ProtectedResourceDecryptService.class,
    ProtectedResourceMonitorService.class, UnifiedConnectionCheckProvider.class, UnifiedResourceConnectionChecker.class,
    PluginConfigManager.class, AgentUnifiedService.class, MockResourceChecker.class,
    UnifiedClusterResourceIntegrityChecker.class, ProtectedEnvironmentServiceImpl.class,
    ProtectedEnvironmentRetrievalsService.class
})
@MockBean({
    SessionService.class, EncryptorService.class, ResourceProvider.class, ResourceLockRestApi.class,
    MessageTemplate.class, SchedulerService.class, JobScheduleService.class, LockService.class, SessionService.class
})
public class UnifiedConnectionCheckProviderTest {
    @Autowired
    private ResourceService resourceService;

    @Resource
    private ProtectedResourceExtendInfoMapper protectedResourceExtendInfoMapper;

    @Resource
    private ProtectedResourceMapper protectedResourceMapper;

    @MockBean
    private PluginConfigManager pluginConfigManager;

    @Autowired
    private ProviderManager providerManager;

    @MockBean
    private AgentUnifiedService agentUnifiedService;

    @MockBean
    private LockService lockService;

    @MockBean
    private ResourceExtensionManager resourceExtensionManager;

    @Qualifier("unifiedResourceConnectionChecker")
    private UnifiedResourceConnectionChecker unifiedResourceConnectionChecker;

    @Autowired
    private UnifiedConnectionCheckProvider unifiedConnectionCheckProvider;

    @MockBean
    private DeployTypeService deployTypeService;

    @MockBean(name = "defaultResourceProvider")
    private DefaultResourceProvider defaultResourceProvider;

    @MockBean
    private SystemSwitchInternalService systemSwitchInternalService;

    @MockBean
    private JobService jobService;

    @MockBean
    private CommonAlarmService commonAlarmService;

    @MockBean
    private ResourceScanService resourceScanService;

    @MockBean
    private ResourceAlarmService resourceAlarmService;

    @MockBean
    private CyberEngineResourceService cyberEngineResourceService;

    @MockBean
    private AgentQueryService agentQueryService;

    @Before
    public void prepare() {
        protectedResourceExtendInfoMapper.delete(new QueryWrapper<>());
        protectedResourceMapper.delete(new QueryWrapper<>());
        SystemSwitchDto systemSwitchDto = new SystemSwitchDto();
        systemSwitchDto.setStatus(SwitchStatusEnum.OFF);
        Mockito.when(systemSwitchInternalService.queryByName(Mockito.any())).thenReturn(systemSwitchDto);
    }

    /**
     * 用例名称：验证带环境信息的资源创建<br/>
     * 前置条件：环境信息在数据库中创建成功<br/>
     * check点：资源创建成功，创建回调数据完整<br/>
     */
    @Test
    public void check_resource_connection_success() {
        PluginConfig pluginConfig = new PluginConfig();
        pluginConfig.setType("HDFS");

        CollectableConfig collectableConfig = new CollectableConfig();
        collectableConfig.setEnvironment("hosts");

        PowerMockito.when(pluginConfigManager.getPluginConfig(eq("HDFS"))).thenReturn(Optional.of(pluginConfig));

        PowerMockito.when(resourceExtensionManager.invoke(eq("HDFS"), eq("functions.connection.dependency"), any()))
            .thenReturn(Collections.singletonList(collectableConfig));

        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        PowerMockito.when(agentUnifiedService.checkApplication(any(), any())).thenReturn(agentBaseDto);

        AppEnvResponse appEnvResponse = new AppEnvResponse();
        NodeInfo nodeInfo1 = new NodeInfo();
        nodeInfo1.setEndpoint("127.0.0.1");
        appEnvResponse.setNodes(Arrays.asList(nodeInfo1));
        PowerMockito.when(agentUnifiedService.getClusterInfo(any(), any())).thenReturn(appEnvResponse);

        ProtectedResource resource = new ProtectedResource();
        resource.setUserId(UUID.randomUUID().toString());
        resource.setSubType("HDFS");
        resource.setAuth(createAuthentication());
        resource.setExtendInfoByKey("username", "admin");
        resource.setExtendInfoByKey("password", "Admin@123");

        ProtectedResource host1 = new ProtectedResource();
        host1.setUuid(UUID.randomUUID().toString());
        ProtectedResource host2 = new ProtectedResource();
        host2.setUuid(UUID.randomUUID().toString());

        resource.setDependencies(Collections.singletonMap("hosts", Arrays.asList(host1, host2)));

        ProtectedEnvironment environment1 = new ProtectedEnvironment();
        environment1.setUsername("admin");
        environment1.setPassword("password");
        environment1.setName("environment-name");
        environment1.setOsType("linux");
        environment1.setSubType("protected_environment_Env1");
        environment1.setUuid(host1.getUuid());
        environment1.setAuth(createAuthentication());
        environment1.setEndpoint("127.0.0.1");

        ProtectedEnvironment environment2 = new ProtectedEnvironment();
        environment2.setUsername("admin");
        environment2.setPassword("password");
        environment2.setName("environment-name");
        environment2.setOsType("linux");
        environment2.setSubType("protected_environment_Env1");
        environment2.setUuid(host2.getUuid());
        environment2.setAuth(createAuthentication());
        environment2.setEndpoint("127.0.0.2");

        resourceService.create(new ProtectedResource[] {environment1, environment2});

        List<ActionResult> check = providerManager.findProvider(ResourceConnectionCheckProvider.class, resource)
            .tryCheckConnection(resource, unifiedResourceConnectionChecker)
            .getActionResults();
        Assert.assertEquals(2, check.size());
        Assert.assertEquals(0, check.get(0).getCode());
        Assert.assertEquals(0, check.get(1).getCode());
    }

    /**
     * 用例名称：检查连通性遇到错误时能被check住<br/>
     * 前置条件：无<br/>
     * check点：检查连通性过程中遇到错误，对返回结果check抛出异常<br/>
     */
    @Test
    public void action_result_should_be_check_when_failed() {
        ProtectedResource resource = new ProtectedResource();
        Assert.assertThrows(LegoCheckedException.class, () -> unifiedConnectionCheckProvider.checkConnection(resource));
    }

    private Authentication createAuthentication() {
        Authentication authentication = new Authentication();
        authentication.setAuthPwd("password");
        authentication.setExtendInfo(new HashMap<>(Collections.singletonMap("password", "password")));
        return authentication;
    }
}
