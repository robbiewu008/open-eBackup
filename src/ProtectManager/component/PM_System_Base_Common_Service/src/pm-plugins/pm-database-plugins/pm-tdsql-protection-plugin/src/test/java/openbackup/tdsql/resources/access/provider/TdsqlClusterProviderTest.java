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
package openbackup.tdsql.resources.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;

import openbackup.access.framework.resource.service.provider.UnifiedConnectionCheckProvider;
import openbackup.data.access.client.sdk.api.framework.agent.AgentUnifiedRestApi;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tdsql.resources.access.constant.TdsqlConstant;
import openbackup.tdsql.resources.access.dto.cluster.OssNode;
import openbackup.tdsql.resources.access.dto.cluster.SchedulerNode;
import openbackup.tdsql.resources.access.service.TdsqlService;

import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 功能描述
 *
 */
@RunWith(MockitoJUnitRunner.class)
public class TdsqlClusterProviderTest {
    @Mock
    private ProviderManager mockProviderManager;

    @Mock
    private PluginConfigManager mockPluginConfigManager;

    @Mock
    private TdsqlService mockTdsqlService;

    @Mock
    private ResourceService mockResourceService;

    @Mock
    private UnifiedConnectionCheckProvider mockUnifiedConnectionCheckProvider;

    private TdsqlClusterProvider tdsqlClusterProviderUnderTest;

    @Mock
    private AgentUnifiedRestApi mockAgentUnifiedRestApi;

    @Before
    public void setUp() {
        tdsqlClusterProviderUnderTest = new TdsqlClusterProvider(mockProviderManager, mockPluginConfigManager,
            mockTdsqlService, mockResourceService);
    }

    /**
     * 用例场景：TDSQL环境检查类过滤
     * 前置条件：无
     * 检查点：过滤成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(tdsqlClusterProviderUnderTest.applicable(ResourceSubTypeEnum.TDSQL_CLUSTER.getType()));
        Assert.assertFalse(tdsqlClusterProviderUnderTest.applicable("object"));
    }

    /**
     * 用例场景：注册/更新集群信息时对环境信息进行检查
     * 前置条件：集群信息正确
     * 检查点：环境信息进行检查无异常
     */
    @Test
    public void test_check_success() {
        // mock node connection check true
        when(mockTdsqlService.singleOssNodeConnectCheck(any(), any())).thenReturn(true);
        when(mockTdsqlService.singleSchedulerNodeConnectCheck(any(), any())).thenReturn(true);

        // mock data
        List<OssNode> ossNodes = Arrays.asList(getOssNode());
        when(mockTdsqlService.getOssNode(any())).thenReturn(ossNodes);
        List<SchedulerNode> schedulerNodes = Arrays.asList(getSchedulerNode());
        when(mockTdsqlService.getSchedulerNode(any())).thenReturn(schedulerNodes);
        when(mockTdsqlService.getEnvironmentById(any())).thenReturn(getClusterEnvironment());

        // mock appEnvResponse
        AppEnvResponse appEnvResponse = getAppEnvResponse();
        when(mockTdsqlService.queryClusterInfo(any(), any())).thenReturn(appEnvResponse);

        // run the test
        ProtectedEnvironment protectedEnvironment = getClusterEnvironment();
        tdsqlClusterProviderUnderTest.register(protectedEnvironment);
        Assert.assertEquals(protectedEnvironment.getEndpoint(), "192.168.147.38,192.168.147.39,192.168.147.40");
    }

    /**
     * 用例场景：注册/更新集群信息时对环境信息进行检查
     * 前置条件：版本信息缺失
     * 检查点：抛出异常
     */
    @Test
    public void test_check_failed_should_throw_get_cluster_version_failed() {
        // mock node connection check true
        when(mockTdsqlService.singleOssNodeConnectCheck(any(), any())).thenReturn(true);
        when(mockTdsqlService.singleSchedulerNodeConnectCheck(any(), any())).thenReturn(true);

        // mock data
        List<OssNode> ossNodes = Arrays.asList(getOssNode());
        when(mockTdsqlService.getOssNode(any())).thenReturn(ossNodes);
        List<SchedulerNode> schedulerNodes = Arrays.asList(getSchedulerNode());
        when(mockTdsqlService.getSchedulerNode(any())).thenReturn(schedulerNodes);
        when(mockTdsqlService.getEnvironmentById(any())).thenReturn(getClusterEnvironment());

        // run the test
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlClusterProviderUnderTest.register(getClusterEnvironment()));
        Assert.assertEquals("get cluster version failed.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.TARGET_CLUSTER_ADD_FAILED, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：注册/更新集群信息时对环境信息进行检查
     * 前置条件：集群Oss节点连通性校验失败
     * 检查点：抛出异常
     */
    @Test
    public void test_check_failed_should_throw_failed_to_connect_to_the_target_oss_cluster() {
        // mock oss node connection check false, scheduler node connection check true
        when(mockTdsqlService.singleOssNodeConnectCheck(any(), any())).thenReturn(false);

        // mock data
        List<OssNode> ossNodes = Arrays.asList(getOssNode());
        when(mockTdsqlService.getOssNode(any())).thenReturn(ossNodes);
        List<SchedulerNode> schedulerNodes = Arrays.asList(getSchedulerNode());
        when(mockTdsqlService.getSchedulerNode(any())).thenReturn(schedulerNodes);
        when(mockTdsqlService.getEnvironmentById(any())).thenReturn(getClusterEnvironment());

        // run the test
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlClusterProviderUnderTest.register(getClusterEnvironment()));
        Assert.assertEquals("Failed to connect to the target oss cluster.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.TARGET_CLUSTER_ADD_FAILED, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：注册/更新集群信息时对环境信息进行检查
     * 前置条件：集群scheduler节点连通性校验失败
     * 检查点：抛出异常
     */
    @Test
    public void test_check_failed_should_throw_failed_to_connect_to_the_target_scheduler_cluster() {
        // mock oss node connection check false, scheduler node connection check true
        when(mockTdsqlService.singleOssNodeConnectCheck(any(), any())).thenReturn(true);
        when(mockTdsqlService.singleSchedulerNodeConnectCheck(any(), any())).thenReturn(false);

        // mock data
        List<OssNode> ossNodes = Arrays.asList(getOssNode());
        when(mockTdsqlService.getOssNode(any())).thenReturn(ossNodes);
        List<SchedulerNode> schedulerNodes = Arrays.asList(getSchedulerNode());
        when(mockTdsqlService.getSchedulerNode(any())).thenReturn(schedulerNodes);
        when(mockTdsqlService.getEnvironmentById(any())).thenReturn(getClusterEnvironment());

        // run the test
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlClusterProviderUnderTest.register(getClusterEnvironment()));
        Assert.assertEquals("Failed to connect to the target scheduler cluster.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.TARGET_CLUSTER_ADD_FAILED, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：注册/更新集群信息时对环境信息进行检查
     * 前置条件：Oss节点信息为空
     * 检查点：抛出异常
     */
    @Test
    public void test_check_failed_should_throw_oss_node_param_is_empty() {
        // mock data
        OssNode ossNode = new OssNode();
        List<OssNode> ossNodes = Arrays.asList(ossNode);
        when(mockTdsqlService.getOssNode(any())).thenReturn(ossNodes);
        List<SchedulerNode> schedulerNodes = Arrays.asList(getSchedulerNode());

        // run the test
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlClusterProviderUnderTest.register(getClusterEnvironment()));
        Assert.assertEquals("ossNode param is empty", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：注册/更新集群信息时对环境信息进行检查
     * 前置条件：scheduler节点信息为空
     * 检查点：抛出异常
     */
    @Test
    public void test_check_failed_should_throw_scheduler_node_param_is_empty() {
        // mock data
        List<OssNode> ossNodes = Arrays.asList(getOssNode());
        when(mockTdsqlService.getOssNode(any())).thenReturn(ossNodes);
        SchedulerNode schedulerNode = new SchedulerNode();
        List<SchedulerNode> schedulerNodes = Arrays.asList(schedulerNode);
        when(mockTdsqlService.getSchedulerNode(any())).thenReturn(schedulerNodes);

        // run the test
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlClusterProviderUnderTest.register(getClusterEnvironment()));
        Assert.assertEquals("schedulerNode param is empty", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：注册/更新集群信息时对环境信息进行检查
     * 前置条件：环境信息uuid已被注册
     * 检查点：抛出异常
     */
    @Test
    public void test_check_failed_should_throw_the_cluster_has_been_registered() {
        // mock data
        List<OssNode> ossNodes = Arrays.asList(getOssNode());
        when(mockTdsqlService.getOssNode(any())).thenReturn(ossNodes);
        List<SchedulerNode> schedulerNodes = Arrays.asList(getSchedulerNode());
        when(mockTdsqlService.getSchedulerNode(any())).thenReturn(schedulerNodes);
        ProtectedEnvironment protectedEnvironment = getClusterEnvironment();
        protectedEnvironment.setUuid(null);
        when(mockTdsqlService.getEnvironmentById(any())).thenReturn(protectedEnvironment);
        ProtectedResource protectedResource = getClusterEnvironment();
        protectedResource.setUuid("c6520e1a-0b01-3029-adad-80f6d301cb51");
        when(mockTdsqlService.getResourceById(any())).thenReturn(protectedResource);

        // run the test
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlClusterProviderUnderTest.register(protectedEnvironment));
        Assert.assertEquals("The cluster has been registered.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.PROTECTED_ENV_REPEATED, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：查询环境资源信息
     * 前置条件：集群信息正确，环境信息无异常
     * 检查点：成功返回资源列表
     */
    @Test
    public void test_browse_success() {
        BrowseEnvironmentResourceConditions environmentConditions = new BrowseEnvironmentResourceConditions();
        environmentConditions.setEnvId("envId");
        environmentConditions.setParentId("parentId");
        environmentConditions.setResourceType(ResourceSubTypeEnum.TDSQL_CLUSTER.getType());
        environmentConditions.setPageNo(0);
        environmentConditions.setPageSize(0);
        environmentConditions.setConditions("conditions");

        // Configure AgentUnifiedService.getDetailPageList(...).
        final ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("1878ba2e-be98-4004-890a-ee18cc0e1422");
        protectedResource.setVersion("6.6.6");
        ProtectedObject protectedObject = new ProtectedObject();
        protectedObject.setResourceId("123");
        HashMap<String, String> map = new HashMap<>();
        map.put("version", "7.7.7");
        protectedResource.setExtendInfo(map);
        protectedResource.setProtectedObject(protectedObject);
        final PageListResponse<ProtectedResource> protectedResourcePageListResponse = new PageListResponse<>(0,
            Arrays.asList(protectedResource));
        List<ProtectedResource> records = new ArrayList<>();
        ProtectedResource record = new ProtectedResource();
        record.setUuid("");
        record.setName("");
        record.setType("Database");
        record.setSubType("TDSQL-cluster");
        record.setParentName("");
        record.setParentUuid("");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("clusterInstanceInfo","{\"groups\": [{\"setId\": \"set_1687337011_9\", \"dataNodes\": [{\"setId\": \"set_1687337011_9\", \"ip\": \"8.40.160.135\", \"port\": \"4003\", \"isMaster\": 1, \"defaultsFile\": \"/data/tdsql_run/4003/mysql-server-8.0.24/etc/my_4003.cnf\", \"socket\": \"/data/4003/prod/mysql.sock\", \"linkStatus\": \"0\"}, {\"setId\": \"set_1687337011_9\", \"ip\": \"8.40.160.131\", \"port\": \"4003\", \"isMaster\": 0, \"defaultsFile\": \"/data/tdsql_run/4003/mysql-server-8.0.24/etc/my_4003.cnf\", \"socket\": \"/data/4003/prod/mysql.sock\", \"linkStatus\": \"0\"}]}], \"id\": \"set_1687337011_9\", \"type\": \"0\"}");
        record.setExtendInfo(extendInfo);
        records.add(record);
        protectedResourcePageListResponse.setRecords(records);
        when(mockTdsqlService.getBrowseResult(any(), any())).thenReturn(protectedResourcePageListResponse);

        // Run the test
        PageListResponse<ProtectedResource> result = tdsqlClusterProviderUnderTest.browse(getClusterEnvironment(),
            environmentConditions);
        Assert.assertEquals(result.getRecords().size(), 1);
    }

    /**
     * 用例场景：查询环境资源信息
     * 前置条件：注册实例时输入错误的setID，未获取到数据节点
     * 检查点：抛出异常
     */
    @Test
    public void test_browse_failed_should_throw_no_data_node_info() {
        BrowseEnvironmentResourceConditions environmentConditions = getEnvironmentConditionsFromRegister();

        // Configure AgentUnifiedService.getDetailPageList(...), records is null
        final ProtectedResource protectedResource = new ProtectedResource();
        final PageListResponse<ProtectedResource> protectedResourcePageListResponse = new PageListResponse<>(0,
            Arrays.asList(protectedResource));
        List<ProtectedResource> records = new ArrayList<>();
        protectedResourcePageListResponse.setRecords(records);
        when(mockTdsqlService.getBrowseResult(any(), any())).thenReturn(protectedResourcePageListResponse);

        // Run the test
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlClusterProviderUnderTest.browse(getClusterEnvironment(), environmentConditions));
        Assert.assertEquals("The data node information of the entered instance cannot be scanned or queried.",
            legoCheckedException.getMessage());
        Assert.assertEquals(TdsqlConstant.NO_DATA_NODE_INFO, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：查询环境资源信息
     * 前置条件：注册实例时输入错误的setID，未获取到数据节点，样例2
     * 检查点：抛出异常
     */
    @Test
    public void test_browse_failed_should_throw_no_data_node_info_2() {
        BrowseEnvironmentResourceConditions environmentConditions = getEnvironmentConditionsFromRegister();

        // Configure AgentUnifiedService.getDetailPageList(...), records is null
        final ProtectedResource protectedResource = new ProtectedResource();
        final PageListResponse<ProtectedResource> protectedResourcePageListResponse = new PageListResponse<>(0,
            Arrays.asList(protectedResource));
        List<ProtectedResource> records = new ArrayList<>();
        ProtectedResource record = new ProtectedResource();
        record.setUuid("");
        record.setName("");
        record.setType("Database");
        record.setSubType("TDSQL-cluster");
        record.setParentName("");
        record.setParentUuid("");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("clusterInstanceInfo","{\"groups\": [], \"id\": \"set_1687337011_9\", \"type\": \"0\"}");
        record.setExtendInfo(extendInfo);
        records.add(record);
        protectedResourcePageListResponse.setRecords(records);
        when(mockTdsqlService.getBrowseResult(any(), any())).thenReturn(protectedResourcePageListResponse);

        // Run the test
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlClusterProviderUnderTest.browse(getClusterEnvironment(), environmentConditions));
        Assert.assertEquals("The data node information of the entered instance cannot be scanned or queried",
            legoCheckedException.getMessage());
        Assert.assertEquals(TdsqlConstant.NO_DATA_NODE_INFO, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：查询环境资源信息
     * 前置条件：删除实例后执行备份操作，导致未能获取到数据节点
     * 检查点：抛出异常
     */
    @Test
    public void test_browse_failed_should_throw_no_instance_exists() {
        BrowseEnvironmentResourceConditions environmentConditions = getEnvironmentConditionsFromBackUp();

        // Configure AgentUnifiedService.getDetailPageList(...), records is null
        final ProtectedResource protectedResource = new ProtectedResource();
        final PageListResponse<ProtectedResource> protectedResourcePageListResponse = new PageListResponse<>(0,
            Arrays.asList(protectedResource));
        List<ProtectedResource> records = new ArrayList<>();
        protectedResourcePageListResponse.setRecords(records);
        when(mockTdsqlService.getBrowseResult(any(), any())).thenReturn(protectedResourcePageListResponse);

        // Run the test
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlClusterProviderUnderTest.browse(getClusterEnvironment(), environmentConditions));
        Assert.assertEquals("Check whether the registered instance exists on the TDSQL chitu management console.",
            legoCheckedException.getMessage());
        Assert.assertEquals(TdsqlConstant.NO_INSTANCE_EXISTS, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：查询环境资源信息
     * 前置条件：删除实例后执行备份操作，导致未能获取到数据节点，样例2
     * 检查点：抛出异常
     */
    @Test
    public void test_browse_failed_should_throw_no_instance_exists_2() {
        BrowseEnvironmentResourceConditions environmentConditions = getEnvironmentConditionsFromBackUp();

        // Configure AgentUnifiedService.getDetailPageList(...), records is null
        final ProtectedResource protectedResource = new ProtectedResource();
        final PageListResponse<ProtectedResource> protectedResourcePageListResponse = new PageListResponse<>(0,
            Arrays.asList(protectedResource));
        List<ProtectedResource> records = new ArrayList<>();
        ProtectedResource record = new ProtectedResource();
        record.setUuid("");
        record.setName("");
        record.setType("Database");
        record.setSubType("TDSQL-cluster");
        record.setParentName("");
        record.setParentUuid("");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("clusterInstanceInfo","{\"groups\": [], \"id\": \"set_1687337011_9\", \"type\": \"0\"}");
        record.setExtendInfo(extendInfo);
        records.add(record);
        protectedResourcePageListResponse.setRecords(records);
        when(mockTdsqlService.getBrowseResult(any(), any())).thenReturn(protectedResourcePageListResponse);

        // Run the test
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlClusterProviderUnderTest.browse(getClusterEnvironment(), environmentConditions));
        Assert.assertEquals("Check whether the registered instance exists on the TDSQL chitu management console",
            legoCheckedException.getMessage());
        Assert.assertEquals(TdsqlConstant.NO_INSTANCE_EXISTS, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：受保护环境健康状态检查
     * 前置条件：集群已注册
     * 检查点：环境健康状态检查无异常
     */
    @Test
    public void test_health_check_oss_node_and_scheduler_node_success() {
        ProtectedEnvironment protectedEnvironment = getClusterEnvironment();

        ActionResult actionResult1 = new ActionResult(ActionResult.SUCCESS_CODE, "");
        ActionResult actionResult2 = new ActionResult(ActionResult.SUCCESS_CODE, "");

        List<ActionResult> actionResultList = Lists.newArrayList(actionResult1, actionResult2);
        ResourceCheckContext context = new ResourceCheckContext();
        context.setActionResults(actionResultList);

        when(mockProviderManager.findProvider(any(), any())).thenReturn(mockUnifiedConnectionCheckProvider);
        when(mockUnifiedConnectionCheckProvider.tryCheckConnection(any())).thenReturn(context);

        tdsqlClusterProviderUnderTest.healthCheckWithResultStatus(protectedEnvironment);
        Assert.assertEquals(protectedEnvironment.getEndpoint(), "192.168.147.38,192.168.147.39,192.168.147.40");
    }

    @Test
    public void test_health_check_cluster_group_success() {
        ProtectedEnvironment protectedEnvironment = getClusterEnvironment();

        ActionResult actionResult1 = new ActionResult(ActionResult.SUCCESS_CODE, "");
        ActionResult actionResult2 = new ActionResult(ActionResult.SUCCESS_CODE, "");

        List<ActionResult> actionResultList = Lists.newArrayList(actionResult1, actionResult2);
        ResourceCheckContext context = new ResourceCheckContext();
        context.setActionResults(actionResultList);
        PowerMockito.when(mockTdsqlService.getChildren(protectedEnvironment.getUuid(), ResourceSubTypeEnum.TDSQL_CLUSTERGROUP.getType())).thenReturn(Collections.singletonList(getClusterGroupEnvironment()));
        when(mockProviderManager.findProvider(any(), any())).thenReturn(mockUnifiedConnectionCheckProvider);
        when(mockUnifiedConnectionCheckProvider.tryCheckConnection(any())).thenReturn(context);
        tdsqlClusterProviderUnderTest.healthCheckWithResultStatus(protectedEnvironment);
        Assert.assertEquals(protectedEnvironment.getEndpoint(), "192.168.147.38,192.168.147.39,192.168.147.40");
    }

    @Test
    public void test_health_check_cluster_group_fail() {
        ProtectedEnvironment protectedEnvironment = getClusterEnvironment();

        ActionResult actionResult1 = new ActionResult(ActionResult.SUCCESS_CODE, "");
        ActionResult actionResult2 = new ActionResult(ActionResult.SUCCESS_CODE, "");

        List<ActionResult> actionResultList = Lists.newArrayList(actionResult1, actionResult2);
        ResourceCheckContext context = new ResourceCheckContext();
        context.setActionResults(actionResultList);
        PowerMockito.when(mockTdsqlService.getChildren(protectedEnvironment.getUuid(), ResourceSubTypeEnum.TDSQL_CLUSTERGROUP.getType())).thenReturn(Collections.singletonList(getClusterGroupEnvironment()));
        when(mockProviderManager.findProvider(any(), any())).thenReturn(mockUnifiedConnectionCheckProvider);
        when(mockUnifiedConnectionCheckProvider.tryCheckConnection(any())).thenReturn(context);
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("1");
        agentBaseDto.setErrorMessage("errorMessage");
        when(mockAgentUnifiedRestApi.check(any(), any(), any())).thenReturn(agentBaseDto);
        tdsqlClusterProviderUnderTest.healthCheckWithResultStatus(protectedEnvironment);
        Assert.assertEquals(protectedEnvironment.getEndpoint(), "192.168.147.38,192.168.147.39,192.168.147.40");
    }

    private BrowseEnvironmentResourceConditions getEnvironmentConditionsFromRegister() {
        String json
            = "{\"envId\":\"786f2161-1be9-322c-8355-52a5851e45c7\",\"resourceType\":\"TDSQL-cluster\",\"pageNo\":0,\"pageSize\":200,\"conditions\":\"{\\\"type\\\":\\\"0\\\",\\\"id\\\":\\\"set_1691408284_7\\\"}\"}";
        return JsonUtil.read(json, BrowseEnvironmentResourceConditions.class);
    }

    private BrowseEnvironmentResourceConditions getEnvironmentConditionsFromBackUp() {
        String json
            = "{\"resourceType\":\"TDSQL-cluster\", \"pageNo\":0, \"pageSize\":0, \"conditions\":\"{\\\"id\\\":\\\"set_1691407769_5\\\",\\\"name\\\":\\\"set_1691407769_5\\\",\\\"type\\\":\\\"0\\\",\\\"cluster\\\":\\\"786f2161-1be9-322c-8355-52a5851e45c7\\\",\\\"groups\\\":[{\\\"setId\\\":\\\"set_1691407769_5\\\",\\\"dataNodes\\\":[{\\\"ip\\\":\\\"8.40.168.195\\\",\\\"port\\\":\\\"4002\\\",\\\"defaultsFile\\\":\\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\",\\\"socket\\\":\\\"/data1/tdengine/data/4002/prod/mysql.sock\\\",\\\"isMaster\\\":\\\"1\\\",\\\"priority\\\":\\\"2\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"b87a49f4-3765-4511-9445-89d2e7bcd248\\\",\\\"linkStatus\\\":\\\"1\\\"},{\\\"ip\\\":\\\"8.40.168.193\\\",\\\"port\\\":\\\"4002\\\",\\\"defaultsFile\\\":\\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\",\\\"socket\\\":\\\"/data1/tdengine/data/4002/prod/mysql.sock\\\",\\\"isMaster\\\":\\\"0\\\",\\\"priority\\\":\\\"2\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"4bf8f51a-48c2-4075-b4ae-9b9566b0090b\\\",\\\"linkStatus\\\":\\\"1\\\"},{\\\"ip\\\":\\\"8.40.168.194\\\",\\\"port\\\":\\\"4002\\\",\\\"defaultsFile\\\":\\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\",\\\"socket\\\":\\\"/data1/tdengine/data/4002/prod/mysql.sock\\\",\\\"isMaster\\\":\\\"0\\\",\\\"priority\\\":\\\"2\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"558a648e-841b-497b-b413-0ff1df01c247\\\",\\\"linkStatus\\\":\\\"1\\\"}]}]}\"}";
        return JsonUtil.read(json, BrowseEnvironmentResourceConditions.class);
    }

    private ProtectedEnvironment getClusterEnvironment() {
        String json
            = "{\"uuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"name\":\"testzyl\",\"type\":\"Database\",\"subType\":\"TDSQL-cluster\",\"createdTime\":\"2023-05-31 20:10:22.084\",\"rootUuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"sourceType\":\"register\",\"protectionStatus\":0,\"extendInfo\":{\"linkStatus\":\"1\",\"clusterInfo\":\"{\\\"ossNodes\\\":[{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\\\",\\\"ip\\\":\\\"8.40.147.38\\\",\\\"port\\\":\\\"8080\\\"},{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\\\",\\\"ip\\\":\\\"8.40.147.39\\\",\\\"port\\\":\\\"8080\\\"},{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"16f74c9f-915c-4af6-91f6-40c643f13fd5\\\",\\\"ip\\\":\\\"8.40.147.40\\\",\\\"port\\\":\\\"8080\\\"}],\\\"schedulerNodes\\\":[{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\\\",\\\"ip\\\":\\\"8.40.147.38\\\"},{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\\\",\\\"ip\\\":\\\"8.40.147.39\\\"},{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"16f74c9f-915c-4af6-91f6-40c643f13fd5\\\",\\\"ip\\\":\\\"8.40.147.40\\\"}]}\"},\"endpoint\":\"192.168.147.38,192.168.147.39,192.168.147.40\",\"port\":0,\"auth\":{\"authType\":2,\"authKey\":\"DES\",\"authPwd\":\"DES\",\"extendInfo\":{}},\"dependencies\":{\"agents\":[{\"uuid\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"name\":\"tdsql-h63\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.40,8.40.147.40,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"$citations_agents_96e73f8ccba74641bc75d44c16b7d97e\":\"0fc6cb490c73476bb90aa69e40f3c931\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_5ce5b61e6fed4c618a6131ad28ef2e48\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\"},\"endpoint\":\"192.168.147.40\",\"port\":59531,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false},{\"uuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"name\":\"host-8-40-147-32\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.38,8.40.147.38,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_8503624548bf45d68368c26cf12027dc\":\"0fc6cb490c73476bb90aa69e40f3c931\",\"$citations_agents_435d0f7267f14c40879d86094149ed51\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\"},\"endpoint\":\"192.168.147.38\",\"port\":59530,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false},{\"uuid\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"name\":\"host-8-40-147-33\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.39,8.40.147.39,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"$citations_agents_dea1dc850efa4568bc13d815ba0be3d7\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_156c0a940f72494bbfb802a7db1e34d2\":\"0fc6cb490c73476bb90aa69e40f3c931\"},\"endpoint\":\"192.168.147.39\",\"port\":59522,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false}]},\"linkStatus\":\"0\",\"scanInterval\":3600,\"cluster\":false}";
        return JsonUtil.read(json, ProtectedEnvironment.class);
    }

    private ProtectedEnvironment getClusterGroupEnvironment() {
        String json  =  "{\n" + "  \"uuid\": \"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\n"
            + "  \"name\": \"group_1698889827_3\",\n" + "  \"parentUuid\": \"96590445-0df7-31f4-806b-9fb9e4ed548d\",\n"
            + "  \"type\": \"Database\",\n" + "  \"subType\": \"TDSQL-clusterGroup\",\n" + "  \"extendInfo\": {\n"
            + "    \"clusterGroupInfo\": \"{\\\"id\\\":\\\"group_1698889827_3\\\",\\\"name\\\":\\\"group_1698889827_3\\\",\\\"cluster\\\":\\\"9e68a8f1-7ad4-3eef-a808-dce3b2062120\\\",\\\"type\\\":\\\"1\\\",\\\"group\\\":{\\\"groupId\\\":\\\"group_1698889827_3\\\",\\\"setIds\\\":[\\\"set_1698890174_3\\\",\\\"set_1698890103_1\\\"],\\\"dataNodes\\\":[{\\\"ip\\\":\\\"8.40.168.190\\\",\\\"parentUuid\\\":\\\"dd15b622-fd7c-4a7c-9841-b0fb45b4201f\\\"},{\\\"ip\\\":\\\"8.40.168.191\\\",\\\"parentUuid\\\":\\\"ce56a464-3b7b-4016-88f2-58ae12fb6d1d\\\"},{\\\"ip\\\":\\\"8.40.168.192\\\",\\\"parentUuid\\\":\\\"c75146f7-7e2a-41d6-b110-28d0e22245ee\\\"}]}}\"\n"
            + "  }\n" + "}";
        return JsonUtil.read(json, ProtectedEnvironment.class);
    }

    private OssNode getOssNode() {
        String json
            = "{\"nodeType\":\"ossNode\",\"parentUuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"ip\":\"8.40.147.38\",\"port\":\"8080\"}";
        return JsonUtil.read(json, OssNode.class);
    }

    private SchedulerNode getSchedulerNode() {
        String json
            = "{\"nodeType\":\"schedulerNode\",\"parentUuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"ip\":\"8.40.147.38\"}";
        return JsonUtil.read(json, SchedulerNode.class);
    }

    private AppEnvResponse getAppEnvResponse() {
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("version", "9.9.9");
        appEnvResponse.setExtendInfo(extendInfo);
        appEnvResponse.setUuid("Uuid");
        appEnvResponse.setEndpoint("Endpoint");
        appEnvResponse.setRole(1);
        appEnvResponse.setName("Name");
        appEnvResponse.setSubType("SubType");
        appEnvResponse.setType("Type");
        List<NodeInfo> nodeInfos = new ArrayList<>();
        appEnvResponse.setNodes(nodeInfos);
        return appEnvResponse;
    }
}
