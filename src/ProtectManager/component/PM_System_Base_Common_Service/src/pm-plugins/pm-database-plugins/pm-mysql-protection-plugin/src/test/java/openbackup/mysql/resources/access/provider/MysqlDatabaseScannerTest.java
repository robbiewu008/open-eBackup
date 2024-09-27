package openbackup.mysql.resources.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.client.sdk.api.framework.agent.AgentUnifiedRestApi;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppResource;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.utils.DatabaseScannerUtils;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.provider.config.MysqlAgentProxyConfig;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.mockito.internal.util.collections.Sets;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.test.util.ReflectionTestUtils;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Optional;

/**
 * 数据库扫描测试类
 *
 * @author xWX950025
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-18
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {FeignBuilder.class, DatabaseScannerUtils.class})
public class MysqlDatabaseScannerTest {
    private final static String instanceName = "InstanceName";

    private MysqlDatabaseScanner mysqlDatabaseScanner;

    private ResourceService resourceService;

    private MysqlInstanceProvider mysqlInstanceProvider;

    private ProtectObjectRestApi protectObjectRestApi = Mockito.mock(ProtectObjectRestApi.class);

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Before
    public void init() {
        this.resourceService = Mockito.mock(ResourceService.class);
        this.mysqlInstanceProvider = Mockito.mock(MysqlInstanceProvider.class);
        MysqlAgentProxyConfig agentProxyProperties = new MysqlAgentProxyConfig();
        agentProxyProperties.setHost("10.44.218.91");
        agentProxyProperties.setPort(3306);
        PowerMockito.mockStatic(FeignBuilder.class);
        this.mysqlDatabaseScanner = new MysqlDatabaseScanner( this.resourceService,
            mysqlInstanceProvider);
        ReflectionTestUtils.setField(this.mysqlDatabaseScanner, "protectObjectRestApi", this.protectObjectRestApi);
    }

    /**
     * 用例场景 扫描数据库失败
     * 前置条件：check失败
     * 检查点: 扫描失败
     */
    @Test
    public void scan_database_failed_when_check_error() throws IllegalAccessException {
        ProtectedEnvironment protectedEnvironment = getProtectedEnvironment();
        AgentUnifiedRestApi scanAgentRest = scanDatabases();
        PowerMockito.field(MysqlDatabaseScanner.class, "scanAgentRest").set(mysqlDatabaseScanner, scanAgentRest);
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("500");
        PowerMockito.when(scanAgentRest.check(any(), any(), any())).thenReturn(agentBaseDto);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any(), any())).thenReturn(getQueryResult());
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> mysqlDatabaseScanner.scan(protectedEnvironment));
        Assert.assertTrue(legoCheckedException.getMessage().contains("check error."));
    }

    /**
     * 用例场景 扫描数据库成功
     * 前置条件：实例注册正常
     * 检查点: 扫描成功
     */
    @Test
    public void scanDatabases_success() throws IllegalAccessException {
        ProtectedEnvironment protectedEnvironment = getProtectedEnvironment();
        AgentUnifiedRestApi scanAgentRest = scanDatabases();
        AgentDetailDto result = new AgentDetailDto();
        AppResource appResource = new AppResource();
        HashMap<String, String> ext = new HashMap<>();
        ext.put(DatabaseConstants.VERSION, "7.2.3");
        ext.put(MysqlConstants.DEPLOY_OPERATING_SYSTEM, "centos");
        appResource.setExtendInfo(ext);
        result.setResourceList(Collections.singletonList(appResource));
        PowerMockito.field(MysqlDatabaseScanner.class, "scanAgentRest").set(mysqlDatabaseScanner, scanAgentRest);
        PowerMockito.when(scanAgentRest.getDetail(any(), any(), any())).thenReturn(result);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any(), any())).thenReturn(getQueryResult());
        mockCheckSuccess(scanAgentRest);
        List<ProtectedResource> resources = mysqlDatabaseScanner.scan(protectedEnvironment);
        Assert.assertEquals(2, resources.size());
    }

    /**
     * 用例场景 扫描数据库为空
     * 前置条件：实例注册正常
     * 检查点: 实例下面没有数据库
     */
    @Test
    public void scanDatabases_with_empty_databases() throws IllegalAccessException {
        ProtectedEnvironment protectedEnvironment = getProtectedEnvironment();
        AgentUnifiedRestApi scanAgentRest = scanDatabases();
        PowerMockito.field(MysqlDatabaseScanner.class, "scanAgentRest").set(mysqlDatabaseScanner, scanAgentRest);
        PowerMockito.when(scanAgentRest.getDetail(any(), any(), any())).thenReturn(new AgentDetailDto());
        List<ProtectedResource> resources = mysqlDatabaseScanner.scan(protectedEnvironment);
        Assert.assertEquals(0, resources.size());
    }

    private AgentUnifiedRestApi scanDatabases() {
        AgentUnifiedRestApi scanAgentRest = PowerMockito.mock(AgentUnifiedRestApi.class);
        PageListResponse<ProtectedResource> queryRes = new PageListResponse<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setName(instanceName);
        protectedResource.setDependencies(Collections.emptyMap());
        HashMap<String, String> hashMap = new HashMap<>();
        protectedResource.setExtendInfo(hashMap);
        queryRes.setRecords(Collections.singletonList(protectedResource));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any(), any())).thenReturn(queryRes);
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(java.util.Optional.of(protectedResource));
        return scanAgentRest;
    }

    /**
     * 用例场景 扫描数据库为空
     * 前置条件：实例注册正常
     * 检查点: 扫描结果为空
     */
    @Test
    public void scanDatabases_empty() {
        ProtectedEnvironment protectedEnvironment = getProtectedEnvironment();
        PageListResponse<ProtectedResource> queryRes = new PageListResponse<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setDependencies(Collections.emptyMap());
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any(), any())).thenReturn(queryRes);
        List<ProtectedResource> resources = mysqlDatabaseScanner.scan(protectedEnvironment);
        Assert.assertEquals(0, resources.size());
    }

    private ProtectedEnvironment getProtectedEnvironment() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("111111");
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(3390);
        return protectedEnvironment;
    }

    /**
     * 用例场景 数据库扫描provider过滤
     * 前置条件：实例注册正常
     * 检查点: 成功过滤数据库扫描provider
     */
    @Test
    public void applicable_success() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setSubType(ResourceSubTypeEnum.U_BACKUP_AGENT.getType());
        Assert.assertTrue((mysqlDatabaseScanner.applicable(environment)));
    }

    private PageListResponse<ProtectedResource> getQueryResult() {
        PageListResponse<ProtectedResource> result = new PageListResponse<>();
        result.setTotalCount(1);
        ProtectedResource instance = new ProtectedResource();
        instance.setExtendInfo(new HashMap<>());
        result.setRecords(Collections.singletonList(instance));
        return result;
    }

    private void mockCheckSuccess(AgentUnifiedRestApi scanAgentRest) {
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        PowerMockito.when(scanAgentRest.check(any(), any(), any())).thenReturn(agentBaseDto);
    }

    @Test
    public void scanDatabase_for_eapp() {
        ProtectedEnvironment protectedEnvironment = getProtectedEnvironment();
        ProtectedResource instance = new ProtectedResource();
        instance.setExtendInfo(ImmutableMap.of(DatabaseConstants.CLUSTER_TYPE, MysqlConstants.EAPP));
        List<ProtectedResource> instances = Lists.newArrayList(instance);
        PowerMockito.mockStatic(DatabaseScannerUtils.class);
        PowerMockito.when(DatabaseScannerUtils.getInstancesByEnvironment(protectedEnvironment.getUuid(),
            ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType(), resourceService)).thenReturn(instances);
        List<ProtectedResource> resources = mysqlDatabaseScanner.scan(protectedEnvironment);
        Assert.assertEquals(1, instances.size());
        Assert.assertEquals(instances, resources);
    }

    @Test
    public void test_handle_destroy_database() {
        ProtectedResource instance = new ProtectedResource();
        instance.setUuid("test");
        PageListResponse<ProtectedResource> queryResult = getQueryResult();
        PowerMockito.when(resourceService.queryRelatedResourceUuids(anyString(), any()))
            .thenReturn(Sets.newSet("testUUID"));
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("testProtectId");
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(protectedResource));
        mysqlDatabaseScanner.handleDestroyDatabase(instance, queryResult.getRecords());
    }
}