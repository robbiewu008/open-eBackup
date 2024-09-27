package openbackup.goldendb.protection.access.provider;

import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.goldendb.protection.access.constant.GoldenDbConstant;
import openbackup.goldendb.protection.access.dto.instance.GoldenInstance;
import openbackup.goldendb.protection.access.dto.instance.Gtm;
import openbackup.goldendb.protection.access.dto.instance.MysqlNode;
import openbackup.goldendb.protection.access.service.GoldenDbService;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

@RunWith(MockitoJUnitRunner.class)
public class GoldenDbClusterInstanceConnectionCheckerTest {

    @Mock
    private ProtectedEnvironmentRetrievalsService mockEnvironmentRetrievalsService;

    @Mock
    private AgentUnifiedService mockAgentUnifiedService;

    @Mock
    private GoldenDbService mockGoldenDbService;

    @Mock
    private GoldenDbClusterConnectionChecker goldenDbClusterConnectionChecker;

    private GoldenDbClusterInstanceConnectionChecker goldenDbClusterInstanceConnectionCheckerUnderTest;

    @Before
    public void setUp() {
        goldenDbClusterInstanceConnectionCheckerUnderTest =
            new GoldenDbClusterInstanceConnectionChecker(mockEnvironmentRetrievalsService, mockAgentUnifiedService,
                mockGoldenDbService, goldenDbClusterConnectionChecker);
    }

    @Test
    public void testApplicable() {
        // Setup
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.GOLDENDB_CLUSETER_INSTANCE.getType());

        // Run the test
        boolean result = goldenDbClusterInstanceConnectionCheckerUnderTest.applicable(resource);

        // Verify the results
        assertTrue(result);
    }

    @Test
    public void testCollectConnectableResources() {
        String clusterInfo = getEnvironment().getExtendInfo().get("clusterInfo");
        GoldenInstance instance = JsonUtil.read(clusterInfo, GoldenInstance.class);
        List<Gtm> gtms = instance.getGtm();
        LinkedList<MysqlNode> mysqlNodes = new LinkedList<>();
        instance.getGroup().forEach(group -> {
            mysqlNodes.addAll(group.getMysqlNodes());
        });
        when(mockGoldenDbService.getComputeNode(any())).thenReturn(mysqlNodes);
        when(mockGoldenDbService.getGtmNode(any())).thenReturn(gtms);

        // Run the test
        final Map<ProtectedResource, List<ProtectedEnvironment>> result =
            goldenDbClusterInstanceConnectionCheckerUnderTest.collectConnectableResources(getEnvironment());

    }

    @Test
    public void testCollectActionResults() {
        ActionResult commonResult = new ActionResult();
        commonResult.setCode(0);
        commonResult.setMessage("success");
        ActionResult pluginResult = new ActionResult();
        pluginResult.setCode(GoldenDbConstant.NODE_TYPE_MISMATCH);
        pluginResult.setMessage("{\"errorCode\": 1577209938, \"parameters\": [\"zxdb3,dataNode,220a0393-1783-49ef-be3a-8434325ff778\"], \"errorMessage\": null}");
        List<CheckReport<Object>> checkReport = new ArrayList<>();
        CheckReport<Object> objectCheckReport = new CheckReport<>();
        List<CheckResult<Object>> list = new ArrayList<>();
        CheckResult<Object> checkResult_1 = new CheckResult<>();
        checkResult_1.setResults(commonResult);
        list.add(checkResult_1);
        CheckResult<Object> checkResult_2 = new CheckResult<>();
        checkResult_2.setResults(pluginResult);
        list.add(checkResult_2);
        objectCheckReport.setResults(list);
        checkReport.add(objectCheckReport);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.8.8.8");
        PowerMockito.when(mockGoldenDbService.getResourceById(any())).thenReturn(environment);
        List<ActionResult> results = goldenDbClusterInstanceConnectionCheckerUnderTest.collectActionResults(
            checkReport, new HashMap<>());
        Assert.assertEquals(results.size(),2);
        System.out.println(results);
    }

    private ProtectedEnvironment getEnvironment() {
        String json =
            "{\"parentUuid\":\"5a9e688f541c4eb7a5017406c21839eb\",\"name\":\"cluster3\",\"type\":\"Database\",\"subType\":\"GoldenDB-clusterInstance\",\"auth\":{\"authType\":2,\"authKey\":\"super\",\"authPwd\":\"Huawei@123\"},\"port\":\"\",\"hostName\":\"\",\"ip\":\"\",\"extendInfo\":{\"linkStatus\":\"0\",\"clusterInfo\":\"{\\\"id\\\":\\\"3\\\",\\\"name\\\":\\\"cluster3\\\",\\\"group\\\":[{\\\"groupId\\\":\\\"1\\\",\\\"mysqlNodes\\\":[{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"5\\\",\\\"name\\\":\\\"DN5\\\",\\\"role\\\":\\\"master\\\",\\\"ip\\\":\\\"8.40.162.216\\\",\\\"port\\\":\\\"5504\\\",\\\"osUser\\\":\\\"zxdb3\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"3884a310-db2b-4265-bcc4-26c6c1abb064\\\"},{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"6\\\",\\\"name\\\":\\\"DN6\\\",\\\"role\\\":\\\"slave\\\",\\\"ip\\\":\\\"8.40.162.217\\\",\\\"port\\\":\\\"5503\\\",\\\"osUser\\\":\\\"zxdb4\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"8bc55739-8811-4b64-abac-35a49486a14c\\\"}]},{\\\"groupId\\\":\\\"2\\\",\\\"databaseNum\\\":\\\"2\\\",\\\"mysqlNodes\\\":[{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"7\\\",\\\"name\\\":\\\"DN5\\\",\\\"role\\\":\\\"slave\\\",\\\"ip\\\":\\\"8.40.162.216\\\",\\\"port\\\":\\\"5503\\\",\\\"osUser\\\":\\\"zxdb5\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"3884a310-db2b-4265-bcc4-26c6c1abb064\\\"},{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"8\\\",\\\"name\\\":\\\"DN6\\\",\\\"role\\\":\\\"master\\\",\\\"ip\\\":\\\"8.40.162.217\\\",\\\"port\\\":\\\"5504\\\",\\\"osUser\\\":\\\"zxdb6\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"8bc55739-8811-4b64-abac-35a49486a14c\\\"}]}],\\\"gtm\\\":[{\\\"nodeType\\\":\\\"gtmNode\\\",\\\"parentUuid\\\":\\\"3884a310-db2b-4265-bcc4-26c6c1abb064\\\",\\\"osUser\\\":\\\"zxgtm1\\\"},{\\\"nodeType\\\":\\\"gtmNode\\\",\\\"parentUuid\\\":\\\"8bc55739-8811-4b64-abac-35a49486a14c\\\",\\\"osUser\\\":\\\"zxgtm1\\\"}]}\"},\"dependencies\":{\"agents\":[{\"uuid\":\"8bc55739-8811-4b64-abac-35a49486a14c\"},{\"uuid\":\"3884a310-db2b-4265-bcc4-26c6c1abb064\"}]}}";
        ProtectedEnvironment read = JsonUtil.read(json, ProtectedEnvironment.class);
        return read;
    }
}
