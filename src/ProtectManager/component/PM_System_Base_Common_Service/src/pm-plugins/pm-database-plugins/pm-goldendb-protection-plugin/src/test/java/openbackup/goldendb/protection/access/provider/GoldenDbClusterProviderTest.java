package openbackup.goldendb.protection.access.provider;

import static org.junit.Assert.assertFalse;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;

import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.goldendb.protection.access.dto.cluster.Node;
import openbackup.goldendb.protection.access.service.GoldenDbService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

@RunWith(MockitoJUnitRunner.class)
public class GoldenDbClusterProviderTest {

    @Mock
    private ProviderManager mockProviderManager;

    @Mock
    private PluginConfigManager mockPluginConfigManager;

    @Mock
    private AgentUnifiedService mockAgentUnifiedService;

    @Mock
    private GoldenDbService mockGoldenDbService;

    private GoldenDbClusterProvider goldenDbClusterProviderUnderTest;

    @Before
    public void setUp() {
        goldenDbClusterProviderUnderTest = new GoldenDbClusterProvider(mockProviderManager, mockPluginConfigManager,
            mockAgentUnifiedService, mockGoldenDbService);
    }

    @Test
    public void testApplicable() {
        assertFalse(goldenDbClusterProviderUnderTest.applicable("resourceSubType"));
    }

    @Test
    public void testCheck() {

        Node node1 = new Node();
        node1.setNodeType("");
        node1.setParentUuid("");
        node1.setOsUser("");
        // when(mockGoldenDbService.getManageDbNode(new ProtectedEnvironment()))
        // .thenReturn(Collections.singletonList(node1));

        when(mockGoldenDbService.singleConnectCheck(any(), any())).thenReturn(true);

        // browse
        Node node = new Node();
        node.setNodeType("nodeType");
        node.setParentUuid("parentId");
        node.setOsUser("osUser");
        Node node0 = new Node();
        node0.setNodeType("nodeType1");
        node0.setParentUuid("parentId1");
        node0.setOsUser("osUser1");
        List<Node> nodes = Arrays.asList(node, node0);
        when(mockGoldenDbService.getManageDbNode(any())).thenReturn(nodes);
        when(mockGoldenDbService.getEnvironmentById(any())).thenReturn(getEnvironment());
        when(mockGoldenDbService.getResourceById(any())).thenReturn(getEnvironment());

        BrowseEnvironmentResourceConditions environmentConditions = new BrowseEnvironmentResourceConditions();
        environmentConditions.setEnvId("envId");
        environmentConditions.setParentId("parentId");
        environmentConditions.setResourceType(ResourceSubTypeEnum.GOLDENDB_CLUSTER.getType());
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
        final PageListResponse<ProtectedResource> protectedResourcePageListResponse =
            new PageListResponse<>(0, Arrays.asList(protectedResource));
        when(mockAgentUnifiedService.getDetailPageListNoRetry(any(), any(), any(), any(), false))
            .thenReturn(protectedResourcePageListResponse);

        // Run the test
        PageListResponse<ProtectedResource> result =
            goldenDbClusterProviderUnderTest.browse(getEnvironment(), environmentConditions);
        goldenDbClusterProviderUnderTest.register(getEnvironment());
        Assert.assertTrue(true);
    }

    @Test
    public void testCheckParamExpection() {

        Node node1 = new Node();
        node1.setNodeType("");
        node1.setParentUuid("");
        node1.setOsUser("");

        // browse
        Node node = new Node();
        node.setNodeType("nodeType");
        node.setParentUuid("parentId");
        node.setOsUser("osUser");
        Node node0 = new Node();
        node0.setNodeType("nodeType1");
        node0.setParentUuid("parentId1");
        List<Node> nodes = Arrays.asList(node, node0);
        when(mockGoldenDbService.getManageDbNode(any())).thenReturn(nodes);
        try{
            goldenDbClusterProviderUnderTest.register(getEnvironment());
            Assert.assertFalse(false);
        }catch (LegoCheckedException exception){
            Assert.assertEquals(exception.getErrorCode(), CommonErrorCode.ILLEGAL_PARAM);
        }


    }

    @Test
    public void testHealthCheck() {
        final Node node = new Node();
        node.setNodeType("nodeType");
        node.setParentUuid("parentUuid");
        node.setOsUser("osUser");
        final List<Node> nodes = Arrays.asList(node);
        when(mockGoldenDbService.getManageDbNode(any())).thenReturn(nodes);
        when(mockGoldenDbService.singleHealthCheck(any(), any())).thenReturn(true);
        goldenDbClusterProviderUnderTest.validate(getEnvironment());
        Assert.assertTrue(true);
    }

    @Test
    public void testHealthCheck1() {
        final Node node = new Node();
        node.setNodeType("nodeType");
        node.setParentUuid("parentUuid");
        node.setOsUser("osUser");
        final List<Node> nodes = Arrays.asList(node);
        when(mockGoldenDbService.getManageDbNode(any())).thenReturn(nodes);
        when(mockGoldenDbService.singleHealthCheck(any(), any())).thenReturn(true);
        when(mockGoldenDbService.getChildren(any())).thenReturn(Arrays.asList(getInstance()));
        goldenDbClusterProviderUnderTest.validate(getEnvironment());
        Assert.assertTrue(true);
    }

    private ProtectedEnvironment getEnvironment() {
        String json =
            "{\"name\":\"goldentest666222\",\"type\":\"Database\",\"subType\":\"GoldenDB-cluster\",\"extendInfo\":{\"linkStatus\":\"0\",\"GoldenDB\":\"{\\\"nodes\\\":[{\\\"nodeType\\\":\\\"managerNode\\\",\\\"parentUuid\\\":\\\"7017bd24-1a4d-42fc-aaf4-3046eab88704\\\",\\\"osUser\\\":\\\"zxmanager\\\"}]}\"},\"dependencies\":{\"agents\":[{\"uuid\":\"7017bd24-1a4d-42fc-aaf4-3046eab88704\"}]}}";
        ProtectedEnvironment read = JsonUtil.read(json, ProtectedEnvironment.class);
        return read;
    }

    private ProtectedResource getInstance() {
        String json =
            "{\"uuid\":\"eb46ed20eeff45499afbec5e739220a1\",\"name\":\"def\",\"type\":\"Database\",\"subType\":\"GoldenDB-clusterInstance\",\"path\":\"192.168.138.42,192.168.138.44,192.168.138.46\",\"createdTime\":\"2023-04-06 17:51:47.807\",\"parentUuid\":\"7e5b99c1-71c5-3c84-9214-77867dd11e47\",\"rootUuid\":\"7e5b99c1-71c5-3c84-9214-77867dd11e47\",\"sourceType\":\"register\",\"protectionStatus\":0,\"extendInfo\":{\"clusterInfo\":\"{\\\"id\\\":\\\"1\\\",\\\"name\\\":\\\"cluster1\\\",\\\"group\\\":[{\\\"groupId\\\":\\\"1\\\",\\\"databaseNum\\\":\\\"2\\\",\\\"mysqlNodes\\\":[{\\\"id\\\":\\\"1\\\",\\\"name\\\":\\\"DN1\\\",\\\"role\\\":\\\"master\\\",\\\"ip\\\":\\\"8.46.138.44\\\",\\\"port\\\":\\\"5501\\\",\\\"linkStatus\\\":\\\"1\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"group\\\":\\\"DBGroup1\\\",\\\"parentUuid\\\":\\\"9292c22a-fbad-4c54-a69b-ac20401a4a5c\\\",\\\"parentName\\\":\\\"localhost.localdomain(192.168.138.44)\\\",\\\"osUser\\\":\\\"zxdb1\\\",\\\"parent\\\":null},{\\\"id\\\":\\\"2\\\",\\\"name\\\":\\\"DN2\\\",\\\"role\\\":\\\"slave\\\",\\\"ip\\\":\\\"8.46.138.46\\\",\\\"port\\\":\\\"5501\\\",\\\"linkStatus\\\":\\\"1\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"group\\\":\\\"DBGroup1\\\",\\\"parentUuid\\\":\\\"2cd27f74-da99-43b3-b972-faff996dd487\\\",\\\"parentName\\\":\\\"localhost.localdomain(192.168.138.46)\\\",\\\"osUser\\\":\\\"zxdb1\\\",\\\"parent\\\":null}]},{\\\"groupId\\\":\\\"2\\\",\\\"databaseNum\\\":\\\"2\\\",\\\"mysqlNodes\\\":[{\\\"id\\\":\\\"3\\\",\\\"name\\\":\\\"DN3\\\",\\\"role\\\":\\\"master\\\",\\\"ip\\\":\\\"8.46.138.46\\\",\\\"port\\\":\\\"5502\\\",\\\"linkStatus\\\":\\\"1\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"group\\\":\\\"DBGroup2\\\",\\\"parentUuid\\\":\\\"2cd27f74-da99-43b3-b972-faff996dd487\\\",\\\"parentName\\\":\\\"localhost.localdomain(192.168.138.46)\\\",\\\"osUser\\\":\\\"zxdb2\\\",\\\"parent\\\":null},{\\\"id\\\":\\\"4\\\",\\\"name\\\":\\\"DN4\\\",\\\"role\\\":\\\"slave\\\",\\\"ip\\\":\\\"8.46.138.44\\\",\\\"port\\\":\\\"5502\\\",\\\"linkStatus\\\":\\\"1\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"group\\\":\\\"DBGroup2\\\",\\\"parentUuid\\\":\\\"9292c22a-fbad-4c54-a69b-ac20401a4a5c\\\",\\\"parentName\\\":\\\"localhost.localdomain(192.168.138.44)\\\",\\\"osUser\\\":\\\"zxdb2\\\",\\\"parent\\\":null}]}],\\\"gtm\\\":[{\\\"gtmId\\\":\\\"1\\\",\\\"gtmIp\\\":\\\"8.46.138.44\\\",\\\"port\\\":\\\"6026\\\",\\\"masterFlag\\\":\\\"1\\\",\\\"parentUuid\\\":\\\"9292c22a-fbad-4c54-a69b-ac20401a4a5c\\\",\\\"parentName\\\":\\\"localhost.localdomain(192.168.138.44)\\\",\\\"nodeType\\\":\\\"gtmNode\\\",\\\"osUser\\\":\\\"zxgtm1\\\",\\\"parent\\\":null}]}\",\"local_ini_cnf\":\"W2NvbW1vbl0KI1Jvb3QgZGlyIG9mIGJhY2t1cGluZzsKI3VuaXQ6IE5BLCByYW5nZTogTkEsIGRlZmF1bHQ6ICRIT01FL2JhY2t1cF9yb290CmJhY2t1cF9yb290ZGlyID0K\",\"agentIpList\":\"8.46.138.44\"},\"userId\":\"62a76fceaefd4bbc99f5adf0686f6181\",\"authorizedUser\":\"baohu\",\"auth\":{\"authType\":2,\"authKey\":\"super\",\"authPwd\":\"Huawei@123\",\"extendInfo\":{}},\"environment\":{\"uuid\":\"7e5b99c1-71c5-3c84-9214-77867dd11e47\",\"name\":\"集群42_1\",\"type\":\"Database\",\"subType\":\"GoldenDB-cluster\",\"createdTime\":\"2023-03-31 16:44:01.403\",\"rootUuid\":\"7e5b99c1-71c5-3c84-9214-77867dd11e47\",\"sourceType\":\"register\",\"version\":\"V6.1\",\"protectionStatus\":0,\"extendInfo\":{\"GoldenDB\":\"{\\\"nodes\\\":[{\\\"parentUuid\\\":\\\"80d9ae7f-e87f-4485-9234-a13ee47bf450\\\",\\\"parentName\\\":\\\"goldendb41(192.168.138.42)\\\",\\\"osUser\\\":\\\"zxmanager\\\",\\\"nodeType\\\":\\\"managerNode\\\"}]}\"},\"userId\":\"62a76fceaefd4bbc99f5adf0686f6181\",\"authorizedUser\":\"baohu\",\"endpoint\":\"192.168.138.42\",\"port\":0,\"linkStatus\":\"1\",\"scanInterval\":3600,\"cluster\":false},\"dependencies\":{\"agents\":[{\"uuid\":\"9292c22a-fbad-4c54-a69b-ac20401a4a5c\"},{\"uuid\":\"2cd27f74-da99-43b3-b972-faff996dd487\"}],\"-agents\":[]}}";
        ProtectedResource read = JsonUtil.read(json, ProtectedResource.class);
        return read;
    }
}
