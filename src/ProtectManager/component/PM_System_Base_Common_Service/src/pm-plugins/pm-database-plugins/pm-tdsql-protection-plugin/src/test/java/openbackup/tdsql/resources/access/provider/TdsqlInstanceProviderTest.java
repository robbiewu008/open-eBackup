package openbackup.tdsql.resources.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tdsql.resources.access.constant.TdsqlConstant;
import openbackup.tdsql.resources.access.dto.instance.DataNode;
import openbackup.tdsql.resources.access.dto.instance.TdsqlInstance;
import openbackup.tdsql.resources.access.interceptor.TdsqlTaskCheck;
import openbackup.tdsql.resources.access.service.TdsqlService;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 功能描述
 *
 * @author z30047175
 * @since 2023-07-05
 */
@RunWith(MockitoJUnitRunner.class)
public class TdsqlInstanceProviderTest {
    @Mock
    private TdsqlService mockTdsqlService;
    @Mock
    private TdsqlTaskCheck mockTdsqlTaskCheck;
    private TdsqlInstanceProvider tdsqlInstanceProviderUnderTest;
    @Before
    public void setUp() {
        tdsqlInstanceProviderUnderTest = new TdsqlInstanceProvider(mockTdsqlService, mockTdsqlTaskCheck);
    }

    /**
     * 用例场景：TDSQL环境检查类过滤
     * 前置条件：subType为TDSQL-clusterInstance
     * 检查点：过滤成功
     */
    @Test
    public void test_applicable_success() {
        // Setup
        final ProtectedResource object = new ProtectedResource();
        object.setName("name");
        object.setSubType(ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType());

        // Run the test
        boolean result = tdsqlInstanceProviderUnderTest.applicable(object);

        // Verify the results
        Assert.assertTrue(result);
    }

    /**
     * 用例场景：TDSQL环境检查类过滤
     * 前置条件：subType不为TDSQL-clusterInstance
     * 检查点：过滤成功
     */
    @Test
    public void test_applicable_failed() {
        // Setup
        final ProtectedResource object = new ProtectedResource();
        object.setName("name");
        object.setSubType("object");

        // Run the test
        boolean result = tdsqlInstanceProviderUnderTest.applicable(object);

        // Verify the results
        Assert.assertFalse(result);
    }

    /**
     * 用例场景：注册/更新实例信息时对环境信息进行检查
     * 前置条件：实例信息正确
     * 检查点：环境信息进行检查无异常
     */
    @Test
    public void test_before_create_success() {
        // mock data node connection check true
        when(mockTdsqlService.singleDataNodeConnectCheck(any(), any())).thenReturn(true);

        // mock getInstanceDataNodes()
        String clusterInstanceInfo = getEnvironment().getExtendInfo().get("clusterInstanceInfo");
        TdsqlInstance tdsqlInstance = JsonUtil.read(clusterInstanceInfo, TdsqlInstance.class);
        List<DataNode> dataNodeList = tdsqlInstance.getGroups().get(0).getDataNodes();
        DataNode dataNode = dataNodeList.get(0);
        dataNode.setPriority("priority");
        dataNode.setNodeType("nodeType");
        dataNode.setParentUuid("parentUuid");
        dataNode.setLinkStatus("linkStatus");
        dataNodeList.clear();
        dataNodeList.add(dataNode);
        when(mockTdsqlService.getInstanceDataNodes(any())).thenReturn(dataNodeList);

        // mock environment
        ProtectedEnvironment environment = getEnvironment();
        environment.setEndpoint("9.9.9.9");
        when(mockTdsqlService.getEnvironmentById(any())).thenReturn(environment);

        // mock result of getBrowseResult()
        when(mockTdsqlService.getBrowseResult(any(), any())).thenReturn(getProtectedResourcePageListResponse());


        // Run the test
        ProtectedEnvironment protectedEnvironment = getEnvironment();
        tdsqlInstanceProviderUnderTest.beforeCreate(protectedEnvironment);
        tdsqlInstanceProviderUnderTest.beforeUpdate(protectedEnvironment);
        Assert.assertEquals(protectedEnvironment.getUuid(), "7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7");
    }

    /**
     * 用例场景：注册实例信息时对环境信息进行检查
     * 前置条件：实例节点连通性校验失败
     * 检查点：抛出异常
     */
    @Test
    public void test_before_create_failed_should_throw_the_tdsql_instance_dataNode_query_failed() {
        // mock data node connection check true
        when(mockTdsqlService.singleDataNodeConnectCheck(any(), any())).thenReturn(false);

        // mock getInstanceDataNodes()
        String clusterInstanceInfo = getEnvironment().getExtendInfo().get("clusterInstanceInfo");
        TdsqlInstance tdsqlInstance = JsonUtil.read(clusterInstanceInfo, TdsqlInstance.class);
        List<DataNode> dataNodeList = tdsqlInstance.getGroups().get(0).getDataNodes();
        DataNode dataNode = dataNodeList.get(0);
        dataNode.setPriority("priority");
        dataNode.setNodeType("nodeType");
        dataNode.setParentUuid("parentUuid");
        dataNode.setLinkStatus("linkStatus");
        dataNodeList.clear();
        dataNodeList.add(dataNode);
        when(mockTdsqlService.getInstanceDataNodes(any())).thenReturn(dataNodeList);

        // mock result of getBrowseResult()
        when(mockTdsqlService.getBrowseResult(any(), any())).thenReturn(getProtectedResourcePageListResponse());


        // Run the test
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlInstanceProviderUnderTest.beforeCreate(getEnvironment()));
        Assert.assertEquals("The TDSQL instance dataNode query failed.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：更新实例信息时对环境信息进行检查
     * 前置条件：实例节点连通性校验失败
     * 检查点：抛出异常
     */
    @Test
    public void test_before_update_failed_should_throw_the_tdsql_instance_dataNode_query_failed() {
        // mock data node connection check true
        when(mockTdsqlService.singleDataNodeConnectCheck(any(), any())).thenReturn(false);

        // mock getInstanceDataNodes()
        String clusterInstanceInfo = getEnvironment().getExtendInfo().get("clusterInstanceInfo");
        TdsqlInstance tdsqlInstance = JsonUtil.read(clusterInstanceInfo, TdsqlInstance.class);
        List<DataNode> dataNodeList = tdsqlInstance.getGroups().get(0).getDataNodes();
        DataNode dataNode = dataNodeList.get(0);
        dataNode.setPriority("priority");
        dataNode.setNodeType("nodeType");
        dataNode.setParentUuid("parentUuid");
        dataNode.setLinkStatus("linkStatus");
        dataNodeList.clear();
        dataNodeList.add(dataNode);
        when(mockTdsqlService.getInstanceDataNodes(any())).thenReturn(dataNodeList);

        // mock result of getBrowseResult()
        when(mockTdsqlService.getBrowseResult(any(), any())).thenReturn(getProtectedResourcePageListResponse());

        // Run the test
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlInstanceProviderUnderTest.beforeUpdate(getEnvironment()));
        Assert.assertEquals("The TDSQL instance dataNode query failed.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：创建、更新实例信息时对环境信息进行检查
     * 前置条件：实例节点参数校验 第一部分校验失败
     * 检查点：抛出异常
     */
    @Test
    public void test_before_create_and_before_update_failed_should_throw_tdsql_data_node_param_is_empty_1() {
        // mock getInstanceDataNodes()
        String clusterInstanceInfo = getEnvironment().getExtendInfo().get("clusterInstanceInfo");
        TdsqlInstance tdsqlInstance = JsonUtil.read(clusterInstanceInfo, TdsqlInstance.class);
        List<DataNode> dataNodeList = tdsqlInstance.getGroups().get(0).getDataNodes();
        DataNode dataNode = dataNodeList.get(0);
        dataNode.setPriority("priority");
        dataNode.setNodeType("nodeType");
        dataNode.setParentUuid("parentUuid");
        dataNode.setLinkStatus("linkStatus");
        dataNode.setDefaultsFile(null);
        dataNodeList.clear();
        dataNodeList.add(dataNode);
        when(mockTdsqlService.getInstanceDataNodes(any())).thenReturn(dataNodeList);

        // Run the test
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlInstanceProviderUnderTest.beforeCreate(getEnvironment()));
        Assert.assertEquals("TDSQL DataNode param is empty", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, legoCheckedException.getErrorCode());
        LegoCheckedException legoCheckedException_1 = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlInstanceProviderUnderTest.beforeUpdate(getEnvironment()));
        Assert.assertEquals("TDSQL DataNode param is empty", legoCheckedException_1.getMessage());
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, legoCheckedException_1.getErrorCode());
    }

    /**
     * 用例场景：创建、更新实例信息时对环境信息进行检查
     * 前置条件：实例节点参数校验 第二部分校验失败
     * 检查点：抛出异常
     */
    @Test
    public void test_before_create_and_before_update_failed_should_throw_tdsql_data_node_param_is_empty_2() {
        // mock getInstanceDataNodes()
        String clusterInstanceInfo = getEnvironment().getExtendInfo().get("clusterInstanceInfo");
        TdsqlInstance tdsqlInstance = JsonUtil.read(clusterInstanceInfo, TdsqlInstance.class);
        List<DataNode> dataNodeList = tdsqlInstance.getGroups().get(0).getDataNodes();
        DataNode dataNode = dataNodeList.get(0);
        dataNode.setNodeType("nodeType");
        dataNode.setParentUuid("parentUuid");
        dataNode.setLinkStatus("linkStatus");
        dataNodeList.clear();
        dataNodeList.add(dataNode);
        when(mockTdsqlService.getInstanceDataNodes(any())).thenReturn(dataNodeList);

        // Run the test
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlInstanceProviderUnderTest.beforeCreate(getEnvironment()));
        Assert.assertEquals("TDSQL DataNode param is empty", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, legoCheckedException.getErrorCode());
        LegoCheckedException legoCheckedException_1 = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlInstanceProviderUnderTest.beforeUpdate(getEnvironment()));
        Assert.assertEquals("TDSQL DataNode param is empty", legoCheckedException_1.getMessage());
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, legoCheckedException_1.getErrorCode());
    }

    /**
     * 用例场景：创建、更新实例信息时对环境信息进行检查
     * 前置条件：实例节点参数校验 第三部分校验失败
     * 检查点：抛出异常
     */
    @Test
    public void test_before_create_and_before_update_failed_should_throw_tdsql_data_node_param_is_empty_3() {
        // mock getInstanceDataNodes()
        String clusterInstanceInfo = getEnvironment().getExtendInfo().get("clusterInstanceInfo");
        TdsqlInstance tdsqlInstance = JsonUtil.read(clusterInstanceInfo, TdsqlInstance.class);
        List<DataNode> dataNodeList = tdsqlInstance.getGroups().get(0).getDataNodes();
        DataNode dataNode = dataNodeList.get(0);
        dataNode.setPriority("priority");
        dataNode.setNodeType("nodeType");
        dataNode.setLinkStatus("linkStatus");
        dataNodeList.clear();
        dataNodeList.add(dataNode);
        when(mockTdsqlService.getInstanceDataNodes(any())).thenReturn(dataNodeList);

        // Run the test
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlInstanceProviderUnderTest.beforeCreate(getEnvironment()));
        Assert.assertEquals("TDSQL DataNode param is empty.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, legoCheckedException.getErrorCode());
        LegoCheckedException legoCheckedException_1 = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlInstanceProviderUnderTest.beforeUpdate(getEnvironment()));
        Assert.assertEquals("TDSQL DataNode param is empty.", legoCheckedException_1.getMessage());
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, legoCheckedException_1.getErrorCode());
    }

    /**
     * 用例场景：创建实例信息时对环境信息进行检查
     * 前置条件：实例节点已被注册
     * 检查点：抛出异常
     */
    @Test
    public void test_before_create_failed_should_throw_the_instance_has_been_registered() {
        // mock getInstanceDataNodes()
        String clusterInstanceInfo = getEnvironment().getExtendInfo().get("clusterInstanceInfo");
        TdsqlInstance tdsqlInstance = JsonUtil.read(clusterInstanceInfo, TdsqlInstance.class);
        List<DataNode> dataNodeList = tdsqlInstance.getGroups().get(0).getDataNodes();
        DataNode dataNode = dataNodeList.get(0);
        dataNode.setPriority("priority");
        dataNode.setNodeType("nodeType");
        dataNode.setParentUuid("parentUuid");
        dataNode.setLinkStatus("linkStatus");
        dataNodeList.clear();
        dataNodeList.add(dataNode);
        when(mockTdsqlService.getInstanceDataNodes(any())).thenReturn(dataNodeList);

        ProtectedEnvironment protectedEnvironment = getEnvironment();
        protectedEnvironment.setExtendInfoByKey("id", null);
        List<ProtectedResource> instances = new ArrayList<>();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("id", "set_1685328362_6");
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setExtendInfo(extendInfo);
        instances.add(protectedResource);
        when(mockTdsqlService.getChildren(any(), any())).thenReturn(instances);

        // Run the test
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlInstanceProviderUnderTest.beforeCreate(protectedEnvironment));
        Assert.assertEquals("The instance has been registered！", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.DB_INSTANCE_HAS_REGISTERED, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：创建实例信息时对环境信息进行检查
     * 前置条件：实例节点已被注册
     * 检查点：抛出异常
     */
    @Test
    public void test_before_create_and_before_update_failed_should_throw_browse_compute_node_zero() {
        // mock getInstanceDataNodes()
        String clusterInstanceInfo = getEnvironment().getExtendInfo().get("clusterInstanceInfo");
        TdsqlInstance tdsqlInstance = JsonUtil.read(clusterInstanceInfo, TdsqlInstance.class);
        List<DataNode> dataNodeList = tdsqlInstance.getGroups().get(0).getDataNodes();
        DataNode dataNode = dataNodeList.get(0);
        dataNode.setPriority("priority");
        dataNode.setNodeType("nodeType");
        dataNode.setParentUuid("parentUuid");
        dataNode.setLinkStatus("linkStatus");
        dataNodeList.clear();
        dataNodeList.add(dataNode);
        when(mockTdsqlService.getInstanceDataNodes(any())).thenReturn(dataNodeList);

        // mock result of getBrowseResult()
        PageListResponse<ProtectedResource> protectedResourcePageListResponse = getProtectedResourcePageListResponse();
        List<ProtectedResource> records = new ArrayList<>();
        protectedResourcePageListResponse.setRecords(records);
        when(mockTdsqlService.getBrowseResult(any(), any())).thenReturn(protectedResourcePageListResponse);

        // Run the test
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlInstanceProviderUnderTest.beforeCreate(getEnvironment()));
        Assert.assertEquals("browse compute node zero", legoCheckedException.getMessage());
        Assert.assertEquals(TdsqlConstant.NO_DATA_NODE_INFO, legoCheckedException.getErrorCode());
        LegoCheckedException legoCheckedException_1 = Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlInstanceProviderUnderTest.beforeUpdate(getEnvironment()));
        Assert.assertEquals("browse compute node zero", legoCheckedException_1.getMessage());
        Assert.assertEquals(TdsqlConstant.NO_DATA_NODE_INFO, legoCheckedException_1.getErrorCode());
    }

    private PageListResponse<ProtectedResource> getProtectedResourcePageListResponse() {
        final ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("1878ba2e-be98-4004-890a-ee18cc0e1422");
        protectedResource.setVersion("9.11.6");
        ProtectedObject protectedObject = new ProtectedObject();
        protectedObject.setResourceId("123");
        HashMap<String, String> map = new HashMap<>();
        map.put("version", "9.11.6");
        protectedResource.setExtendInfo(map);
        protectedResource.setProtectedObject(protectedObject);
        final PageListResponse<ProtectedResource> protectedResourcePageListResponse =
            new PageListResponse<>(0, Arrays.asList(protectedResource));
        List<ProtectedResource> records = new ArrayList<>();
        ProtectedResource record = getEnvironment();
        records.add(record);
        protectedResourcePageListResponse.setRecords(records);
        return protectedResourcePageListResponse;
    }

    private ProtectedEnvironment getEnvironment() {
        String json ="{\"uuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"parentUuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"name\":\"test1\",\"type\":\"Database\",\"subType\":\"TDSQL-clusterInstance\",\"auth\":{\"authType\":2,\"authKey\":\"account\",\"authPwd\":\"password\"},\"port\":\"4444\",\"hostName\":\"8.40.99.99\",\"ip\":\"8.40.99.99\",\"extendInfo\":{\"linkStatus\":\"1\",\"clusterInstanceInfo\":\"{\\\"id\\\":\\\"set_1685328362_6\\\",\\\"name\\\":\\\"set_1685328362_6\\\",\\\"type\\\":\\\"0\\\",\\\"cluster\\\":\\\"cluster-150\\\",\\\"groups\\\":[{\\\"setId\\\":\\\"set_1685328362_6\\\",\\\"dataNodes\\\":[{\\\"setId\\\":\\\"set_1685328362_6\\\",\\\"ip\\\":\\\"8.40.147.38\\\",\\\"port\\\":\\\"4002\\\",\\\"isMaster\\\":1,\\\"defaultsFile\\\":\\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\",\\\"socket\\\":\\\"/data1/tdengine/data/4002/prod/mysql.sock\\\"},{\\\"setId\\\":\\\"set_1685328362_6\\\",\\\"ip\\\":\\\"8.40.147.39\\\",\\\"port\\\":\\\"4002\\\",\\\"isMaster\\\":0,\\\"defaultsFile\\\":\\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\",\\\"socket\\\":\\\"/data1/tdengine/data/4002/prod/mysql.sock\\\"},{\\\"setId\\\":\\\"set_1685328362_6\\\",\\\"ip\\\":\\\"8.40.147.40\\\",\\\"port\\\":\\\"4002\\\",\\\"isMaster\\\":0,\\\"defaultsFile\\\":\\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\",\\\"socket\\\":\\\"/data1/tdengine/data/4002/prod/mysql.sock\\\"}]}],\\\"id\\\":\\\"set_1685328362_6\\\",\\\"type\\\":\\\"0\\\"}\"},\"dependencies\":{\"agents\":[{\"uuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\"},{\"uuid\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\"},{\"uuid\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\"}]}}";
        return JsonUtil.read(json, ProtectedEnvironment.class);
    }

    private TdsqlInstance getInstance() {
        String clusterInstanceInfo = "{\"id\":\"set_1685328362_6\",\"name\":\"set_1685328362_6\",\"type\":\"0\",\"cluster\":\"cluster-150\",\"groups\":[{\"setId\":\"set_1685328362_6\",\"dataNodes\":[{\"setId\":\"set_1685328362_6\",\"ip\":\"8.40.147.38\",\"port\":\"4002\",\"isMaster\":1,\"defaultsFile\":\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\",\"socket\":\"/data1/tdengine/data/4002/prod/mysql.sock\"},{\"setId\":\"set_1685328362_6\",\"ip\":\"8.40.147.39\",\"port\":\"4002\",\"isMaster\":0,\"defaultsFile\":\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\",\"socket\":\"/data1/tdengine/data/4002/prod/mysql.sock\"},{\"setId\":\"set_1685328362_6\",\"ip\":\"8.40.147.40\",\"port\":\"4002\",\"isMaster\":0,\"defaultsFile\":\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\",\"socket\":\"/data1/tdengine/data/4002/prod/mysql.sock\"}]}],\"id\":\"set_1685328362_6\",\"type\":\"0\"}";
        return JsonUtil.read(clusterInstanceInfo, TdsqlInstance.class);
    }
}
