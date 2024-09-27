package openbackup.access.framework.resource;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.atLeastOnce;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import openbackup.access.framework.resource.dto.ResourceDependencyRelation;
import openbackup.access.framework.resource.mock.MockResourceChecker;
import openbackup.access.framework.resource.persistence.dao.ProtectedEnvironmentExtendInfoMapper;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceExtendInfoMapper;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceMapper;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceRepositoryImpl;
import openbackup.access.framework.resource.persistence.model.ProtectedAgentExtendPo;
import openbackup.access.framework.resource.persistence.model.ProtectedEnvironmentExtendInfoPo;
import openbackup.access.framework.resource.persistence.model.ProtectedEnvironmentPo;
import openbackup.access.framework.resource.persistence.model.ProtectedResourceExtendInfoPo;
import openbackup.access.framework.resource.persistence.model.ProtectedResourcePo;
import openbackup.access.framework.resource.provider.DefaultResourceProvider;
import openbackup.access.framework.resource.service.JobScheduleService;
import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.ProtectedEnvironmentServiceImpl;
import openbackup.access.framework.resource.service.ProtectedResourceDecryptService;
import openbackup.access.framework.resource.service.ProtectedResourceDesensitizeService;
import openbackup.access.framework.resource.service.ProtectedResourceEncryptService;
import openbackup.access.framework.resource.service.ProtectedResourceMonitorService;
import openbackup.access.framework.resource.service.ProtectedResourceRepository;
import openbackup.access.framework.resource.service.ProtectedResourceServiceImpl;
import openbackup.access.framework.resource.service.ProtectedResourceWatchService;
import openbackup.access.framework.resource.service.ResourceAlarmService;
import openbackup.access.framework.resource.service.ResourceAlarmServiceImpl;
import openbackup.access.framework.resource.service.ResourceScanService;
import openbackup.access.framework.resource.service.provider.UnifiedClusterResourceIntegrityChecker;
import openbackup.access.framework.resource.service.provider.UnifiedConnectionCheckProvider;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.access.framework.resource.validator.JsonSchemaValidatorImpl;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.dao.ProtectedObjectMapper;
import openbackup.data.access.framework.core.entity.ProtectedObjectPo;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.CollectableConfig;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfig;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.plugin.ResourceExtensionManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CyberEngineResourceService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceGroupResult;
import openbackup.data.protection.access.provider.sdk.resource.ResourceBase;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceQueryParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resource.VstoreResourceQueryParam;
import openbackup.data.protection.access.provider.sdk.resource.model.AgentTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.model.ResourceExtendInfoKeyConstants;
import openbackup.data.protection.access.provider.sdk.resource.model.ResourceUpsertRes;
import openbackup.data.protection.access.provider.sdk.util.ResourceCheckContextUtil;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.pack.lock.Lock;
import openbackup.system.base.pack.lock.LockService;
import openbackup.system.base.query.DefaultPageQueryFieldNamingStrategy;
import openbackup.system.base.query.PageQueryService;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.query.SnakeCasePageQueryFieldNamingStrategy;
import com.huawei.oceanprotect.system.base.schedule.service.SchedulerService;
import openbackup.system.base.sdk.alarm.CommonAlarmService;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.lock.ResourceLockRestApi;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.service.hostagent.AgentQueryService;
import openbackup.system.base.util.MessageTemplate;
import com.huawei.oceanprotect.system.sdk.dto.SystemSwitchDto;
import com.huawei.oceanprotect.system.sdk.enums.SwitchStatusEnum;
import com.huawei.oceanprotect.system.sdk.service.SystemSwitchInternalService;

import com.baomidou.mybatisplus.autoconfigure.MybatisPlusAutoConfiguration;
import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.core.metadata.IPage;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;
import com.fasterxml.jackson.databind.JsonNode;
import com.google.common.collect.Lists;

import org.apache.commons.lang3.StringUtils;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mockito;
import org.mockito.internal.util.collections.Sets;
import org.mybatis.spring.annotation.MapperScan;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.api.support.membermodification.MemberMatcher;
import org.powermock.api.support.membermodification.MemberModifier;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.powermock.reflect.Whitebox;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration;
import org.springframework.boot.autoconfigure.sql.init.SqlInitializationAutoConfiguration;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.annotation.ComponentScan;
import org.springframework.core.io.support.PathMatchingResourcePatternResolver;
import org.springframework.core.io.support.ResourcePatternResolver;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.util.ReflectionTestUtils;

import java.io.IOException;
import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;
import java.util.stream.IntStream;
import java.util.stream.Stream;

import javax.annotation.Resource;

/**
 * Resource Integration Test
 *
 * @author l00272247
 * @since 2022-01-24
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@ComponentScan(basePackages = {"openbackup.access.framework.resource.persistence.dao"})
@MapperScan(basePackages = {
    "openbackup.access.framework.resource.persistence.dao",
    "openbackup.data.access.framework.core.dao"
})
@SpringBootTest(classes = {
    DataSourceAutoConfiguration.class, MybatisPlusAutoConfiguration.class, SqlInitializationAutoConfiguration.class,
    ProtectedResourceServiceImpl.class, ProtectedResourceRepositoryImpl.class, JsonSchemaValidatorImpl.class,
    PageQueryService.class, DefaultPageQueryFieldNamingStrategy.class, SnakeCasePageQueryFieldNamingStrategy.class,
    ProviderManager.class, ProtectedResourceWatchService.class, ProtectedResourceEncryptService.class,
    ProtectedResourceDesensitizeService.class, ProtectedResourceDecryptService.class,
    ProtectedResourceMonitorService.class, UnifiedConnectionCheckProvider.class, UnifiedResourceConnectionChecker.class,
    PluginConfigManager.class, AgentUnifiedService.class, MockResourceChecker.class,
    UnifiedClusterResourceIntegrityChecker.class, ProtectedEnvironmentServiceImpl.class,
    ProtectedEnvironmentRetrievalsService.class, ResourceAlarmServiceImpl.class
})
@MockBean({
    SessionService.class, EncryptorService.class, ResourceProvider.class, ResourceLockRestApi.class,
    MessageTemplate.class, SchedulerService.class, JobScheduleService.class, LockService.class, SessionService.class,
    DeployTypeService.class, CommonAlarmService.class
})
public class ResourceIntegrationTest {
    private static final String RESOURCE_ID = UUID.randomUUID().toString();

    @Autowired
    private ResourceService resourceService;

    @Resource
    private ProtectedResourceExtendInfoMapper protectedResourceExtendInfoMapper;

    @Resource
    private ProtectedResourceMapper protectedResourceMapper;

    @Resource
    private ProtectedObjectMapper protectedObjectMapper;

    @Autowired
    private SessionService sessionService;

    @Resource
    private ProtectedEnvironmentExtendInfoMapper protectedEnvironmentExtendInfoMapper;

    @Autowired
    private EncryptorService encryptorService;

    @MockBean
    private PluginConfigManager pluginConfigManager;

    @MockBean
    private AgentQueryService agentQueryService;

    @MockBean
    private AgentUnifiedService agentUnifiedService;

    @MockBean
    private LockService lockService;

    @MockBean
    private ResourceExtensionManager resourceExtensionManager;

    @MockBean
    CommonAlarmService commonAlarmService;

    @Autowired
    ProtectedResourceServiceImpl protectedResourceServices;

    @Autowired
    ProtectedResourceRepositoryImpl protectedResourceRepository;

    @MockBean
    private JobService jobService;

    @MockBean
    private SystemSwitchInternalService systemSwitchInternalService;

    @MockBean(name = "defaultResourceProvider")
    private DefaultResourceProvider resourceProvider;

    @MockBean
    private ResourceScanService resourceScanService;

    @MockBean
    private JobCenterRestApi jobCenterRestApi;

    @Autowired
    private ResourceAlarmService resourceAlarmService;

    @MockBean
    private CyberEngineResourceService cyberEngineResourceService;

    private static final String PAGE_NO = "pageNo";

    private static final String PAGE_SIZE = "pageSize";

    private static final String CPU_RATE = "cpuRate";

    private static final String MEM_RATE = "memRate";

    @Before
    public void prepare() {
        protectedResourceExtendInfoMapper.delete(new QueryWrapper<>());
        protectedResourceMapper.delete(new QueryWrapper<>());
        SystemSwitchDto systemSwitchDto = new SystemSwitchDto();
        systemSwitchDto.setStatus(SwitchStatusEnum.OFF);
        Mockito.when(systemSwitchInternalService.queryByName(Mockito.any())).thenReturn(systemSwitchDto);
        Lock lockMock = Mockito.mock(Lock.class);
        Mockito.when(lockMock.tryLock(Mockito.anyLong(), Mockito.any(TimeUnit.class))).thenReturn(true);
        Mockito.when(lockService.createSQLDistributeLock(Mockito.any())).thenReturn(lockMock);
        Mockito.when(lockService.createDistributeLock(Mockito.any())).thenReturn(lockMock);
    }

    @Test
    public void test_create_resource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("uuid");
        resource.setUserId("admin");
        resource.setRootUuid("root-uuid");
        resource.setParentUuid("uuid");
        resource.setSubType("protected_resource_Res");
        create_resource(resource);
        ProtectedResource protectedResource = getProtectedResource();
        Assert.assertEquals(Integer.valueOf(0), protectedResource.getProtectionStatus());
    }

    private void initial_resource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("uuid");
        resource.setUserId("admin");
        resource.setRootUuid("root-uuid");
        resource.setParentUuid("uuid");
        resource.setSubType("protected_resource_Res");
        create_resource(resource);
    }

    @Test
    public void test_create_environment() {
        mockKmcRestApi();
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUserId("admin");
        environment.setEndpoint("localhost");
        environment.setUsername("admin");
        environment.setPassword("password");
        environment.setName("environment-name");
        environment.setOsType("linux");
        environment.setSubType("protected_environment_Env");
        environment.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
        environment.setExtendInfoByKey("key", "value");
        environment.setExtendInfoByKey("dependencies",
            new JSONObject().set("host", new String[] {RESOURCE_ID}).toString());
        mockCurrentUser();

        create_resource(environment);
        List<ProtectedEnvironmentExtendInfoPo> list = protectedEnvironmentExtendInfoMapper.selectList(null);
        Assert.assertEquals(1, list.size());

        ProtectedEnvironmentExtendInfoPo protectedEnvironmentExtendInfoPo = list.get(0);
        String uuid = protectedEnvironmentExtendInfoPo.getUuid();
        PageListResponse<ProtectedResource> response = resourceService.query(0, 10,
            Collections.singletonMap("uuid", uuid));
        List<ProtectedResource> resources = response.getRecords();
        Assert.assertEquals(1, resources.size());

        ProtectedResource resource = resources.get(0);
        Assert.assertTrue(resource instanceof ProtectedEnvironment);
        Assert.assertEquals("admin", ((ProtectedEnvironment) resource).getUsername());
    }

    /**
     * 用例名称：验证ProtectedResource环境资源对象的更新接口基本功能是否正常。<br/>
     * 前置条件：ProtectedResource环境资源对象入库成功。<br/>
     * check点：<br/>
     * 1、非truncate模式下，更新扩展属性，可保留未指定属性值；<br/>
     * 2、truncate模式下，更新扩展属性，可清楚未指定属性值。<br/>
     */
    @Test
    public void test_update_resource() {
        PowerMockito.when(encryptorService.encrypt(any())).thenAnswer(invocation -> invocation.getArgument(0));
        test_create_resource();
        mockCurrentUser();

        ProtectedResource resource0 = getProtectedResource();
        resource0.setPath("path");
        resource0.getExtendInfo().put("username", "username");
        resourceService.update(new ProtectedResource[] {resource0});
        ProtectedResource resource1 = getProtectedResource();
        Assert.assertEquals("path", resource1.getPath());
        Assert.assertEquals("username", resource1.getExtendInfoByKey("username"));

        resource1.setPath("/");
        resource1.setName("name");
        resource1.getExtendInfo().put("username", "admin");
        resource1.getExtendInfo().put("password", "Admin@123");
        resourceService.update(new ProtectedResource[] {resource1});
        ProtectedResource resource2 = getProtectedResource();
        Assert.assertEquals("/", resource2.getPath());
        Assert.assertEquals("admin", resource2.getExtendInfoByKey("username"));
        Assert.assertEquals("Admin@123", resource2.getExtendInfoByKey("password"));

        resource2.getExtendInfo().remove("username");
        resource2.getExtendInfo().put("password", "new-password");
        resourceService.update(new ProtectedResource[] {resource2});
        ProtectedResource resource3 = getProtectedResource();
        Assert.assertEquals("/", resource3.getPath());
        Assert.assertEquals("admin", resource3.getExtendInfoByKey("username"));
        Assert.assertEquals("new-password", resource3.getExtendInfoByKey("password"));

        resource3.getExtendInfo().remove("username");
        resource3.getExtendInfo().put("password", "password");
        resourceService.update(true, new ProtectedResource[] {resource3});
        ProtectedResource resource4 = getProtectedResource();
        Assert.assertEquals("/", resource4.getPath());
        Assert.assertNull(resource4.getExtendInfoByKey("username"));
        Assert.assertFalse(resource4.getExtendInfo().containsKey("username"));
        Assert.assertEquals("password", resource4.getExtendInfoByKey("password"));
    }

    /**
     * 用例名称：验证 更新环境时，代理主机能够更新subtype字段
     * 前置条件：环境信息在数据库中创建成功
     * check点：更新环境时，代理主机能够更新subtype字段
     */
    @Test
    public void test_update_resource_when_agent_update_can_update_subtype() {
        PowerMockito.when(encryptorService.encrypt(any())).thenAnswer(invocation -> invocation.getArgument(0));
        test_create_resource();
        mockCurrentUser();

        ProtectedResource resource = getProtectedResource();
        Assert.assertEquals(resource.getSubType(), "protected_resource_Res");

        resource.setSubType(ResourceSubTypeEnum.DB_BACKUP_AGENT.getType());
        resourceService.update(true, new ProtectedResource[] {resource});
        ProtectedResource resourceAfter = getProtectedResource();
        Assert.assertEquals(resourceAfter.getSubType(), "DBBackupAgent");
    }

    /**
     * 用例名称：验证HCSClientHost类型可以直接更新
     * 前置条件：环境信息在数据库中创建成功
     * check点：更新环境时，能够更新name字段
     */
    @Test
    public void test_update_resource_when_hcs_update_can_update() {
        PowerMockito.when(encryptorService.encrypt(any())).thenAnswer(invocation -> invocation.getArgument(0));
        ResourceFeature resourceFeature = new ResourceFeature();
        resourceFeature.setShouldSaveDirectlyWhenScan(true);
        PowerMockito.when(resourceProvider.getResourceFeature()).thenReturn(resourceFeature);
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("uuid");
        resource.setUserId("admin");
        resource.setRootUuid("root-uuid");
        resource.setParentUuid("uuid");
        resource.setSourceType("autoscan");
        resource.setSubType(ResourceSubTypeEnum.HCS_CLOUD_HOST.getType());
        create_resource(resource);
        ProtectedResource protectedResource = getProtectedResource();
        Assert.assertEquals(Integer.valueOf(0), protectedResource.getProtectionStatus());
        mockCurrentUser();

        resource.setName("a2");
        resourceService.update(true, new ProtectedResource[] {resource});
        ProtectedResource resourceAfter = getProtectedResource();
        Assert.assertEquals(resourceAfter.getName(), "a2");
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
        resource.setUserId(RESOURCE_ID);
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

        ActionResult[] check = resourceService.check(resource);
        Assert.assertEquals(2, check.length);
        Assert.assertEquals(0, check[0].getCode());
        Assert.assertEquals(0, check[1].getCode());
    }

    /**
     * 用例名称：验证带环境信息的资源创建<br/>
     * 前置条件：环境信息在数据库中创建成功<br/>
     * check点：资源创建成功，创建回调数据完整<br/>
     */
    @Test
    public void check_resource_connection_success_when_config_environment_is_null() {
        PluginConfig pluginConfig = new PluginConfig();
        pluginConfig.setType("HDFS");

        CollectableConfig collectableConfig = new CollectableConfig();

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
        resource.setUserId(RESOURCE_ID);
        resource.setSubType("HDFS");
        resource.setParentUuid(UUID.randomUUID().toString());
        resource.setAuth(createAuthentication());
        resource.setExtendInfoByKey("username", "admin");
        resource.setExtendInfoByKey("password", "Admin@123");

        ProtectedEnvironment environment1 = new ProtectedEnvironment();
        environment1.setUsername("admin");
        environment1.setPassword("password");
        environment1.setName("environment-name");
        environment1.setOsType("linux");
        environment1.setSubType("protected_environment_Env1");
        environment1.setUuid(resource.getParentUuid());
        environment1.setAuth(createAuthentication());
        environment1.setEndpoint("127.0.0.1");

        resourceService.create(new ProtectedResource[] {environment1});

        ActionResult[] check = resourceService.check(resource);
        Assert.assertEquals(1, check.length);
        Assert.assertEquals(0, check[0].getCode());
    }

    /**
     * 用例名称：资源以及全部保存后，选择其他主机测试成功<br/>
     * 前置条件：1、测试资源已经保存；2、选择主机也是保存好<br/>
     * check点：测试成功，且获取到检查结果为0
     */
    @Test
    public void check_resource_connection_success_when_resource_is_already_saved() {
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

        Lock lockMock = Mockito.mock(Lock.class);
        Mockito.when(lockMock.tryLock(Mockito.anyLong(), Mockito.any(TimeUnit.class))).thenReturn(true);
        Mockito.when(lockService.createDistributeLock(Mockito.any())).thenReturn(lockMock);

        ProtectedResource resource = new ProtectedResource();
        resource.setUserId(RESOURCE_ID);
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

        resourceService.create(new ProtectedResource[] {resource});

        PageListResponse<ProtectedResource> resourcePageListResponse = resourceService.query(0, 1,
            Collections.singletonMap("subType", "HDFS"));
        Assert.assertEquals(1, resourcePageListResponse.getRecords().size());

        resource.setUuid(resourcePageListResponse.getRecords().get(0).getUuid());
        Authentication authentication = createAuthentication();
        authentication.setAuthPwd("change_password");
        resource.setAuth(authentication);

        ActionResult[] check = resourceService.check(resource);
        Assert.assertEquals(2, check.length);
        Assert.assertEquals(0, check[0].getCode());
        Assert.assertEquals(0, check[1].getCode());
    }

    /**
     * 用例名称：当资源有子资源时，此时不进行校验<br/>
     * 前置条件：1、测试资源已经保存；2、选择主机也是保存好<br/>
     * check点：测试成功，且获取到检查结果为0
     */
    @Test
    public void check_resource_connection_success_when_resource_has_children_node() {
        PluginConfig pluginConfig = new PluginConfig();
        pluginConfig.setType("HDFS");

        CollectableConfig collectableConfig = new CollectableConfig();
        collectableConfig.setResource("children");
        collectableConfig.setEnvironment("agents");

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

        Lock lockMock = Mockito.mock(Lock.class);
        Mockito.when(lockMock.tryLock(Mockito.anyLong(), Mockito.any(TimeUnit.class))).thenReturn(true);
        Mockito.when(lockService.createDistributeLock(Mockito.any())).thenReturn(lockMock);

        ProtectedResource resource = new ProtectedResource();
        resource.setUserId(RESOURCE_ID);
        resource.setSubType("HDFS");
        resource.setAuth(createAuthentication());
        resource.setExtendInfoByKey("username", "admin");
        resource.setExtendInfoByKey("password", "Admin@123");

        ProtectedResource host1 = new ProtectedResource();
        host1.setUuid(UUID.randomUUID().toString());
        ProtectedResource host2 = new ProtectedResource();
        host2.setUuid(UUID.randomUUID().toString());

        ProtectedResource node1 = new ProtectedResource();
        node1.setAuth(createAuthentication());
        node1.setDependencies(Collections.singletonMap("agents", Arrays.asList(host1)));

        ProtectedResource node2 = new ProtectedResource();
        node2.setAuth(createAuthentication());
        node2.setDependencies(Collections.singletonMap("agents", Arrays.asList(host2)));

        resource.setDependencies(Collections.singletonMap("children", Arrays.asList(node1, node2)));

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

        ActionResult[] check = resourceService.check(resource);
        Assert.assertEquals(2, check.length);
        Assert.assertEquals(0, check[0].getCode());
        Assert.assertEquals(0, check[1].getCode());
    }

    @Test
    public void test_extended_checker_success() {
        PluginConfig pluginConfig = new PluginConfig();
        pluginConfig.setType("mock_sub_type");

        CollectableConfig collectableConfig = new CollectableConfig();
        collectableConfig.setEnvironment("agents");

        PowerMockito.when(pluginConfigManager.getPluginConfig(eq("mock_sub_type")))
            .thenReturn(Optional.of(pluginConfig));

        PowerMockito.when(
                resourceExtensionManager.invoke(eq("mock_sub_type"), eq("functions.connection.dependency"), any()))
            .thenReturn(Collections.singletonList(collectableConfig));

        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        PowerMockito.when(agentUnifiedService.checkApplication(any(), any())).thenReturn(agentBaseDto);

        AppEnvResponse appEnvResponse = new AppEnvResponse();
        NodeInfo nodeInfo1 = new NodeInfo();
        nodeInfo1.setName("environment-name");
        appEnvResponse.setNodes(Arrays.asList(nodeInfo1));
        PowerMockito.when(agentUnifiedService.getClusterInfo(any(), any())).thenReturn(appEnvResponse);

        ProtectedResource resource = new ProtectedResource();
        resource.setUserId(RESOURCE_ID);
        resource.setSubType("mock_sub_type");
        resource.setAuth(createAuthentication());
        resource.setExtendInfoByKey("username", "admin");
        resource.setExtendInfoByKey("password", "Admin@123");

        ProtectedResource host1 = new ProtectedResource();
        host1.setUuid(UUID.randomUUID().toString());

        resource.setDependencies(Collections.singletonMap("agents", Arrays.asList(host1)));

        ProtectedEnvironment environment1 = new ProtectedEnvironment();
        environment1.setUsername("admin");
        environment1.setPassword("password");
        environment1.setName("environment-name");
        environment1.setOsType("linux");
        environment1.setSubType("protected_environment_Env1");
        environment1.setUuid(host1.getUuid());
        environment1.setAuth(createAuthentication());
        environment1.setEndpoint("127.0.0.1");

        resourceService.create(new ProtectedResource[] {environment1});

        ActionResult[] check = resourceService.check(resource);
        Assert.assertEquals(0, check.length);
    }

    @Test
    public void test_extended_checker_fail_when_cluster_nodes_and_agents_not_match() {
        PluginConfig pluginConfig = new PluginConfig();
        pluginConfig.setType("mock_sub_type");

        CollectableConfig collectableConfig = new CollectableConfig();
        collectableConfig.setEnvironment("agents");

        PowerMockito.when(pluginConfigManager.getPluginConfig(eq("mock_sub_type")))
            .thenReturn(Optional.of(pluginConfig));

        PowerMockito.when(
                resourceExtensionManager.invoke(eq("mock_sub_type"), eq("functions.connection.dependency"), any()))
            .thenReturn(Collections.singletonList(collectableConfig));

        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        PowerMockito.when(agentUnifiedService.checkApplication(any(), any())).thenReturn(agentBaseDto);

        AppEnvResponse appEnvResponse = new AppEnvResponse();
        NodeInfo nodeInfo1 = new NodeInfo();
        nodeInfo1.setName("environment-name");
        nodeInfo1.setEndpoint("127.0.0.1");
        NodeInfo nodeInfo2 = new NodeInfo();
        nodeInfo1.setName("environment-name-1");
        nodeInfo1.setEndpoint("127.0.0.2");
        appEnvResponse.setNodes(Arrays.asList(nodeInfo1, nodeInfo2));
        PowerMockito.when(agentUnifiedService.getClusterInfo(any(), any())).thenReturn(appEnvResponse);

        ProtectedResource resource = new ProtectedResource();
        resource.setUserId(RESOURCE_ID);
        resource.setSubType("mock_sub_type");
        resource.setAuth(createAuthentication());
        resource.setExtendInfoByKey("username", "admin");
        resource.setExtendInfoByKey("password", "Admin@123");

        ProtectedResource host1 = new ProtectedResource();
        host1.setUuid(UUID.randomUUID().toString());

        resource.setDependencies(Collections.singletonMap("agents", Arrays.asList(host1)));

        ProtectedEnvironment environment1 = new ProtectedEnvironment();
        environment1.setUsername("admin");
        environment1.setPassword("password");
        environment1.setName("environment-name");
        environment1.setOsType("linux");
        environment1.setSubType("protected_environment_Env1");
        environment1.setUuid(host1.getUuid());
        environment1.setAuth(createAuthentication());
        environment1.setEndpoint("127.0.0.1");
        resourceService.create(new ProtectedResource[] {environment1});

        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> resourceService.check(resource, true));
        Assert.assertEquals(ResourceCheckContextUtil.UNION_ERROR, legoCheckedException.getErrorCode());
    }

    @Test
    public void test_queryAgentResourceList() {
        List<ProtectedResourcePo> mockList = new ArrayList<>();

        // 在线同时新增扩展信息表有agent上报数据
        ProtectedEnvironmentPo protectedEnvironment1 = new ProtectedEnvironmentPo();
        protectedEnvironment1.setLinkStatus("1");
        protectedEnvironment1.setProtectedAgentExtendPo(new ProtectedAgentExtendPo());
        protectedEnvironment1.setCreatedTime(new Timestamp(System.currentTimeMillis()));
        ProtectedResourcePo protectedResourcePo1 = protectedEnvironment1;

        // 在线同时新增扩展信息表无agent上报数据
        ProtectedEnvironmentPo protectedEnvironment2 = new ProtectedEnvironmentPo();
        protectedEnvironment2.setLinkStatus("1");
        protectedEnvironment2.setCreatedTime(new Timestamp(System.currentTimeMillis()));
        ProtectedResourcePo protectedResourcePo2 = protectedEnvironment2;

        // 离线同时新增扩展信息表无agent上报数据
        ProtectedEnvironmentPo protectedEnvironment3 = new ProtectedEnvironmentPo();
        protectedEnvironment3.setLinkStatus("0");
        protectedEnvironment3.setCreatedTime(new Timestamp(System.currentTimeMillis()));
        ProtectedResourcePo protectedResourcePo3 = protectedEnvironment3;

        mockList.add(protectedResourcePo1);
        mockList.add(protectedResourcePo2);
        mockList.add(protectedResourcePo3);
        ProtectedResourceRepository mockRepository = PowerMockito.mock(ProtectedResourceRepository.class);
        Whitebox.setInternalState(resourceService, "repository", mockRepository);
        PowerMockito.when(mockRepository.queryAgentResourceList(any())).thenReturn(mockList);
        PowerMockito.when(mockRepository.queryAgentResourceCount(any())).thenReturn(1);
        Map<String, Object> map = new HashMap<>();
        map.put(PAGE_NO, 0);
        map.put(PAGE_SIZE, 1);
        map.put(CPU_RATE, "asc");
        map.put(MEM_RATE, "desc");
        map.put("isShared", Lists.newArrayList(false));
        PageListResponse<ProtectedResource> response = resourceService.queryAgentResourceList(map);
        Whitebox.setInternalState(resourceService, "repository", protectedResourceRepository);
        Assert.assertEquals(response.getRecords().size(), 1);
    }

    @Test
    public void test_query_resource_dependency_relation_success() {
        ProtectedResourcePo parent = new ProtectedResourcePo();
        parent.setUuid(UUID.randomUUID().toString());
        parent.setParentUuid(null);
        parent.setCreatedTime(new Timestamp(System.currentTimeMillis()));

        ProtectedResourcePo children = new ProtectedResourcePo();
        children.setUuid(UUID.randomUUID().toString());
        children.setParentUuid(parent.getUuid());
        children.setCreatedTime(new Timestamp(System.currentTimeMillis()));

        ProtectedResourcePo dependency = new ProtectedResourcePo();
        dependency.setUuid(UUID.randomUUID().toString());
        dependency.setParentUuid(dependency.getUuid());
        dependency.setCreatedTime(new Timestamp(System.currentTimeMillis()));

        protectedResourceMapper.insert(parent);
        protectedResourceMapper.insert(children);
        protectedResourceMapper.insert(dependency);

        ProtectedResourceExtendInfoPo protectedResourceExtendInfoPo = new ProtectedResourceExtendInfoPo();
        protectedResourceExtendInfoPo.setUuid(UUID.randomUUID().toString());
        protectedResourceExtendInfoPo.setKey("$citations_hosts_24412121");
        protectedResourceExtendInfoPo.setResourceId(dependency.getUuid());
        protectedResourceExtendInfoPo.setValue(children.getUuid());

        protectedResourceExtendInfoMapper.insert(protectedResourceExtendInfoPo);

        System.out.println(
            protectedResourceMapper.queryRelatedResourceUuids(Collections.singletonList(parent.getUuid())));

        List<ResourceDependencyRelation> resourceDependencyRelations
            = protectedResourceMapper.queryResourceDependencyRelation(Collections.singletonList(parent.getUuid()));

        Assert.assertEquals(Sets.newSet(parent.getUuid(), children.getUuid(), dependency.getUuid()),
            resourceDependencyRelations.stream().map(ResourceDependencyRelation::getUuid).collect(Collectors.toSet()));
    }

    /**
     * query protected resource
     *
     * @return protected resource
     */
    protected ProtectedResource getProtectedResource() {
        PageListResponse<ProtectedResource> response = resourceService.query(0, 10, null);
        return response.getRecords().get(0);
    }

    @Test
    public void test_update_environment() {
        test_create_environment();
        ProtectedEnvironment environment0 = (ProtectedEnvironment) getProtectedResource();
        environment0.setEndpoint("127.0.0.1");
        environment0.setOsType("windows");
        environment0.setExtendInfoByKey("field0", "value0");
        resourceService.update(new ProtectedResource[] {environment0});
        ProtectedEnvironment environment1 = (ProtectedEnvironment) getProtectedResource();
        Assert.assertEquals("127.0.0.1", environment1.getEndpoint());
        Assert.assertEquals("windows", environment1.getOsType());
        Assert.assertEquals("value0", environment1.getExtendInfoByKey("field0"));
    }

    /**
     * 用例名称：验证直接更新资源属性到数据库方法功能是否正常。<br/>
     * 前置条件：源数据准备完成。<br/>
     * check点：合并结果正确。<br/>
     */
    @Test
    public void test_update_source_directly() {
        // 测试更新资源
        PowerMockito.when(encryptorService.encrypt(any())).thenAnswer(invocation -> invocation.getArgument(0));
        initial_resource();
        ProtectedResource resourceUpdate = getProtectedResource();
        resourceUpdate.setPath("path");
        resourceService.updateSourceDirectly(Stream.of(resourceUpdate).collect(Collectors.toList()));
        ProtectedResource resource1 = getProtectedResource();
        Assert.assertEquals("path", resource1.getPath());

        // 测试更新环境资源
        this.create_environment();
        ProtectedEnvironment environmentOld = (ProtectedEnvironment) getProtectedResource();
        String uuid = environmentOld.getUuid();
        ProtectedEnvironment environmentNew = new ProtectedEnvironment();
        environmentNew.setUuid(uuid);
        environmentNew.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        resourceService.updateSourceDirectly(Stream.of(environmentNew).collect(Collectors.toList()));
        ProtectedEnvironment resource2 = (ProtectedEnvironment) getProtectedResource();
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), resource2.getLinkStatus());
    }

    private void create_environment() {
        mockKmcRestApi();
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUserId("admin");
        environment.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
        environment.setSubType(ResourceSubTypeEnum.NAS_SHARE.getType());
        create_resource(environment);
    }

    /**
     * 用例名称：验证创建资源产经，资源扩展属性存在敏感信息的情况下，扩展字段加密功能是否正确。<br/>
     * 前置条件：JsonSchema配置准备就绪。<br/>
     * check点：资源变更时间回调机制正常工作无异常抛出，敏感信息被正确加密。<br/>
     */
    @Test
    public void test_resource_with_secret() {
        mockCurrentUser();
        mockResourceProvider();
        mockKmcRestApi();

        ProtectedResource resource0 = new ProtectedResource();
        resource0.setUserId("admin");
        resource0.setSubType("HDFS");
        resource0.setAuth(createAuthentication());
        resource0.setExtendInfoByKey("username", "admin");
        resource0.setExtendInfoByKey("password", "Admin@123");

        ProtectedResource resource1 = new ProtectedResource();
        resource1.setUserId("admin");
        resource1.setSubType("HDFS");
        resource1.setExtendInfoByKey("username", "admin");
        resource1.setExtendInfoByKey("password", "Admin@123");
        resource1.setAuth(createAuthentication());
        create_resource(resource0, resource1);

        List<ProtectedResource> resources = resourceService.query(0, 10, null).getRecords();
        for (ProtectedResource resource : resources) {
            Assert.assertNotNull(resource);
            String password = resource.getExtendInfoByKey("password");
            Assert.assertNotNull(password);
            Assert.assertEquals("$Admin@123$", password);
            Authentication auth = resource.getAuth();
            Assert.assertNotNull(auth);
            Assert.assertNotNull(auth.getExtendInfo());
            Assert.assertEquals("$password$", auth.getAuthPwd());
            Assert.assertEquals("$password$", auth.getExtendInfo().get("password"));
        }
    }

    private void mockKmcRestApi() {
        PowerMockito.when(encryptorService.decrypt(anyString())).thenAnswer(invocation -> {
            String text = invocation.getArgument(0);
            return "$" + text;
        });
        PowerMockito.when(encryptorService.encrypt(any())).thenAnswer(invocation -> {
            String vo = invocation.getArgument(0);
            return vo + "$";
        });
    }

    private Authentication createAuthentication() {
        Authentication authentication = new Authentication();
        authentication.setAuthPwd("password");
        authentication.setExtendInfo(new HashMap<>(Collections.singletonMap("password", "password")));
        return authentication;
    }

    private void mockResourceProvider() {
        PowerMockito.when(resourceProvider.applicable(any())).thenReturn(true);
    }

    @Test
    public void test_query_resource_with_protected_object() {
        create_resource();

        ProtectedResource resource0 = getProtectedResource();
        ProtectedObject protectedObject0 = resource0.getProtectedObject();
        Assert.assertNull(protectedObject0);

        String uuid = resource0.getUuid();
        ProtectedObjectPo protectedObjectPo = new ProtectedObjectPo();
        protectedObjectPo.setUuid(uuid);
        protectedObjectPo.setType("type");
        protectedObjectPo.setResourceId(uuid);
        protectedObjectPo.setChainId(uuid);
        protectedObjectMapper.insert(protectedObjectPo);

        ProtectedResource resource1 = getProtectedResource();
        ProtectedObject protectedObject1 = resource1.getProtectedObject();
        Assert.assertNotNull(protectedObject1);
        Assert.assertEquals(uuid, protectedObject1.getUuid());
    }

    /**
     * 用例名称：通过DAO接口查询资源功能是否正常。 前置条件：资源在数据库中已创建成功 check点：资源查询成功
     */
    @Test
    public void test_query_resource_by_mapper() {
        Map<String, String> properties = create_resource();
        Page<ProtectedResourcePo> page = new Page<>(1, 10);
        QueryWrapper<ProtectedResourcePo> wrapper = new QueryWrapper<>();
        IPage<ProtectedEnvironmentPo> result = protectedResourceMapper.paginate(page, wrapper, StringUtils.EMPTY);

        List<ProtectedEnvironmentPo> resourcePoList = result.getRecords();
        Assert.assertNotNull(resourcePoList);

        ProtectedResourcePo resourcePo = resourcePoList.get(0);
        List<ProtectedResourceExtendInfoPo> extendInfoList = resourcePo.getExtendInfoList();
        Assert.assertNotNull(extendInfoList);
        Assert.assertEquals(5, extendInfoList.size());

        Set<String> expect = properties.keySet();
        Set<String> actual = extendInfoList.stream()
            .map(ProtectedResourceExtendInfoPo::getKey)
            .collect(Collectors.toSet());
        Assert.assertEquals(expect, actual);
    }

    private Map<String, String> create_resource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setName("resource-name");
        resource.setUserId("admin");
        resource.setSubType("protected_resource_Res");
        Map<String, String> properties = new HashMap<>();
        properties.put("username", "username");
        properties.put("password", "password");
        properties.put("tenantName", "System_vStore");
        properties.put("yyy", "2");
        properties.put("isAddLanFree", "1");
        resource.setExtendInfo(properties);
        create_resource(resource);

        mockCurrentUser();
        return properties;
    }

    private void mockCurrentUser() {
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setRoles(Collections.emptyList());
        userBo.setId("admin");
        PowerMockito.when(sessionService.getCurrentUser()).thenReturn(userBo);
    }

    /**
     * 用例名称：验证资源服务层查询资源<br/>
     * 前置条件：资源在数据库中已创建成功<br/>
     * check点：<br/>
     * 1. 资源查询成功<br/>
     * 2. 资源查询顺序正确<br/>
     */
    @Test
    public void test_query_resource_by_service() {
        Map<String, String> properties = create_resource();

        test_query_resource_result(properties, null, 1);

        for (int i = 0; i < 5; i++) {
            Map<String, String> props = create_resource();
            test_query_resource_result(props, null, 2 + i);
        }

        List<List<String>> records = IntStream.range(0, 10)
            .mapToObj(index -> resourceService.query(0, 10, null).getRecords())
            .map(list -> list.stream()
                .map(item -> item.getUuid() + ":" + item.getCreatedTime())
                .collect(Collectors.toList()))
            .collect(Collectors.toList());
        long count = records.stream().distinct().count();
        Assert.assertEquals(1, count);
    }

    private void test_query_resource_result(Map<String, String> properties, Map<String, Object> conditions, int count) {
        PageListResponse<ProtectedResource> result = resourceService.query(0, 10, clone(conditions));

        List<ProtectedResource> resources = result.getRecords();
        Assert.assertNotNull(resources);
        Assert.assertEquals(count, resources.size());
        if (resources.size() > 0) {
            Assert.assertEquals(count, resources.size());

            ProtectedResource resource = resources.get(0);
            Assert.assertEquals(properties, resource.getExtendInfo());
        }
    }

    private Map<String, Object> clone(Map<String, Object> conditions) {
        if (conditions == null) {
            return null;
        }
        Map<String, Object> object = new HashMap<>(conditions);
        conditions.forEach((key, value) -> {
            if (value instanceof Map) {
                @SuppressWarnings("unchecked") Map<String, Object> data = (Map<String, Object>) value;
                object.put(key, clone(data));
            }
        });
        return object;
    }

    @Test
    public void test_query_resource_by_service_with_condition() {
        PowerMockito.when(encryptorService.encrypt(any())).thenAnswer(invocation -> invocation.getArgument(0));
        Map<String, String> properties = create_resource();
        test_query_resource_result(properties, Collections.singletonMap("username", "username"), 1);
        test_query_resource_result(properties, Collections.singletonMap("password", "password"), 1);

        Map<String, Object> condition0 = new HashMap<>();
        condition0.put("username", "username");
        condition0.put("password", "password");
        test_query_resource_result(properties, condition0, 1);

        Map<String, Object> condition1 = new HashMap<>();
        condition1.put("username", "password");
        condition1.put("password", "username");
        test_query_resource_result(properties, condition1, 0);

        Map<String, Object> condition2 = new HashMap<>();
        condition2.put("username", Arrays.asList(Collections.singleton("in"), "username", "password"));
        condition2.put("password", Arrays.asList(Collections.singleton("in"), "username", "password"));
        test_query_resource_result(properties, condition2, 1);

        Map<String, Object> condition3 = Collections.singletonMap("name", "resource-name");
        test_query_resource_result(properties, condition3, 1);

        Map<String, Object> condition4 = Collections.singletonMap("name", "resource-name1");
        test_query_resource_result(properties, condition4, 0);

        Map<String, Object> condition5 = Collections.singletonMap("name",
            Arrays.asList(Collections.singleton("~~"), "name"));
        test_query_resource_result(properties, condition5, 1);

        Map<String, Object> condition6 = Collections.singletonMap("name",
            Arrays.asList(Collections.singleton("~~"), "name1"));
        test_query_resource_result(properties, condition6, 0);

        Map<String, Object> condition7 = Collections.singletonMap("linkStatus",
            Arrays.asList(Collections.singleton("in"), "0", "1"));
        test_query_resource_result(properties, condition7, 0);

        Map<String, Object> condition8 = Collections.singletonMap("name",
            Arrays.asList(Collections.singleton("!="), "resource-name"));
        test_query_resource_result(properties, condition8, 0);

        Map<String, Object> condition9 = Collections.singletonMap("name",
            Arrays.asList(Collections.singleton("=="), null));
        test_query_resource_result(properties, condition9, 0);

        Map<String, Object> condition10 = Collections.singletonMap("name",
            Arrays.asList(Collections.singleton("!="), null));
        test_query_resource_result(properties, condition10, 1);

        Map<String, Object> condition11 = Collections.singletonMap("tenantName",
            Arrays.asList(Collections.singleton("~~"), "Store"));
        test_query_resource_result(properties, condition11, 1);

        Map<String, Object> condition12 = Collections.singletonMap("tenantName",
            Arrays.asList(Collections.singleton("~~"), "stem_vs"));
        test_query_resource_result(properties, condition12, 1);

        Map<String, Object> condition13 = Collections.singletonMap("tenantName",
            Arrays.asList(Collections.singleton("~~"), "-vs"));
        test_query_resource_result(properties, condition13, 0);

        Map<String, Object> condition14 = Collections.singletonMap("tenantName", null);
        test_query_resource_result(properties, condition14, 0);

        Map<String, Object> condition15 = Collections.singletonMap("xxx", null);
        test_query_resource_result(properties, condition15, 1);

        Map<String, Object> condition16 = Collections.singletonMap("xxx",
            Arrays.asList(Collections.singletonList("=="), null));
        test_query_resource_result(properties, condition16, 1);

        Map<String, Object> condition17 = Collections.singletonMap("xxx",
            Arrays.asList(Collections.singletonList("!="), null));
        test_query_resource_result(properties, condition17, 0);

        Map<String, Object> condition18 = Collections.singletonMap("xxx",
            Arrays.asList(Collections.singletonList("!="), "1"));
        test_query_resource_result(properties, condition18, 1);

        Map<String, Object> condition19 = Collections.singletonMap("yyy",
            Arrays.asList(Collections.singletonList("!="), "1"));
        test_query_resource_result(properties, condition19, 1);

        Map<String, Object> condition20 = Collections.singletonMap("isCluster",
            Arrays.asList(Collections.singletonList("=="), false));
        test_query_resource_result(properties, condition20, 0);
    }

    private void create_resource(ProtectedResource... resources) {
        mockCurrentUser();
        String[] ids = resourceService.create(resources);
        Assert.assertEquals(resources.length, ids.length);
    }

    /**
     * 用例名称：验证资源服务层删除资源<br/>
     * 前置条件：资源在数据库中已创建成功<br/>
     * check点：资源删除成功<br/>
     */
    @Test
    public void test_delete_resource_success() {
        test_create_environment();
        List<ProtectedEnvironmentExtendInfoPo> list = protectedEnvironmentExtendInfoMapper.selectList(null);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(RESOURCE_ID);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(
            ResourceConstants.CITATION + ResourceConstants.CITATION_SEPERATOR + list.get(0).getUuid() + "_host_"
                + System.currentTimeMillis(), list.get(0).getUuid());
        protectedResource.setExtendInfo(extendInfo);
        protectedResource.setSubType(ResourceSubTypeEnum.NAS_SHARE.getType());
        create_resource(protectedResource);

        int resourceCount0 = protectedResourceMapper.selectCount(null).intValue();
        Assert.assertNotEquals(0, resourceCount0);
        int resourceExtendInfoCount0 = protectedResourceExtendInfoMapper.selectCount(null).intValue();
        Assert.assertNotEquals(0, resourceExtendInfoCount0);

        List<ProtectedResourcePo> protectedResourcePos = protectedResourceMapper.selectList(null);
        Assert.assertEquals(RESOURCE_ID, protectedResourcePos.get(1).getUuid());

        LambdaQueryWrapper<ProtectedResourceExtendInfoPo> lambdaQueryWrapper = new LambdaQueryWrapper();
        lambdaQueryWrapper.eq(ProtectedResourceExtendInfoPo::getResourceId, protectedResourcePos.get(1).getUuid());
        protectedResourceExtendInfoMapper.delete(lambdaQueryWrapper);

        ProtectedResource resource = getProtectedResource();
        resourceService.delete(new String[] {resource.getUuid()});

        int resourceCount1 = protectedResourceMapper.selectCount(null).intValue();
        Assert.assertEquals(1, resourceCount1);
    }

    /**
     * 用例场景：测试受保护环境被其他资源依赖时，不能删除
     * 前置条件：受保护环境被其他资源依赖
     * 检查 点： 抛出正确错误提示
     */
    @Test
    public void test_should_throws_LegoCheckedException_when_env_is_not_depended_on() {
        String dependenciesResourceId = UUID.randomUUID().toString();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(RESOURCE_ID);
        protectedResource.setUserId("admin");
        protectedResource.setSubType("HDFS");
        protectedResource.setAuth(createAuthentication());
        protectedResource.setExtendInfoByKey("username", "admin");
        protectedResource.setExtendInfoByKey(
            ResourceConstants.CITATION + ResourceConstants.CITATION_SEPERATOR + dependenciesResourceId + "_host_"
                + System.currentTimeMillis(), dependenciesResourceId);
        create_resource(protectedResource);

        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> resourceService.delete(new String[] {RESOURCE_ID}));
        Assert.assertEquals("The resource(ID: " + RESOURCE_ID + ") is depended by other resources.",
            legoCheckedException.getMessage());
    }

    /**
     * 用例名称：验证子删除资源时，类型为register的资源或子资源不会被删除<br/>
     * 前置条件：无<br/>
     * check点：删除资源，类型为register的资源或子资源不会被删除，其余的会被删除<br/>
     */
    @Test
    public void delete_resource_can_not_delete_register() {
        mockCurrentUser();
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid("0");
        environment.setRootUuid("0");
        environment.setSourceType(ResourceConstants.SOURCE_TYPE_REGISTER);
        environment.setSubType(ResourceSubTypeEnum.NAS_SHARE.getType());
        ProtectedResource child01 = new ProtectedResource();
        child01.setUuid("01");
        child01.setParentUuid("0");
        child01.setSubType(ResourceSubTypeEnum.NAS_SHARE.getType());
        ProtectedResource child011 = new ProtectedResource();
        child011.setUuid("011");
        child011.setParentUuid("01");
        child011.setSourceType(ResourceConstants.SOURCE_TYPE_REGISTER);
        child011.setSubType(ResourceSubTypeEnum.NAS_SHARE.getType());
        ProtectedResource child0111 = new ProtectedResource();
        child0111.setUuid("0111");
        child0111.setParentUuid("011");
        child0111.setSubType(ResourceSubTypeEnum.NAS_SHARE.getType());
        resourceService.create(new ProtectedResource[] {environment, child01, child011, child0111});
        resourceService.delete(new ResourceDeleteParams(true, false, new String[] {"01"}));
        Set<String> afterDeleteIdSet = protectedResourceMapper.selectList(new QueryWrapper<>())
            .stream()
            .map(ProtectedResourcePo::getUuid)
            .collect(Collectors.toSet());
        Assert.assertTrue(afterDeleteIdSet.contains("011"));
        Assert.assertFalse(afterDeleteIdSet.contains("01"));
        Assert.assertFalse(afterDeleteIdSet.contains("0111"));
    }

    /**
     * 用例名称：验证子资源受保护的情况下父资源删除失败<br/>
     * 前置条件：父子资源准备就绪，子资源受保护<br/>
     * check点：父资源删除失败<br/>
     */
    @Test
    public void test_delete_resource_with_protected_sub_resource() {
        test_create_environment();
        ProtectedResource parent = getProtectedResource();
        String rootUuid = parent.getUuid();
        ProtectedResource child = new ProtectedResource();
        child.setUuid("resource-uuid");
        child.setUserId("admin");
        child.setRootUuid(rootUuid);
        child.setSubType("protected_resource_Res");
        create_resource(child);
        ProtectedObjectPo protectedObjectPo = new ProtectedObjectPo();
        protectedObjectPo.setUuid("resource-uuid");
        protectedObjectPo.setResourceId("resource-uuid");
        protectedObjectPo.setChainId("resource-uuid");
        protectedObjectPo.setType("some-type");
        protectedObjectPo.setName("resource-name");
        protectedObjectMapper.insert(protectedObjectPo);
        LegoCheckedException error = assertThrows(LegoCheckedException.class,
            () -> resourceService.delete(new String[] {rootUuid}));
        assertEquals(ProtectedResourceRepositoryImpl.RESOURCE_ALREADY_BIND_SLA, error.getErrorCode());
        assertTrue(error.getMessage().contains("Having resources are bound to SLAs."));
    }

    /**
     * 用例名称：验证带环境信息的资源创建<br/>
     * 前置条件：环境信息在数据库中创建成功<br/>
     * check点：资源创建成功，创建回调数据完整<br/>
     */
    @Test
    public void test_create_resource_with_environment() {
        PowerMockito.when(resourceProvider.applicable(any())).thenReturn(true);
        test_create_environment();
        ProtectedResource environment = getProtectedResource();
        String rootUuid = environment.getUuid();
        ProtectedResource resource1 = new ProtectedResource();
        resource1.setUuid("resource-uuid1");
        resource1.setUserId("admin");
        resource1.setRootUuid(rootUuid);
        resource1.setParentUuid(rootUuid);
        resource1.setSubType("protected_resource_Res");
        ProtectedResource resource2 = new ProtectedResource();
        resource2.setUuid("resource-uuid2");
        resource2.setUserId("admin");
        resource2.setRootUuid(rootUuid);
        resource2.setParentUuid(rootUuid);
        resource2.setSubType("protected_resource_Res");
        create_resource(resource1, resource2);
        ArgumentCaptor<ProtectedResource> protectedResourceArgumentCaptor = ArgumentCaptor.forClass(
            ProtectedResource.class);
        verify(resourceProvider, atLeastOnce()).beforeCreate(protectedResourceArgumentCaptor.capture());
        Assert.assertNotNull(protectedResourceArgumentCaptor.getValue());

        Map<String, Object> condition0 = Collections.singletonMap("environment",
            Collections.singletonMap("name", Arrays.asList(Collections.singleton("~~"), "-name")));
        test_query_resource_result(Collections.emptyMap(), condition0, 2);

        Map<String, Object> condition1 = Collections.singletonMap("environment",
            Collections.singletonMap("name", Arrays.asList(Collections.singleton("~~"), "+name")));
        test_query_resource_result(Collections.emptyMap(), condition1, 0);

        Set<String> uuidList1 = resourceService.queryRelatedResourceUuids(rootUuid, new String[0]);
        Assert.assertEquals(3, uuidList1.size());

        Set<String> uuidList2 = resourceService.queryRelatedResourceUuids(rootUuid, new String[] {"resource-uuid2"});
        Assert.assertEquals(2, uuidList2.size());

        PageListResponse<ProtectedResource> response = resourceService.query(0, 1,
            Collections.singletonMap("uuid", "resource-uuid2"));
        List<ProtectedResource> resources = response.getRecords();
        Assert.assertEquals(1, resources.size());
        checkResource(resources);
    }

    private void checkResource(List<ProtectedResource> resources) {
        ProtectedResource resource = resources.get(0);
        ProtectedEnvironment environment = resource.getEnvironment();
        Assert.assertNotNull(environment);
        Assert.assertEquals("$password$", environment.getPassword());
    }

    @Test
    public void test_update_authentication() {
        mockKmcRestApi();

        test_create_resource();
        ProtectedResource resource0 = getProtectedResource();
        Assert.assertNull(resource0.getAuth());

        Authentication authentication0 = new Authentication();
        authentication0.setAuthType(1);
        authentication0.setAuthKey("key");
        authentication0.setExtendInfo(Collections.singletonMap("k", "v"));
        resourceService.update(
            new ProtectedResource[] {createResourceUpdateData(resource0.getUuid(), authentication0)});

        ProtectedResource resource1 = getProtectedResource();
        Authentication authentication1 = resource1.getAuth();
        Assert.assertNotNull(authentication1);
        Assert.assertEquals("key", authentication1.getAuthKey());
        Assert.assertEquals(1, authentication1.getAuthType());
        Map<String, String> extendInfo = authentication1.getExtendInfo();
        Assert.assertNotNull(extendInfo);
        Assert.assertEquals("$v$", extendInfo.get("k"));

        Authentication authentication2 = new Authentication();
        authentication2.setAuthType(2);
        authentication2.setAuthPwd("pwd");
        authentication2.setExtendInfo(Collections.singletonMap("x", "y"));
        resourceService.update(
            new ProtectedResource[] {createResourceUpdateData(resource1.getUuid(), authentication2)});

        ProtectedResource resource2 = getProtectedResource();
        Authentication authentication3 = resource2.getAuth();
        Assert.assertEquals("key", authentication3.getAuthKey());
        Assert.assertEquals("$pwd$", authentication3.getAuthPwd());
        Assert.assertNotNull(authentication3);
        Assert.assertEquals(2, authentication3.getAuthType());
        Map<String, String> extendInfo1 = authentication3.getExtendInfo();
        Assert.assertNotNull(extendInfo1);
        Assert.assertEquals("$$v$$", extendInfo1.get("k"));
        Assert.assertEquals("$y$", extendInfo1.get("x"));
    }

    private ProtectedResource createResourceUpdateData(String uuid, Authentication authentication) {
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid(uuid);
        resource.setAuth(authentication);
        return resource;
    }

    @Test
    public void test_upsert() {
        mockKmcRestApi();

        test_create_resource();
        ProtectedResource resource0 = getProtectedResource();
        ProtectedResource resource1 = new ProtectedResource();
        resource1.setUuid(resource0.getUuid());
        resource1.setName("new-resource-name");
        resource1.setExtendInfoByKey("k", "v");

        ProtectedResource resource2 = new ProtectedResource();
        resource2.setName("xxx");
        resource2.setExtendInfoByKey("xxx", "xxx");
        resource2.setUserId("admin");
        resource2.setSubType("protected_resource_Res");

        ResourceUpsertRes upsertRes = resourceService.upsert(new ProtectedResource[] {resource1, resource2});
        String[] uuidList = upsertRes.getIncreaseResourceUuids();
        Assert.assertNotNull(uuidList);
        Assert.assertEquals(1, uuidList.length);

        List<ProtectedResource> resources = resourceService.query(0, 10, null).getRecords();
        Assert.assertEquals(2, resources.size());
    }

    /**
     * 用例场景：测试upsert插入时限制数量 <br/>
     * 前置条件：数据库已有1个资源，最大资源数量为2，新增2个资源 <br/>
     * 检查点: 是否只新增了1个资源
     */
    @Test
    public void test_upsert_limit_resource_count() {
        mockKmcRestApi();
        // mock类成员变量maxResourceNum最大值为2
        ReflectionTestUtils.setField(resourceService, "maxResourceNum", 2);

        test_create_resource();
        ProtectedResource resource1 = new ProtectedResource();
        resource1.setName("new-resource-name");
        resource1.setUserId("admin");
        resource1.setSubType("protected_resource_Res");

        ProtectedResource resource2 = new ProtectedResource();
        resource2.setName("xxx");
        resource2.setExtendInfoByKey("xxx", "xxx");
        resource2.setUserId("admin");
        resource2.setSubType("protected_resource_Res");

        ResourceUpsertRes upsertRes = resourceService.upsert(new ProtectedResource[] {resource1, resource2});
        String[] uuidList = upsertRes.getIncreaseResourceUuids();
        Assert.assertNotNull(uuidList);
        Assert.assertTrue(upsertRes.isOverLimit());
        Assert.assertEquals(1, uuidList.length);

        List<ProtectedResource> resources = resourceService.query(0, 10, null).getRecords();
        Assert.assertEquals(2, resources.size());
        ReflectionTestUtils.setField(resourceService, "maxResourceNum", 20000);
    }

    /**
     * 创建资源，带依赖的情况
     */
    @Test
    public void create_resource_with_dependency_success() {
        mockCurrentUser();
        createHost();
        createEnv();
        List<ProtectedResource> records = resourceService.query(0, 10, null).getRecords();
        Assert.assertEquals(records.size(), 7);
        // 测试用例保证name不同
        Map<String, ProtectedResource> nameMap = records.stream()
            .collect(Collectors.toMap(ResourceBase::getName, e -> e));
        Map<String, ProtectedResource> uuidMap = records.stream()
            .collect(Collectors.toMap(ResourceBase::getUuid, e -> e));
        // 验证parent
        Assert.assertEquals(nameMap.get("node-01").getParentUuid(), nameMap.get("cluster-instance-01").getUuid());
        // 验证root
        Assert.assertEquals(nameMap.get("node-01").getRootUuid(), nameMap.get("cluster01").getUuid());
        // 验证citation
        Map<String, String> extendInfo = nameMap.get("node-01").getExtendInfo();
        extendInfo.forEach((k, v) -> {
            if (k.startsWith(ResourceConstants.CITATION)) {
                Assert.assertEquals(uuidMap.get(v).getName(), "cluster-instance-01");
            }
        });
        // 验证查询的dependency
        Optional<ProtectedResource> cluster01 = resourceService.getResourceById(nameMap.get("cluster01").getRootUuid());
        Assert.assertTrue(cluster01.get().getDependencies().size() > 0);
    }

    @Test
    public void update_resource_with_dependency_success() {
        mockCurrentUser();
        createHost();
        ProtectedEnvironment environment = createEnv();

        ProtectedResource clusterInstance = environment.getDependencies().get(ResourceConstants.CHILDREN).get(0);
        List<ProtectedResource> hostInstanceList = clusterInstance.getDependencies().get(ResourceConstants.CHILDREN);
        ProtectedResource hostInstanceUpdate = hostInstanceList.get(0);
        ProtectedResource hostInstanceDelete = hostInstanceList.get(1);
        hostInstanceUpdate.setName("host-instance-update");
        clusterInstance.getDependencies().remove(ResourceConstants.CHILDREN);
        clusterInstance.getDependencies()
            .put(ResourceConstants.CHILDREN, Collections.singletonList(hostInstanceUpdate));
        clusterInstance.getDependencies()
            .put("-" + ResourceConstants.CHILDREN, Collections.singletonList(hostInstanceDelete));
        resourceService.update(new ProtectedResource[] {environment});
        List<ProtectedResource> records = resourceService.query(0, 10, null).getRecords();
        Map<String, ProtectedResource> nameMap = records.stream()
            .collect(Collectors.toMap(ResourceBase::getName, e -> e));
        Map<String, ProtectedResource> uuidMap = records.stream()
            .collect(Collectors.toMap(ResourceBase::getUuid, e -> e));
        Assert.assertEquals("host-instance-update", uuidMap.get(hostInstanceUpdate.getUuid()).getName());
        Assert.assertEquals(uuidMap.get(hostInstanceDelete.getUuid()).getExtendInfo().size(), 0);
    }

    private ProtectedEnvironment createEnv() {
        return createEnv(getProtectedEnvironmentFromFile("resource/testAddEnv.json"));
    }

    private ProtectedEnvironment createEnv(ProtectedEnvironment environment) {
        resourceService.create(new ProtectedResource[] {environment});
        return environment;
    }

    private ProtectedEnvironment getProtectedEnvironmentFromFile(String classPath) {
        ResourcePatternResolver resolver = new PathMatchingResourcePatternResolver();
        org.springframework.core.io.Resource resource = resolver.getResource("classpath:" + classPath);
        JsonNode jsonNode = null;
        try {
            jsonNode = JsonUtil.read(resource.getInputStream());
        } catch (IOException e) {
            Assert.assertThrows("read json error.", e.getClass(), () -> {
            });
            return null;
        }
        ProtectedEnvironment environment = JsonUtil.read(jsonNode.toString(), ProtectedEnvironment.class);
        environment.setUuid(environment.getName());
        environment.setRootUuid(environment.getUuid());
        environment.setSubType(ResourceSubTypeEnum.NAS_SHARE.getType());
        return environment;
    }

    private void createHost() {
        String host1 = "{\n" + "    \"uuid\": \"host-uuid-01\",\n" + "    \"name\": \"host-01\",\n"
            + "    \"userId\":\"admin\"\n" + "}";
        String host2 = "{\n" + "    \"uuid\": \"host-uuid-02\",\n" + "    \"name\": \"host-02\",\n"
            + "    \"userId\":\"admin\"\n" + "}";
        String host3 = "{\n" + "    \"uuid\": \"host-uuid-03\",\n" + "    \"name\": \"host-03\",\n"
            + "    \"userId\":\"admin\"\n" + "}";
        ProtectedResource host1ProtectedResource = JsonUtil.read(host1, ProtectedResource.class);
        ProtectedResource host2ProtectedResource = JsonUtil.read(host2, ProtectedResource.class);
        ProtectedResource host3ProtectedResource = JsonUtil.read(host3, ProtectedResource.class);
        host1ProtectedResource.setSubType(ResourceSubTypeEnum.NAS_SHARE.getType());
        host2ProtectedResource.setSubType(ResourceSubTypeEnum.NAS_SHARE.getType());
        host3ProtectedResource.setSubType(ResourceSubTypeEnum.NAS_SHARE.getType());
        String[] hostUUids = resourceService.create(
            new ProtectedResource[] {host1ProtectedResource, host2ProtectedResource, host3ProtectedResource});
        Assert.assertEquals(hostUUids.length, 3);
    }

    /**
     * 用例名称：验证带dependency条件的查询<br/>
     * 前置条件：环境信息在数据库中创建成功<br/>
     * check点：带dependency的context查询结果和之前参数查询结果保持一致<br/>
     */
    @Test
    public void query_resource_dependency_result_same_with_origin() {
        mockCurrentUser();
        createHost();
        ProtectedEnvironment environment = createEnv();
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("uuid", Arrays.asList("host-uuid-01", "host-uuid-02", "host-uuid-03", environment.getUuid()));
        List<ProtectedResource> resourcesByQueryOrigin = resourceService.query(0, 10, conditions).getRecords();
        ResourceQueryParams context = new ResourceQueryParams();
        context.setConditions(conditions);
        context.setShouldQueryDependency(true);
        context.setSize(2);
        List<ProtectedResource> resourcesByQueryContext = resourceService.query(context).getRecords();
        Assert.assertEquals(resourcesByQueryOrigin.size(), resourcesByQueryContext.size());
        Assert.assertTrue(resourcesByQueryOrigin.size() > 0);
        for (int i = 0; i < resourcesByQueryOrigin.size(); i++) {
            Assert.assertEquals(resourcesByQueryOrigin.get(i).getName(), resourcesByQueryContext.get(i).getName());
        }
    }

    @Test
    public void test_query_resource_by_userid_success() {
        mockCurrentUser();
        createHost();
        ProtectedEnvironment environment = createEnv();
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("uuid", Arrays.asList("host-uuid-01", "host-uuid-02", "host-uuid-03", environment.getUuid()));
        List<ProtectedResource> resources = resourceService.queryResourcesByUserId("admin");
        Assert.assertNotNull(resources);
    }

    /**
     * 用例名称：验证带dependency条件的查询<br/>
     * 前置条件：环境信息在数据库中创建成功<br/>
     * check点：带dependency的context查询结果成功查询出dependency<br/>
     */
    @Test
    public void query_resource_dependency_result_with_dependency() {
        mockCurrentUser();
        createHost();
        ProtectedEnvironment environment = createEnv();
        ResourceQueryParams context = new ResourceQueryParams();
        context.setConditions(Collections.singletonMap("uuid", environment.getUuid()));
        context.setShouldQueryDependency(true);
        List<ProtectedResource> resources = resourceService.query(context).getRecords();
        Assert.assertTrue(resources.get(0).getDependencies().size() > 0);
        Assert.assertTrue(resources.get(0).getDependencies().get("children").size() > 0);
    }

    /**
     * 用例名称：验证 根据扩展信息字段分组查询<br/>
     * 前置条件：包含扩展信息的资源在数据库中创建成功<br/>
     * check点：成功查询出数据<br/>
     */
    @Test
    public void test_group_query() {
        mockKmcRestApi();

        for (int i = 0; i < 25; i++) {
            ProtectedEnvironment environment = new ProtectedEnvironment();
            this.setEnvCommonPropeties(environment);
            environment.setExtendInfoByKey("key", "value" + i);
            create_resource(new ProtectedResource[] {environment});
        }

        PageListResponse<ProtectedResource> response = resourceService.query(0, 30, null);
        List<ProtectedResource> resources = response.getRecords();
        Assert.assertEquals(25, resources.size());

        VstoreResourceQueryParam param = VstoreResourceQueryParam.builder()
            .page(0)
            .size(10)
            .isSearchProtectObject(true)
            .key("key")
            .order("+")
            .build();
        PageListResponse<ProtectedResourceGroupResult> groupResultPageListResponse
            = resourceService.groupQueryByExtendInfo(param);

        Assert.assertEquals(10, groupResultPageListResponse.getRecords().size());
        Assert.assertEquals(25, groupResultPageListResponse.getTotalCount());
    }

    private void setEnvCommonPropeties(ProtectedEnvironment environment) {
        environment.setUserId("admin");
        environment.setEndpoint("localhost");
        environment.setUsername("admin");
        environment.setPassword("password");
        environment.setName("environment-name");
        environment.setOsType("linux");
        environment.setSubType("protected_environment_Env");
        environment.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
    }

    /**
     * 用例名称：测试 查询资源的基本属性，不再额外查询资源的dependency依赖信息 sdk暴露接口<br/>
     * 前置条件：带有dependency的资源 在数据库中创建成功<br/>
     * check点：查询结果dependency信息为空<br/>
     */
    @Test
    public void test_get_resource_without_dependency() {
        mockCurrentUser();
        createHost();
        ProtectedEnvironment environment = createEnvWithEnv();
        Assert.assertNotNull(environment);

        Optional<ProtectedResource> resourcesOp = resourceService.getBasicResourceById(true, environment.getUuid());
        Assert.assertNull(resourcesOp.get().getEnvironment());
        Assert.assertNull(resourcesOp.get().getDependencies());
    }

    /**
     * 用例名称：测试 查询资源的基本属性，不再额外查询资源的dependency依赖信息 sdk暴露接口<br/>
     * 前置条件：带有dependency的资源 在数据库中创建成功<br/>
     * check点：查询结果dependency信息为空, 环境信息不为空<br/>
     */
    @Test
    public void get_resource_with_env_without_dependency() {
        mockCurrentUser();
        createHost();
        ProtectedEnvironment environment = createEnvWithEnv();
        Assert.assertNotNull(environment);

        Optional<ProtectedResource> resourcesOp = resourceService.getResourceById(environment.getUuid());
        ProtectedResource protectedResource = resourcesOp.get().getDependencies().get("children").get(0);
        Optional<ProtectedResource> basicResourceById = resourceService.getBasicResourceById(true, true,
            protectedResource.getUuid());
        Assert.assertNotNull(basicResourceById.get().getEnvironment());
        Assert.assertNull(basicResourceById.get().getDependencies());
    }

    private ProtectedEnvironment createEnvWithEnv() {
        Lock lockMock = Mockito.mock(Lock.class);
        Mockito.when(lockMock.tryLock(Mockito.anyLong(), Mockito.any(TimeUnit.class))).thenReturn(true);
        Mockito.when(lockService.createDistributeLock(Mockito.any())).thenReturn(lockMock);
        ResourcePatternResolver resolver = new PathMatchingResourcePatternResolver();
        org.springframework.core.io.Resource resource = resolver.getResource("classpath:resource/testAddEnv.json");
        JsonNode jsonNode = null;
        try {
            jsonNode = JsonUtil.read(resource.getInputStream());
        } catch (IOException e) {
            Assert.assertThrows("read json error.", e.getClass(), () -> {
            });
            return null;
        }
        ProtectedEnvironment environment = JsonUtil.read(jsonNode.toString(), ProtectedEnvironment.class);
        environment.setUuid(environment.getName());
        environment.setRootUuid(environment.getUuid());
        environment.setEnvironment(environment);
        environment.setSubType(ResourceSubTypeEnum.NAS_SHARE.getType());
        resourceService.create(new ProtectedResource[] {environment});
        return environment;
    }

    /**
     * 用例名称：测试 补充资源的dependency信息 sdk暴露接口<br/>
     * 前置条件：带有dependency的资源 在数据库中创建成功<br/>
     * check点：带dependency的context查询结果成功查询出dependency<br/>
     */
    @Test
    public void test_get_resource_supply_dependency() {
        mockCurrentUser();
        createHost();
        ProtectedEnvironment environment = createEnv();

        Optional<ProtectedResource> resourcesOp = resourceService.queryDependency(environment);
        resourceService.setResourceDependency(environment);
        Assert.assertTrue(resourcesOp.get().getDependencies().size() > 0);
        Assert.assertTrue(resourcesOp.get().getDependencies().get("children").size() > 0);
    }

    /**
     * 用例名称：测试 补充资源的dependency信息 <br/>
     * 前置条件：带有dependency的资源 在数据库中创建成功<br/>
     * check点：带dependency的context查询结果成功查询出dependency<br/>
     */
    @Test
    public void test_get_resource_supply_dependency_spe() {
        ProtectedResourcePo parent = new ProtectedResourcePo();
        parent.setUuid(UUID.randomUUID().toString());
        parent.setParentUuid(null);
        parent.setCreatedTime(new Timestamp(System.currentTimeMillis()));
        parent.setSubType(ResourceSubTypeEnum.NAS_SHARE.getType());

        ProtectedResourcePo children = new ProtectedResourcePo();
        children.setUuid(UUID.randomUUID().toString());
        children.setParentUuid(parent.getUuid());
        children.setCreatedTime(new Timestamp(System.currentTimeMillis()));
        children.setSubType(ResourceSubTypeEnum.NAS_SHARE.getType());

        ProtectedResourcePo dependency = new ProtectedResourcePo();
        dependency.setUuid(UUID.randomUUID().toString());
        dependency.setParentUuid(dependency.getUuid());
        dependency.setCreatedTime(new Timestamp(System.currentTimeMillis()));
        dependency.setSubType(ResourceSubTypeEnum.NAS_SHARE.getType());

        protectedResourceMapper.insert(parent);
        protectedResourceMapper.insert(children);
        protectedResourceMapper.insert(dependency);

        ProtectedResourceExtendInfoPo protectedResourceExtendInfoPo = new ProtectedResourceExtendInfoPo();
        protectedResourceExtendInfoPo.setUuid(UUID.randomUUID().toString());
        protectedResourceExtendInfoPo.setKey("$citations_hosts_24412121");
        protectedResourceExtendInfoPo.setResourceId(dependency.getUuid());
        protectedResourceExtendInfoPo.setValue(parent.getUuid());

        protectedResourceExtendInfoMapper.insert(protectedResourceExtendInfoPo);

        List<String> uuids = new ArrayList<>();
        uuids.add(parent.getUuid());
        PageListResponse<ProtectedResource> response = resourceService.query(0, 10,
            Collections.singletonMap("uuid", uuids));
        ProtectedResource db = response.getRecords().get(0);
        Assert.assertNull(db.getDependencies());
        // 填充
        resourceService.setResourceDependency(db);
        Assert.assertNotNull(db.getDependencies());
    }

    /**
     * 用例名称：有相同授权的资源能创建成功<br/>
     * 前置条件：无<br/>
     * check点：有相同授权的资源能创建成功<br/>
     */
    @Test
    public void create_success_when_authorize_same() {
        mockCurrentUser();
        mockLock();
        ProtectedResource host1 = new ProtectedResource();
        host1.setUserId("admin01");
        host1.setAuthorizedUser("tom");
        host1.setUuid("host-uuid-01");
        ProtectedResource host2 = new ProtectedResource();
        host2.setUserId("admin01");
        host2.setAuthorizedUser("tom");
        host2.setUuid("host-uuid-02");
        resourceService.create(new ProtectedResource[] {host1, host2});
        ProtectedResource env = new ProtectedResource();
        env.setUuid("envId");
        env.setRootUuid(env.getUuid());
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put("agents", Arrays.asList(host1, host2));
        env.setDependencies(dependencies);
        resourceService.create(new ProtectedResource[] {env});
        ResourceQueryParams params = new ResourceQueryParams();
        params.setShouldIgnoreOwner(true);
        List<ProtectedResource> records = resourceService.query(params).getRecords();
        records.forEach(elem -> {
            Assert.assertEquals(elem.getUserId(), "admin01");
        });
    }

    /**
     * 用例名称：没有相同授权的资源创建失败<br/>
     * 前置条件：无<br/>
     * check点：没有相同授权的资源创建失败<br/>
     */
    @Test
    public void create_fail_when_authorize_not_same() {
        mockCurrentUser();
        mockLock();
        ProtectedResource host1 = new ProtectedResource();
        host1.setUserId("admin01");
        host1.setAuthorizedUser("tom");
        host1.setUuid("host-uuid-01");
        ProtectedResource host2 = new ProtectedResource();
        host2.setUserId("admin02");
        host2.setAuthorizedUser("tom");
        host2.setUuid("host-uuid-02");
        resourceService.create(new ProtectedResource[] {host1, host2});
        ProtectedResource env = new ProtectedResource();
        env.setUuid("envId");
        env.setRootUuid(env.getUuid());
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put("agents", Arrays.asList(host1, host2));
        env.setDependencies(dependencies);

        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> resourceService.create(new ProtectedResource[] {env}));
        Assert.assertEquals(legoCheckedException.getErrorCode(), CommonErrorCode.RESOURCE_AUTHORIZE_INCONSISTENT);
    }

    private void mockLock() {
        Lock lockMock = Mockito.mock(Lock.class);
        Mockito.when(lockMock.tryLock(Mockito.anyLong(), Mockito.any(TimeUnit.class))).thenReturn(true);
        Mockito.when(lockService.createDistributeLock(Mockito.any())).thenReturn(lockMock);
    }

    /**
     * 用例名称：没有相同授权的资源创建失败<br/>
     * 前置条件：无<br/>
     * check点：没有相同授权的资源创建失败<br/>
     */
    @Test
    public void desensitize_resource_success() {
        ProtectedResource host1 = new ProtectedResource();
        host1.setUuid("uuid01");
        Authentication auth1 = new Authentication();
        auth1.setAuthKey("authKey1");
        auth1.setAuthPwd("authPwd1");
        auth1.setExtendInfo(new HashMap<>());
        auth1.getExtendInfo().put("extKey1", "extValue1");
        host1.setAuth(auth1);

        ProtectedResource host2 = new ProtectedResource();
        host2.setUuid("uuid02");
        Authentication auth2 = new Authentication();
        auth2.setAuthKey("authKey2");
        auth2.setAuthPwd("authPwd2");
        auth2.setExtendInfo(new HashMap<>());
        auth2.getExtendInfo().put("extKey2", "extValue2");
        host2.setAuth(auth2);

        host1.setDependencies(new HashMap<>());
        host1.getDependencies().put("agents", Collections.singletonList(host2));

        resourceService.desensitize(host1);

        Assert.assertNull(host1.getAuth().getAuthPwd());
        Assert.assertNull(host1.getAuth().getExtendInfo());
        Assert.assertNull(host2.getAuth().getAuthPwd());
        Assert.assertNull(host2.getAuth().getExtendInfo());
    }

    /**
     * 用例名称：验证 删除受保护资源时，发出告警<br/>
     * 前置条件：无。<br/>
     * check点：成功告警<br/>
     */
    @Test
    public void test_alarm_when_delete_protected_resource() {
        create_resource();

        ProtectedResource resource0 = getProtectedResource();
        ProtectedObject protectedObject0 = resource0.getProtectedObject();
        Assert.assertNull(protectedObject0);

        String uuid = resource0.getUuid();
        ProtectedObjectPo protectedObjectPo = new ProtectedObjectPo();
        protectedObjectPo.setUuid(uuid);
        protectedObjectPo.setName("protectedResource");
        protectedObjectPo.setType("type");
        protectedObjectPo.setResourceId(uuid);
        protectedObjectPo.setChainId(uuid);
        protectedObjectMapper.insert(protectedObjectPo);

        Set<String> redundantResourceUuids = new HashSet<>();
        redundantResourceUuids.add(uuid);

        PowerMockito.doNothing().when(commonAlarmService).generateAlarm(any());

        ReflectionTestUtils.invokeMethod(resourceAlarmService, "alarmDeleteProtectedResource", redundantResourceUuids);
        verify(commonAlarmService, times(1)).generateAlarm(any());
    }

    /**
     * 用例名称：验证 根据uuid集合查询受保护的资源<br/>
     * 前置条件：无。<br/>
     * check点：成功查询<br/>
     */
    @Test
    public void test_query_protected_resource() {
        create_resource();

        ProtectedResource resource0 = getProtectedResource();
        ProtectedObject protectedObject0 = resource0.getProtectedObject();
        Assert.assertNull(protectedObject0);

        String uuid = resource0.getUuid();
        ProtectedObjectPo protectedObjectPo = new ProtectedObjectPo();
        protectedObjectPo.setUuid(uuid);
        protectedObjectPo.setName("protectedResource");
        protectedObjectPo.setType("type");
        protectedObjectPo.setResourceId(uuid);
        protectedObjectPo.setChainId(uuid);
        protectedObjectMapper.insert(protectedObjectPo);

        List<String> uuids = new ArrayList<>();
        uuids.add(uuid);

        List<ProtectedObjectPo> protectedObjectPos = protectedResourceRepository.queryProtectedObject(uuids);
        Assert.assertEquals(uuid, protectedObjectPos.get(0).getUuid());
    }

    /**
     * 用例名称：创建子资源时权限与父资源一致<br/>
     * 前置条件：无。<br/>
     * check点：子资源权限设置成功<br/>
     */
    @Test
    public void sub_resource_should_has_same_authority_with_parent() {
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setRoles(Collections.emptyList());
        userBo.setId("userId");
        userBo.setName("userName");
        PowerMockito.when(sessionService.getCurrentUser()).thenReturn(userBo);
        ProtectedResource parentResource = new ProtectedResource();
        parentResource.setUuid("parentId");
        parentResource.setUserId("userId");
        parentResource.setAuthorizedUser("userName");
        parentResource.setSubType(ResourceSubTypeEnum.NAS_SHARE.getType());
        resourceService.create(new ProtectedResource[] {parentResource});

        ProtectedResource subResource = new ProtectedResource();
        subResource.setUuid("subId");
        subResource.setParentUuid(parentResource.getUuid());
        subResource.setSubType(ResourceSubTypeEnum.NAS_SHARE.getType());
        resourceService.create(new ProtectedResource[] {subResource});

        List<ProtectedResourcePo> protectedResourcePos = protectedResourceMapper.selectList(
            new QueryWrapper<ProtectedResourcePo>().lambda()
                .in(ProtectedResourcePo::getUuid, Arrays.asList(parentResource.getUuid(), subResource.getUuid())));
        Assert.assertEquals(protectedResourcePos.size(), 2);
        for (ProtectedResourcePo protectedResourcePo : protectedResourcePos) {
            Assert.assertEquals(protectedResourcePo.getUserId(), "userId");
            Assert.assertEquals(protectedResourcePo.getAuthorizedUser(), "userName");
        }
    }

    /**
     * 用例名称：检查主机IP是否受信<br/>
     * 前置条件：无。<br/>
     * check点：内置代理 或 trustworthiness为true: 受信。 其他： 不受信<br/>
     */
    @Test
    public void host_trust() {
        SystemSwitchDto systemSwitchDto = new SystemSwitchDto();
        systemSwitchDto.setStatus(SwitchStatusEnum.ON);
        Mockito.when(systemSwitchInternalService.queryByName(Mockito.any())).thenReturn(systemSwitchDto);

        // 内置代理
        Map<String, String> internalAgents = new HashMap<>();
        internalAgents.put(ResourceExtendInfoKeyConstants.EXT_INFO_SCENARIO, AgentTypeEnum.INTERNAL_AGENT.getValue());
        // 受信
        Map<String, String> trustAgents = new HashMap<>();
        trustAgents.put(ResourceExtendInfoKeyConstants.TRUSTWORTHINESS, "true");
        // 不受信
        Map<String, String> unTrustAgents = new HashMap<>();

        ProtectedEnvironment env1 = new ProtectedEnvironment();
        env1.setUuid("env1");
        env1.setEndpoint("1.1.1.1");
        env1.setType("Host");
        env1.setExtendInfo(internalAgents);
        env1.setSubType(ResourceSubTypeEnum.NAS_SHARE.getType());

        ProtectedEnvironment env2 = new ProtectedEnvironment();
        env2.setUuid("env2");
        env2.setEndpoint("1.1.1.2");
        env2.setType("Host");
        env2.setExtendInfo(trustAgents);
        env2.setSubType(ResourceSubTypeEnum.NAS_SHARE.getType());

        ProtectedEnvironment env3 = new ProtectedEnvironment();
        env3.setUuid("env3");
        env3.setName("env-name3");
        env3.setEndpoint("1.1.1.3");
        env3.setType("Host");
        env3.setExtendInfo(unTrustAgents);
        env3.setSubType(ResourceSubTypeEnum.NAS_SHARE.getType());

        create_resource(env1, env2, env3);

        Assert.assertTrue(protectedResourceServices.checkHostIfBeTrustedByEndpoint("1.1.1.1"));
        Assert.assertTrue(protectedResourceServices.checkHostIfBeTrustedByEndpoint("1.1.1.2"));
        Assert.assertFalse(protectedResourceServices.checkHostIfBeTrustedByEndpoint("1.1.1.3"));
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> Assert.assertFalse(protectedResourceServices.checkHostIfBeTrustedByEndpoint("1.1.1.3", true)));
        Assert.assertEquals(exception.getErrorCode(), CommonErrorCode.RESOURCE_NOT_TRUST);
        Assert.assertTrue(Arrays.equals(exception.getParameters(), new String[] {"env-name3", "1.1.1.3"}));
    }

    /**
     * 用例名称：查询agent的fc配置信息
     * 前置条件：数据库连接正常
     * check点：成功查询出agent的fc配置信息
     */
    @Test
    public void should_return_agent_fc_config_successfully() {
        this.create_resource();
        ProtectedResource agentResource = getProtectedResource();
        Assert.assertEquals("1", agentResource.getExtendInfo().get(ResourceConstants.IS_ADD_LAN_FREE));

        List<String> resIds = new ArrayList<>();
        resIds.add(agentResource.getUuid());
        Map<String, String> agentMap = resourceService.getLanFreeConfig("subType", resIds);
        Assert.assertEquals(agentMap.get(agentResource.getUuid()), "true");
    }

    /**
     * 用例场景：成功过滤出开启了lanFree的配置的agent
     * 前置条件：资源的子类型支持配置lanFree
     * 检查点：过滤成功
     */
    @Test
    public void test_filter_is_lan_free_agent_success() {
        createResourceWithEsn();
        MemberModifier.stub(MemberMatcher.method(ProtectedResourceServiceImpl.class, "supportLanFree")).toReturn(true);
        List<String> resourceIds = new ArrayList<>();
        resourceIds.add("1");
        resourceIds.add("2");
        resourceIds.add("3");
        List<String> filterResourceIds = new ArrayList<>();
        resourceIds.add("2");
        MemberModifier.stub(MemberMatcher.method(ProtectedResourceServiceImpl.class, "filterIsLanFreeResourceIds"))
            .toReturn(filterResourceIds);
        List<String> result = protectedResourceServices.getRelationInLanFree("support", resourceIds);
        Assert.assertEquals(result.size(), 1);
        Assert.assertEquals(result.get(0), "123456789");
    }

    /**
     * 用例场景：过滤开启lanFree配置的agent
     * 前置条件：资源id为空
     * 检查点：返回空数组
     */
    @Test
    public void test_return_empty_list_when_empty_resource_ids() {
        List<String> resourceIds = new ArrayList<>();
        List<String> result = protectedResourceServices.getRelationInLanFree("notSupport", resourceIds);
        Assert.assertEquals(result.size(), 0);
    }

    private void createResourceWithEsn() {
        ProtectedResource resource = new ProtectedResource();
        resource.setName("resource-name");
        resource.setUuid("2");
        resource.setUserId("admin");
        resource.setSubType("protected_resource_Res");
        Map<String, String> properties = new HashMap<>();
        properties.put("username", "username");
        properties.put("password", "password");
        properties.put("tenantName", "System_vStore");
        properties.put("yyy", "2");
        properties.put("isAddLanFree", "1");
        properties.put("clusterEsn", "123456789");
        resource.setExtendInfo(properties);
        create_resource(resource);

        mockCurrentUser();
    }

    @Test
    public void test_queryAgentResourceList_with_user_id() {
        List<ProtectedResourcePo> mockList = new ArrayList<>();

        // 在线同时新增扩展信息表有agent上报数据
        ProtectedEnvironmentPo protectedEnvironment1 = new ProtectedEnvironmentPo();
        protectedEnvironment1.setLinkStatus("1");
        protectedEnvironment1.setProtectedAgentExtendPo(new ProtectedAgentExtendPo());
        protectedEnvironment1.setCreatedTime(new Timestamp(System.currentTimeMillis()));
        mockList.add(protectedEnvironment1);
        ProtectedResourceRepository mockRepository = PowerMockito.mock(ProtectedResourceRepository.class);
        Whitebox.setInternalState(resourceService, "repository", mockRepository);
        PowerMockito.when(mockRepository.queryAgentResourceList(any())).thenReturn(mockList);
        PowerMockito.when(mockRepository.queryAgentResourceCount(any())).thenReturn(1);
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setId("userId");
        List<TokenBo.RoleBo> roles = new ArrayList<>();
        TokenBo.RoleBo roleBo = new TokenBo.RoleBo();
        roleBo.setName("ROLE_SYS_ADMIN");
        roles.add(roleBo);
        userBo.setRoles(roles);
        Mockito.when(sessionService.getCurrentUser()).thenReturn(userBo);
        Map<String, Object> map = new HashMap<>();
        map.put(PAGE_NO, 0);
        map.put(PAGE_SIZE, 1);
        PageListResponse<ProtectedResource> response = resourceService.queryAgentResourceList(map);
        Assert.assertEquals(response.getRecords().size(), 1);
        map.put(PAGE_NO, 1);
        response = resourceService.queryAgentResourceList(map);
        Assert.assertEquals(response.getRecords().size(), 0);
        map.put(PAGE_NO, 2);
        response = resourceService.queryAgentResourceList(map);
        Whitebox.setInternalState(resourceService, "repository", protectedResourceRepository);
        Assert.assertEquals(response.getRecords().size(), 0);
    }
}
