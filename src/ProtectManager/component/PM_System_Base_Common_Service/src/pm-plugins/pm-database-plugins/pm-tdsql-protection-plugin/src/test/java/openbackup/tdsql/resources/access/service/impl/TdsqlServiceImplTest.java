package openbackup.tdsql.resources.access.service.impl;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThrows;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.Mockito.when;

import openbackup.data.access.client.sdk.api.framework.agent.AgentUnifiedRestApi;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tdsql.resources.access.constant.TdsqlConstant;
import openbackup.tdsql.resources.access.dto.cluster.OssNode;
import openbackup.tdsql.resources.access.dto.cluster.SchedulerNode;
import openbackup.tdsql.resources.access.dto.instance.DataNode;
import openbackup.tdsql.resources.access.dto.instance.GroupInfo;
import openbackup.tdsql.resources.access.dto.instance.TdsqlGroup;

import com.alibaba.fastjson.JSON;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnitRunner;
import org.powermock.api.mockito.PowerMockito;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Optional;

/**
 * 功能描述
 *
 * @author z30047175
 * @since 2023-07-03
 */
@RunWith(MockitoJUnitRunner.class)
public class TdsqlServiceImplTest {
    @Mock
    private ResourceService mockResourceService;

    @Mock
    private AgentUnifiedRestApi mockAgentUnifiedRestApi;

    @Mock
    private AgentUnifiedService mockAgentUnifiedService;

    @Mock
    private DmeUnifiedRestApi mockDmeUnifiedRestApi;

    private TdsqlServiceImpl tdsqlServiceImplUnderTest;

    private final String mockEndPoint = "999.999.999.999";

    private final String mockEndPoint2 = "9.9.9.9";

    private final String mockEnvironmentId1 = "999";

    private final String mockEnvironmentUuid = "7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7";

    @Before
    public void setUp() {
        tdsqlServiceImplUnderTest = new TdsqlServiceImpl(mockResourceService,
            mockAgentUnifiedService, mockDmeUnifiedRestApi);
    }

    /**
     * 用例场景：获取agent环境信息成功
     * 前置条件：无
     * 检查点：查询环境信息成功
     */
    @Test
    public void test_get_environment_by_id_success() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setEndpoint(mockEndPoint);
        when(mockResourceService.getResourceById(mockEnvironmentId1)).thenReturn(Optional.of(protectedEnvironment));

        // run the test
        ProtectedEnvironment result = tdsqlServiceImplUnderTest.getEnvironmentById(mockEnvironmentId1);

        // verify the result
        assertEquals(result.getEndpoint(), mockEndPoint);
    }

    /**
     * 用例场景：获取agent环境资源信息失败
     * 前置条件：资源不存在
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_get_environment_by_id_failed() {
        // Setup
        when(mockResourceService.getResourceById(mockEndPoint)).thenReturn(Optional.empty());

        // Run the test
        assertThrows(LegoCheckedException.class, () -> tdsqlServiceImplUnderTest.getEnvironmentById(mockEndPoint));
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
        ProtectedResource result = tdsqlServiceImplUnderTest.getResourceById(mockEnvironmentUuid);

        // verify the result
        assertEquals(result.getEndpoint(), mockEndPoint);
    }

    /**
     * 用例场景：根据uuid获取环境资源信息失败
     * 前置条件：资源不存在
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_get_resource_by_id_failed() {
        // Setup
        when(mockResourceService.getResourceById(mockEnvironmentUuid)).thenReturn(Optional.empty());

        // Run the test
        assertThrows(LegoCheckedException.class, () -> tdsqlServiceImplUnderTest.getResourceById(mockEnvironmentUuid));
    }

    /**
     * 用例场景：获取Oss节点信息成功
     * 前置条件：获取环境信息成功
     * 检查点：查询OssNode信息成功
     */
    @Test
    public void test_get_oss_node() {
        List<OssNode> ossNodeList = tdsqlServiceImplUnderTest.getOssNode(getClusterEnvironment());
        assertEquals(3, ossNodeList.size());
    }

    /**
     * 用例场景：获取Scheduler节点信息成功
     * 前置条件：获取环境信息成功
     * 检查点：查询SchedulerNode信息成功
     */
    @Test
    public void test_get_scheduler_node() {
        List<SchedulerNode> schedulerNodeList = tdsqlServiceImplUnderTest.getSchedulerNode(getClusterEnvironment());
        assertEquals(3, schedulerNodeList.size());
    }

    /**
     * 用例场景：校验Oss节点连通性成功
     * 前置条件：获取环境信息成功，获取Oss节点信息成功
     * 检查点：校验Oss节点连通性成功
     */
    @Test
    public void test_single_oss_node_connect_check_success() {
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setEndpoint(mockEndPoint2);
        agentEnvironment.setPort(9999);
        when(mockResourceService.getResourceById(mockEnvironmentUuid)).thenReturn(Optional.of(agentEnvironment));
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        agentBaseDto.setErrorMessage("errorMessage");
        when(mockAgentUnifiedRestApi.check(any(), any(), any())).thenReturn(agentBaseDto);
        boolean result = tdsqlServiceImplUnderTest.singleOssNodeConnectCheck(
            tdsqlServiceImplUnderTest.getOssNode(getClusterEnvironment()).get(0), getInstanceEnvironment());
        Assert.assertTrue(result);
    }

    /**
     * 用例场景：校验Oss节点连通性失败
     * 前置条件：获取环境信息成功，获取Oss节点信息成功
     * 检查点：校验Oss节点连通性失败
     */
    @Test
    public void test_single_oss_node_connect_check_failed() {
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setEndpoint(mockEndPoint2);
        agentEnvironment.setPort(9999);
        when(mockResourceService.getResourceById(mockEnvironmentUuid)).thenReturn(Optional.of(agentEnvironment));
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("1");
        agentBaseDto.setErrorMessage("errorMessage");
        when(mockAgentUnifiedRestApi.check(any(), any(), any())).thenReturn(agentBaseDto);
        boolean result = tdsqlServiceImplUnderTest.singleOssNodeConnectCheck(
            tdsqlServiceImplUnderTest.getOssNode(getClusterEnvironment()).get(0), getInstanceEnvironment());
        Assert.assertFalse(result);
    }

    /**
     * 用例场景：当校验Oss节点连通性时获取环境资源失败
     * 前置条件：资源不存在
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_check_oss_node_connect_get_environment_by_id_failed() {
        // Run the test
        assertThrows(LegoCheckedException.class, () -> tdsqlServiceImplUnderTest.singleOssNodeConnectCheck(
            tdsqlServiceImplUnderTest.getOssNode(getClusterEnvironment()).get(0), getInstanceEnvironment()));
    }

    /**
     * 用例场景：校验Scheduler节点连通性成功
     * 前置条件：获取环境信息成功，获取Scheduler节点信息成功
     * 检查点：校验Scheduler节点连通性成功
     */
    @Test
    public void test_single_scheduler_node_connect_check_success() {
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setEndpoint(mockEndPoint2);
        agentEnvironment.setPort(9999);
        when(mockResourceService.getResourceById(mockEnvironmentUuid)).thenReturn(Optional.of(agentEnvironment));
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        agentBaseDto.setErrorMessage("errorMessage");
        when(mockAgentUnifiedRestApi.check(any(), any(), any())).thenReturn(agentBaseDto);
        boolean result = tdsqlServiceImplUnderTest.singleSchedulerNodeConnectCheck(
            tdsqlServiceImplUnderTest.getSchedulerNode(getClusterEnvironment()).get(0), getInstanceEnvironment());
        Assert.assertTrue(result);
    }

    /**
     * 用例场景：校验Scheduler节点连通性失败
     * 前置条件：获取环境信息成功，获取Scheduler节点信息成功
     * 检查点：校验Scheduler节点连通性失败
     */
    @Test
    public void test_single_scheduler_node_connect_check_failed() {
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setEndpoint(mockEndPoint2);
        agentEnvironment.setPort(9999);
        when(mockResourceService.getResourceById(mockEnvironmentUuid)).thenReturn(Optional.of(agentEnvironment));
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("1");
        agentBaseDto.setErrorMessage("errorMessage");
        when(mockAgentUnifiedRestApi.check(any(), any(), any())).thenReturn(agentBaseDto);
        boolean result = tdsqlServiceImplUnderTest.singleSchedulerNodeConnectCheck(
            tdsqlServiceImplUnderTest.getSchedulerNode(getClusterEnvironment()).get(0), getInstanceEnvironment());
        Assert.assertFalse(result);
    }

    /**
     * 用例场景：当校验Scheduler节点连通性时获取环境资源失败
     * 前置条件：资源不存在
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_check_scheduler_node_connect_get_environment_by_id_failed() {
        // Run the test
        assertThrows(LegoCheckedException.class, () -> tdsqlServiceImplUnderTest.singleSchedulerNodeConnectCheck(
            tdsqlServiceImplUnderTest.getSchedulerNode(getClusterEnvironment()).get(0), getInstanceEnvironment()));
    }

    /**
     * 用例场景：校验Data节点连通性成功
     * 前置条件：获取环境信息成功，获取Data节点信息成功
     * 检查点：校验Data节点连通性成功
     */
    @Test
    public void test_single_data_node_connect_check_success() {
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setEndpoint(mockEndPoint2);
        agentEnvironment.setPort(9999);
        when(mockResourceService.getResourceById(mockEnvironmentUuid)).thenReturn(Optional.of(agentEnvironment));
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        agentBaseDto.setErrorMessage("errorMessage");
        when(mockAgentUnifiedRestApi.check(any(), any(), any())).thenReturn(agentBaseDto);
        boolean result = tdsqlServiceImplUnderTest.singleDataNodeConnectCheck(getDataNode(), getInstanceEnvironment());
        Assert.assertTrue(result);
    }

    /**
     * 用例场景：校验Data节点连通性失败
     * 前置条件：获取环境信息成功，获取Data节点信息成功
     * 检查点：校验Data节点连通性失败
     */
    @Test
    public void test_single_data_node_connect_check_failed() {
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setEndpoint(mockEndPoint2);
        agentEnvironment.setPort(9999);
        when(mockResourceService.getResourceById(mockEnvironmentUuid)).thenReturn(Optional.of(agentEnvironment));
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("1");
        agentBaseDto.setErrorMessage("errorMessage");
        when(mockAgentUnifiedRestApi.check(any(), any(), any())).thenReturn(agentBaseDto);
        boolean result = tdsqlServiceImplUnderTest.singleDataNodeConnectCheck(getDataNode(), getInstanceEnvironment());
        Assert.assertFalse(result);
    }

    /**
     * 用例场景：当校验Scheduler节点连通性时获取环境资源失败
     * 前置条件：资源不存在
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_check_data_node_connect_get_environment_by_id_failed() {
        // Run the test
        assertThrows(LegoCheckedException.class,
            () -> tdsqlServiceImplUnderTest.singleDataNodeConnectCheck(getDataNode(), getInstanceEnvironment()));
    }

    /**
     * 用例场景：获取环境下面的所有子实例成功
     * 前置条件：无
     * 检查点：获取环境下面的所有子实例成功
     */
    @Test
    public void test_get_children_success() {
        PageListResponse pageListResponse = new PageListResponse();
        pageListResponse.setRecords(Collections.singletonList(getClusterEnvironment()));
        pageListResponse.setTotalCount(1);
        PowerMockito.when(mockResourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(pageListResponse);
        String name = tdsqlServiceImplUnderTest.getChildren("96590445-0df7-31f4-806b-9fb9e4ed548d", ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType()).get(0).getName();
        Assert.assertEquals(name, "testzyl");
    }

    /**
     * 用例场景：更新资源的状态成功
     * 前置条件：无
     * 检查点：更新资源的状态成功
     */
    @Test
    public void test_update_resource_link_status_success() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("resource_uuid1");
        String instanceInfoStr
            = "{\"id\":\"set_1690526294_5\",\"name\":\"set_1690526294_5\",\"type\":\"0\",\"cluster\":\"a851a36d-1c79-304b-8177-00dea9a2cdee\",\"groups\":[{\"setId\":\"set_1690526294_5\",\"dataNodes\":[{\"ip\":\"8.40.168.191\",\"port\":\"4003\",\"defaultsFile\":\"/data/tdsql_run/4003/mysql-server-8.0.24/etc/my_4003.cnf\",\"socket\":\"/data/4003/prod/mysql.sock\",\"isMaster\":\"1\",\"priority\":\"2\",\"nodeType\":\"dataNode\",\"parentUuid\":\"environment_uuid1\",\"linkStatus\":\"1\"},{\"ip\":\"8.40.168.190\",\"port\":\"4003\",\"defaultsFile\":\"/data/tdsql_run/4003/mysql-server-8.0.24/etc/my_4003.cnf\",\"socket\":\"/data/4003/prod/mysql.sock\",\"isMaster\":\"0\",\"priority\":\"2\",\"nodeType\":\"dataNode\",\"parentUuid\":\"environment_uuid2\",\"linkStatus\":\"1\"}]}]}";
        protectedEnvironment.setExtendInfoByKey(TdsqlConstant.CLUSTER_INSTANCE_INFO, instanceInfoStr);

        tdsqlServiceImplUnderTest.updateInstanceLinkStatus(protectedEnvironment, "0");
        Assert.assertEquals(mockEnvironmentUuid, "7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7");
    }

    /**
     * 用例场景：获取计算节点对应的dataNode列表
     * 前置条件：无
     * 检查点：获取计算节点对应的dataNode列表成功
     */
    @Test
    public void test_get_instance_data_nodes_success() {
        List<DataNode> dataNodes = tdsqlServiceImplUnderTest.getInstanceDataNodes(getInstanceEnvironment());
        String json
            = "[{\"setId\": \"set_1685328362_6\", \"ip\": \"8.40.147.38\", \"port\": \"4002\", \"isMaster\": 1, \"defaultsFile\": \"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\", \"socket\": \"/data1/tdengine/data/4002/prod/mysql.sock\"}, {\"setId\": \"set_1685328362_6\", \"ip\": \"8.40.147.39\", \"port\": \"4002\", \"isMaster\": 0, \"defaultsFile\": \"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\", \"socket\": \"/data1/tdengine/data/4002/prod/mysql.sock\"}, {\"setId\": \"set_1685328362_6\", \"ip\": \"8.40.147.40\", \"port\": \"4002\", \"isMaster\": 0, \"defaultsFile\": \"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\", \"socket\": \"/data1/tdengine/data/4002/prod/mysql.sock\"}]";
        List<DataNode> dataNodeList = JSON.parseArray(json, DataNode.class);
        Assert.assertEquals(dataNodes, dataNodeList);
    }

    private ProtectedEnvironment getInstanceEnvironment() {
        String json
            = "{\"uuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"name\":\"test1\",\"type\":\"Database\",\"subType\":\"TDSQL-cluster\",\"parentName\":\"\",\"parentUuid\":\"\",\"extendInfo\":{\"clusterInstanceInfo\":\"{\\\"groups\\\": [{\\\"setId\\\": \\\"set_1685328362_6\\\", \\\"dataNodes\\\": [{\\\"setId\\\": \\\"set_1685328362_6\\\", \\\"ip\\\": \\\"8.40.147.38\\\", \\\"port\\\": \\\"4002\\\", \\\"isMaster\\\": 1, \\\"defaultsFile\\\": \\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\", \\\"socket\\\": \\\"/data1/tdengine/data/4002/prod/mysql.sock\\\"}, {\\\"setId\\\": \\\"set_1685328362_6\\\", \\\"ip\\\": \\\"8.40.147.39\\\", \\\"port\\\": \\\"4002\\\", \\\"isMaster\\\": 0, \\\"defaultsFile\\\": \\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\", \\\"socket\\\": \\\"/data1/tdengine/data/4002/prod/mysql.sock\\\"}, {\\\"setId\\\": \\\"set_1685328362_6\\\", \\\"ip\\\": \\\"8.40.147.40\\\", \\\"port\\\": \\\"4002\\\", \\\"isMaster\\\": 0, \\\"defaultsFile\\\": \\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\", \\\"socket\\\": \\\"/data1/tdengine/data/4002/prod/mysql.sock\\\"}]}], \\\"id\\\": \\\"set_1685328362_6\\\", \\\"type\\\": \\\"0\\\"}\"}}";
        return JsonUtil.read(json, ProtectedEnvironment.class);
    }

    private ProtectedEnvironment getClusterEnvironment() {
        String json
            = "{\"uuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"name\":\"testzyl\",\"type\":\"Database\",\"subType\":\"TDSQL-cluster\",\"createdTime\":\"2023-05-31 20:10:22.084\",\"rootUuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"sourceType\":\"register\",\"protectionStatus\":0,\"extendInfo\":{\"linkStatus\":\"1\",\"clusterInfo\":\"{\\\"ossNodes\\\":[{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\\\",\\\"ip\\\":\\\"8.40.147.38\\\",\\\"port\\\":\\\"8080\\\"},{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\\\",\\\"ip\\\":\\\"8.40.147.39\\\",\\\"port\\\":\\\"8080\\\"},{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"16f74c9f-915c-4af6-91f6-40c643f13fd5\\\",\\\"ip\\\":\\\"8.40.147.40\\\",\\\"port\\\":\\\"8080\\\"}],\\\"schedulerNodes\\\":[{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\\\",\\\"ip\\\":\\\"8.40.147.38\\\"},{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\\\",\\\"ip\\\":\\\"8.40.147.39\\\"},{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"16f74c9f-915c-4af6-91f6-40c643f13fd5\\\",\\\"ip\\\":\\\"8.40.147.40\\\"}]}\"},\"endpoint\":\"192.168.147.38,192.168.147.39,192.168.147.40,192.168.147.38,192.168.147.39,192.168.147.40\",\"port\":0,\"auth\":{\"authType\":2,\"authKey\":\"DES\",\"authPwd\":\"DES\",\"extendInfo\":{}},\"dependencies\":{\"agents\":[{\"uuid\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"name\":\"tdsql-h63\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.40,8.40.147.40,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"$citations_agents_96e73f8ccba74641bc75d44c16b7d97e\":\"0fc6cb490c73476bb90aa69e40f3c931\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_5ce5b61e6fed4c618a6131ad28ef2e48\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\"},\"endpoint\":\"192.168.147.40\",\"port\":59531,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false},{\"uuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"name\":\"host-8-40-147-32\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.38,8.40.147.38,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_8503624548bf45d68368c26cf12027dc\":\"0fc6cb490c73476bb90aa69e40f3c931\",\"$citations_agents_435d0f7267f14c40879d86094149ed51\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\"},\"endpoint\":\"192.168.147.38\",\"port\":59530,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false},{\"uuid\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"name\":\"host-8-40-147-33\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.39,8.40.147.39,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"$citations_agents_dea1dc850efa4568bc13d815ba0be3d7\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_156c0a940f72494bbfb802a7db1e34d2\":\"0fc6cb490c73476bb90aa69e40f3c931\"},\"endpoint\":\"192.168.147.39\",\"port\":59522,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false}]},\"linkStatus\":\"0\",\"scanInterval\":3600,\"cluster\":false}";
        return JsonUtil.read(json, ProtectedEnvironment.class);
    }

    private DataNode getDataNode() {
        DataNode dataNode = new DataNode();
        dataNode.setNodeType("nodeType");
        dataNode.setLinkStatus("linkStatus");
        dataNode.setIp("ip");
        dataNode.setPort("port");
        dataNode.setParentUuid("7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7");
        dataNode.setPriority("priority");
        dataNode.setSocket("socket");
        dataNode.setDefaultsFile("defaultsFile");
        dataNode.setIsMaster("isMaster");
        return dataNode;
    }

    /**
     * 用例场景：校验分布式实例信息成功
     * 前置条件：获取环境信息成功，获取Data节点信息成功
     * 检查点：校验分布式实例信息成功
     */
    @Test
    public void test_check_group_info_success() {
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setEndpoint(mockEndPoint2);
        agentEnvironment.setPort(9999);
        when(mockResourceService.getResourceById(mockEnvironmentUuid)).thenReturn(Optional.of(agentEnvironment));
        String clusterId = "96590445-0df7-31f4-806b-9fb9e4ed548d";
        when(mockResourceService.getResourceById(clusterId)).thenReturn(Optional.of(getClusterEnvironment()));
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        agentBaseDto.setErrorMessage("errorMessage");
        when(mockAgentUnifiedRestApi.check(any(), any(), any())).thenReturn(agentBaseDto);
        boolean result = tdsqlServiceImplUnderTest.checkGroupInfo(getGroupInfo(), getInstanceEnvironment());
        Assert.assertTrue(result);
    }

    /**
     * 用例场景：校验分布式实例信息失败
     * 前置条件：获取环境信息成功，获取Data节点信息成功
     * 检查点：校验分布式实例信息失败
     */
    @Test
    public void test_check_group_info_fail() {
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setEndpoint(mockEndPoint2);
        agentEnvironment.setPort(9999);
        when(mockResourceService.getResourceById(mockEnvironmentUuid)).thenReturn(Optional.of(agentEnvironment));
        String clusterId = "96590445-0df7-31f4-806b-9fb9e4ed548d";
        when(mockResourceService.getResourceById(clusterId)).thenReturn(Optional.of(getClusterEnvironment()));
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("1");
        agentBaseDto.setErrorMessage("errorMessage");
        when(mockAgentUnifiedRestApi.check(any(), any(), any())).thenReturn(agentBaseDto);
        boolean result = tdsqlServiceImplUnderTest.checkGroupInfo(getGroupInfo(), getInstanceEnvironment());
        Assert.assertFalse(result);
    }

    /**
     * 用例场景：校验分布式实例信息成功
     * 前置条件：获取环境信息成功，获取Data节点信息成功
     * 检查点：校验分布式实例信息成功
     */
    @Test
    public void test_check_data_node_match_success() {
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setEndpoint(mockEndPoint2);
        agentEnvironment.setPort(9999);
        when(mockResourceService.getResourceById(mockEnvironmentUuid)).thenReturn(Optional.of(agentEnvironment));
        String clusterId = "96590445-0df7-31f4-806b-9fb9e4ed548d";
        when(mockResourceService.getResourceById(clusterId)).thenReturn(Optional.of(getClusterEnvironment()));
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        agentBaseDto.setErrorMessage("errorMessage");
        when(mockAgentUnifiedRestApi.check(any(), any(), any())).thenReturn(agentBaseDto);
        boolean result = tdsqlServiceImplUnderTest.checkDataNodeIsMatchAgent(
            getGroupInfo().getGroup().getDataNodes().get(0), getInstanceEnvironment());
        Assert.assertTrue(result);
    }

    /**
     * 用例场景：校验分布式实例信息失败
     * 前置条件：获取环境信息成功，获取Data节点信息成功
     * 检查点：校验分布式实例信息失败
     */
    @Test
    public void test_check_data_node_match_fail() {
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setEndpoint(mockEndPoint2);
        agentEnvironment.setPort(9999);
        when(mockResourceService.getResourceById(mockEnvironmentUuid)).thenReturn(Optional.of(agentEnvironment));
        String clusterId = "96590445-0df7-31f4-806b-9fb9e4ed548d";
        when(mockResourceService.getResourceById(clusterId)).thenReturn(Optional.of(getClusterEnvironment()));
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("1");
        agentBaseDto.setErrorMessage("errorMessage");
        when(mockAgentUnifiedRestApi.check(any(), any(), any())).thenReturn(agentBaseDto);
        boolean result = tdsqlServiceImplUnderTest.checkDataNodeIsMatchAgent(
            getGroupInfo().getGroup().getDataNodes().get(0), getInstanceEnvironment());
        Assert.assertFalse(result);
    }

    /**
     * 用例场景：移除资源的数据存储仓白名单
     * 前置条件：Pm环境正常
     * 检查点：无异常
     */
    @Test
    public void removeDataRepoWhiteListOfResource_success() {
        Mockito.doNothing().when(mockDmeUnifiedRestApi).removeRepoWhiteListOfResource(any());
        tdsqlServiceImplUnderTest.removeDataRepoWhiteListOfResource("123");
    }

    /**
     * 用例场景：日志仓解挂载
     * 前置条件：Pm环境正常
     * 检查点：无异常
     */
    @Test
    public void test_umountDataRepo_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setParentUuid(mockEnvironmentUuid);
        TdsqlGroup tdsqlGroup = getGroupInfo();
        ProtectedEnvironment dbRecord = new ProtectedEnvironment();
        dbRecord.setParentUuid(protectedResource.getParentUuid());
        PowerMockito.when(mockResourceService.getResourceById(any())).thenReturn(Optional.of(dbRecord));

        PowerMockito.doNothing()
            .when(mockAgentUnifiedService)
            .removeProtectUnmountRepoNoRetry(any(), any(), any(), any());
        tdsqlServiceImplUnderTest.umountDataRepo(tdsqlGroup, protectedResource);
    }

    private TdsqlGroup getGroupInfo() {
        TdsqlGroup tdsqlGroup = new TdsqlGroup();
        tdsqlGroup.setId("group_123456");
        tdsqlGroup.setType("1");
        tdsqlGroup.setName("group_123456");
        tdsqlGroup.setCluster("96590445-0df7-31f4-806b-9fb9e4ed548d");
        GroupInfo groupInfo = new GroupInfo();
        groupInfo.setSetIds( Arrays.asList("set_1", "set_2", "set_3"));
        DataNode dataNode = new DataNode();
        dataNode.setIp("8.40.168.190");
        dataNode.setParentUuid("7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7");
        groupInfo.setDataNodes(Arrays.asList(dataNode));
        tdsqlGroup.setGroup(groupInfo);
        return tdsqlGroup;
    }
}
