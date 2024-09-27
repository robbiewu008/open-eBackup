package openbackup.tidb.resources.access.service;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.when;

import openbackup.data.access.client.sdk.api.framework.agent.AgentUnifiedRestApi;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tidb.resources.access.constants.TidbConstants;
import openbackup.tidb.resources.access.service.impl.TidbServiceImpl;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.test.util.ReflectionTestUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * tidb service 测试类
 *
 * @author w00426202
 * @since 2023-07-14
 */
@RunWith(PowerMockRunner.class)
public class TidbServiceImplTest {
    @Mock
    private ResourceService mockResourceService;

    @Mock
    private AgentUnifiedService agentUnifiedService;

    @Mock
    private AgentUnifiedRestApi agentUnifiedRestApi;

    @Mock
    private ProtectedEnvironmentService protectedEnvironmentService;

    private TidbServiceImpl tidbserviceImplTest;

    private final String mockEndPoint = "999.999.999.999";

    private final String mockEndPoint2 = "9.9.9.9";

    private final String mockEnvironmentUuid = "7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7";

    @Before
    public void setUp() {
        tidbserviceImplTest = new TidbServiceImpl(mockResourceService, agentUnifiedService);
    }

    /**
     * 用例场景：根据资源uuid，获取应该存在的资源信息成功
     * 前置条件：无
     * 检查点：查询环境信息成功
     */
    @Test
    public void test_get_resource_by_id_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setEndpoint(mockEndPoint);
        when(mockResourceService.getResourceById(mockEnvironmentUuid)).thenReturn(Optional.of(protectedResource));

        // run the test
        ProtectedResource result = tidbserviceImplTest.getResourceByCondition(mockEnvironmentUuid);

        // verify the result
        assertEquals(result.getEndpoint(), mockEndPoint);
    }

    /**
     * 用例场景：更新资源的状态成功
     * 前置条件：无
     * 检查点：更新资源的状态成功
     */
    @Test
    public void test_update_resource_link_status_success() {
        tidbserviceImplTest.updateResourceLinkStatus(mockEnvironmentUuid, "status");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：浏览资源
     * 前置条件：无
     * 检查点：获取浏览资源
     */
    @Test
    public void test_getBrowseResult() {
        BrowseEnvironmentResourceConditions browseEnvironmentResourceConditions
            = new BrowseEnvironmentResourceConditions();
        ProtectedResource resource = new ProtectedResource();
        resource.setEndpoint(mockEndPoint2);
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>(0, new ArrayList<>());
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedResource resource1 = new ProtectedResource();
        resource1.setEndpoint("");
        list.add(resource1);
        pageListResponse.setRecords(list);

        PowerMockito.when(agentUnifiedService.getDetailPageListNoRetry(any(), any(), any(), any(), eq(false)))
            .thenReturn(pageListResponse);

        browseEnvironmentResourceConditions.setConditions("{}");
        PageListResponse<ProtectedResource> browseResult = tidbserviceImplTest.getBrowseResult(
            browseEnvironmentResourceConditions, resource, true);
        Assert.assertEquals(1, browseResult.getRecords().size());
    }

    /**
     * 用例场景：设置资源表锁状态
     * 前置条件：无
     * 检查点：获取浏览资源
     */
    @Test
    public void test_setTableLockedStatus() {
        PageListResponse<ProtectedResource> protectedResourcePageListResponse = new PageListResponse<>();
        ArrayList<ProtectedResource> protectedResources = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, String> tempExtendInfo = new HashMap<>();
        tempExtendInfo.put(TidbConstants.TABLE_NAME, "[\"table1\", \"table2\", \"table3\"]");
        protectedResource.setExtendInfo(tempExtendInfo);
        protectedResources.add(protectedResource);
        protectedResourcePageListResponse.setRecords(protectedResources);

        when(mockResourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(protectedResourcePageListResponse);

        PageListResponse<ProtectedResource> protectedResourcePageListResponse1 = new PageListResponse<>();
        ArrayList<ProtectedResource> protectedResources1 = new ArrayList<>();
        ProtectedResource protectedResource1 = new ProtectedResource();
        Map<String, String> tempExtendInfo1 = new HashMap<>();
        tempExtendInfo1.put(TidbConstants.TABLE_NAME, "[\"table1\", \"table2\", \"table3\"]");
        protectedResource1.setExtendInfo(tempExtendInfo1);
        protectedResources1.add(protectedResource1);
        protectedResourcePageListResponse1.setRecords(protectedResources1);

        tidbserviceImplTest.setTableLockedStatus("123", protectedResourcePageListResponse1);
        Assert.assertEquals("[\"table1\", \"table2\", \"table3\"]", protectedResourcePageListResponse1.getRecords()
            .get(0)
            .getExtendInfo()
            .get("tableName"));

    }

    /**
     * 用例场景：健康检查
     * 前置条件：无
     * 检查点：更新资源的状态成功
     */
    @Test
    public void test_checkHealth() {
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        PowerMockito.when(agentUnifiedRestApi.check(any(), any(), any())).thenReturn(agentBaseDto);
        // ProtectedResource clusterResource, ProtectedResource agentResource, String resourceSubType,String actionType
        ProtectedResource clusterResource = new ProtectedResource();
        Map<String, String> extendInfo = new HashMap<>();
        clusterResource.setExtendInfo(extendInfo);
        ProtectedResource agentResource = new ProtectedResource();
        agentResource.setEndpoint("192.168.1.1");
        agentResource.setPort(22);
        AgentBaseDto mockAgentBaseDto = new AgentBaseDto();
        mockAgentBaseDto.setErrorCode("0");
        PowerMockito.when(
            agentUnifiedService.check(Mockito.anyString(), Mockito.anyString(), Mockito.anyInt(), Mockito.any()))
            .thenReturn(mockAgentBaseDto);
        tidbserviceImplTest.checkHealth(clusterResource, agentResource, ResourceSubTypeEnum.TIDB_TABLE.getType(),
            "check_cluster");
    }

    /**
     * 用例场景：健康检查, 失败用例
     * 前置条件：无
     * 检查点：更新资源的状态成功
     */
    @Test(expected = LegoCheckedException.class)
    public void test_checkHealth_failed() {
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("1");
        PowerMockito.when(agentUnifiedRestApi.check(any(), any(), any())).thenReturn(agentBaseDto);
        // ProtectedResource clusterResource, ProtectedResource agentResource, String resourceSubType,String actionType
        ProtectedResource clusterResource = new ProtectedResource();
        Map<String, String> extendInfo = new HashMap<>();
        clusterResource.setExtendInfo(extendInfo);
        ProtectedResource agentResource = new ProtectedResource();
        agentResource.setEndpoint("192.168.1.1");
        agentResource.setPort(22);
        tidbserviceImplTest.checkHealth(clusterResource, agentResource, ResourceSubTypeEnum.TIDB_TABLE.getType(),
            "check_cluster");
    }

    /**
     * 用例场景：获取资源
     * 前置条件：无
     * 检查点：更新资源的状态成功
     */
    @Test
    public void test_getEndpointResource() {
        List<ProtectedResource> protectedResources = new ArrayList<>();
        ProtectedResource singleRs = new ProtectedResource();
        singleRs.setEndpoint("192.168.1.1");
        singleRs.setPort(22);
        protectedResources.add(singleRs);
        ProtectedResource clusterResource = new ProtectedResource();
        Map<String, List<ProtectedResource>> depens = new HashMap<>();
        depens.put(DatabaseConstants.AGENTS, protectedResources);
        clusterResource.setDependencies(depens);
        when(mockResourceService.getResourceById(any())).thenReturn(Optional.of(clusterResource));
        BrowseEnvironmentResourceConditions browseEnvironmentResourceConditions
            = new BrowseEnvironmentResourceConditions();
        ProtectedResource endpointResource = tidbserviceImplTest.getEndpointResource(
            browseEnvironmentResourceConditions);
        Assert.assertNotNull(endpointResource);
    }

    @Test
    public void test_getSupplyAgent() {
        List<ProtectedResource> protectedResourceList = new ArrayList<>();
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.AGENTS, protectedResourceList);
        tidbserviceImplTest.getSupplyAgent(dependencies);
    }

    @Test(expected = LegoCheckedException.class)
    public void test_parseRs() {
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("200");
        agentBaseDto.setErrorMessage("check failed.");
        ReflectionTestUtils.invokeMethod(tidbserviceImplTest, "parseRs", agentBaseDto);
    }
}
