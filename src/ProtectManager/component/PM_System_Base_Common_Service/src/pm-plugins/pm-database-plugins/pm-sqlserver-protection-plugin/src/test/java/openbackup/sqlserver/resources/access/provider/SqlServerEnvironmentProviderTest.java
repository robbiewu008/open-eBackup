package openbackup.sqlserver.resources.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.client.sdk.api.framework.agent.AgentUnifiedRestApi;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppResource;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * SQL Server环境扫描测试类
 *
 * @author xWX1016404
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-15
 */
public class SqlServerEnvironmentProviderTest {
    private SqlServerEnvironmentProvider sqlServerEnvironmentProvider;

    private ResourceService resourceService;

    private AgentUnifiedRestApi agentUnifiedRestApi;

    private SqlServerBaseService sqlServerBaseService;

    private static ProtectedEnvironment protectedEnvironment;

    private static Map<String, Object> conditions;

    static {
        protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("1");
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(1433);
        conditions = new HashMap<>();
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType());
        conditions.put(DatabaseConstants.HOST_ID, protectedEnvironment.getUuid());
    }

    @Before
    public void init() {
        this.resourceService = Mockito.mock(ResourceService.class);
        this.agentUnifiedRestApi = Mockito.mock(AgentUnifiedRestApi.class);
        this.agentUnifiedRestApi = Mockito.mock(AgentUnifiedRestApi.class);
        this.resourceService = Mockito.mock(ResourceService.class);
        sqlServerBaseService = Mockito.mock(SqlServerBaseService.class);
        this.sqlServerEnvironmentProvider = new SqlServerEnvironmentProvider(resourceService, sqlServerBaseService);
    }

    /**
     * 用例场景：SQL Server实例检查类provider过滤
     * 前置条件：资源类型为SQL Server单实例
     * 检查点：类过滤检查返回成功
     */
    @Test
    public void applicable_success() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setSubType(ResourceSubTypeEnum.U_BACKUP_AGENT.getType());
        Assert.assertTrue(sqlServerEnvironmentProvider.applicable(protectedEnvironment));
    }

    /**
     * 用例场景：SQL Server实例检查类provider过滤
     * 前置条件：资源类型为非SQL Server单实例
     * 检查点：类过滤检查返回失败
     */
    @Test
    public void when_resource_type_not_equal_then_applicable_return_false() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setSubType(ResourceSubTypeEnum.DWS_BACKUP_AGENT.getType());
        Assert.assertFalse(sqlServerEnvironmentProvider.applicable(protectedEnvironment));
    }

    /**
     * 用例场景：SQL Server资源扫描
     * 前置条件：已创建实例，且实例下有一个数据库资源
     * 检查点：SQL Server资源扫描成功，包含一个实例和一个数据库
     */
    @Test
    public void scan_database_success() {
        Mockito.when(agentUnifiedRestApi.getDetail(any(), any(), any())).thenReturn(getAgentDetailDto());
        PageListResponse<ProtectedResource> response = new PageListResponse<>(1,
            Collections.singletonList(getProtectedResource()));
        PageListResponse<ProtectedResource> resourcePageListResponse = new PageListResponse<>(1,
            Collections.singletonList(getProtectedResource()));
        Mockito.when(resourceService.getResourceById(anyString()))
            .thenReturn(Optional.of(getProtectedResource()));
        Mockito.when(resourceService.query(anyInt(), anyInt(), any(), anyString())).thenReturn(response)
            .thenReturn(resourcePageListResponse);
        Mockito.when(sqlServerBaseService.getDatabaseInfoByAgent(any(), any(), any(), any(), anyString()))
            .thenReturn(Collections.singletonList(getProtectedResource()));
        List<ProtectedResource> protectedResources = sqlServerEnvironmentProvider.scan(protectedEnvironment);
        Assert.assertEquals(2, protectedResources.size());
    }

    private AgentDetailDto getAgentDetailDto() {
        AgentDetailDto agentDetailDto = new AgentDetailDto();
        List<AppResource> resourceList = new ArrayList<>(1);
        AppResource appResource = new AppResource();
        Map<String, String> extendInfo = new HashMap<>(1);
        extendInfo.put(DatabaseConstants.DATABASE_ID, "1");
        appResource.setExtendInfo(extendInfo);
        resourceList.add(appResource);
        agentDetailDto.setResourceList(resourceList);
        return agentDetailDto;
    }

    /**
     * 用例场景：SQL Server资源扫描
     * 前置条件：未创建实例
     * 检查点：SQL Server资源扫描成功，返回空数组
     */
    @Test
    public void should_return_empty_list_when_scan_database_with_no_instance() {
        PageListResponse<ProtectedResource> response = new PageListResponse<>(0, new ArrayList<>(0));
        Mockito.when(resourceService.query(anyInt(), anyInt(), any(), anyString())).thenReturn(response);
        List<ProtectedResource> protectedResources = sqlServerEnvironmentProvider.scan(protectedEnvironment);
        Assert.assertEquals(0, protectedResources.size());
    }

    /**
     * 用例场景：SQL Server资源扫描
     * 前置条件：创建实例，但实例下无数据库
     * 检查点：SQL Server资源扫描成功，仅返回一个实例资源
     */
    @Test
    public void should_return_one_resource_when_scan_database_with_no_database() {
        AgentDetailDto agentDetailDto = new AgentDetailDto();
        agentDetailDto.setResourceList(new ArrayList<>(0));
        Mockito.when(agentUnifiedRestApi.getDetail(any(), any(), any())).thenReturn(agentDetailDto);
        PageListResponse<ProtectedResource> response = new PageListResponse<>(1,
            Collections.singletonList(getProtectedResource()));
        Mockito.when(resourceService.query(anyInt(), anyInt(), any(), anyString())).thenReturn(response)
            .thenReturn(new PageListResponse<>(0, new ArrayList<>()));
        List<ProtectedResource> protectedResources = sqlServerEnvironmentProvider.scan(protectedEnvironment);
        Assert.assertEquals(1, protectedResources.size());
    }

    /**
     * 用例场景：SQL Server资源扫描
     * 前置条件：创建实例，但实例下有数据库，但数据库没有返回id字段
     * 检查点：SQL Server资源扫描成功，仅返回一个实例资源
     */
    @Test
    public void should_return_one_resource_when_scan_database_with_no_database_id() {
        AgentDetailDto agentDetailDto = getAgentDetailDto();
        agentDetailDto.getResourceList().get(0).setExtendInfo(new HashMap<>(0));
        Mockito.when(agentUnifiedRestApi.getDetail(any(), any(), any())).thenReturn(agentDetailDto);
        PageListResponse<ProtectedResource> response = new PageListResponse<>(1,
            Collections.singletonList(getProtectedResource()));
        PageListResponse<ProtectedResource> resourcePageListResponse = new PageListResponse<>(1,
            Collections.singletonList(getProtectedResource()));
        Mockito.when(resourceService.query(anyInt(), anyInt(), any(), anyString())).thenReturn(response)
            .thenReturn(resourcePageListResponse);
        List<ProtectedResource> protectedResources = sqlServerEnvironmentProvider.scan(protectedEnvironment);
        Assert.assertEquals(1, protectedResources.size());
    }

    private ProtectedResource getProtectedResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType());
        Authentication auth = new Authentication();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.INSTANCE_PORT, "1433");
        extendInfo.put(DatabaseConstants.DATABASE_ID, "2");
        auth.setExtendInfo(extendInfo);
        resource.setAuth(auth);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("123456");
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.AGENTS, Collections.singletonList(protectedResource));
        resource.setDependencies(dependencies);
        resource.setExtendInfo(extendInfo);
        resource.setUuid("654321");
        return resource;
    }
}
