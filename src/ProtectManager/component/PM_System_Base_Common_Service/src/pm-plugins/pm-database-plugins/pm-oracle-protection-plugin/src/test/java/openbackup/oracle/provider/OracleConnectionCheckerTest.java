package openbackup.oracle.provider;


import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.oracle.service.OracleBaseService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class OracleConnectionCheckerTest{
    private final ResourceService resourceService = Mockito.mock(ResourceService.class);
    private final OracleBaseService oracleBaseService= Mockito.mock(OracleBaseService.class);
    private final ProtectedEnvironmentRetrievalsService retrievalsService= Mockito.mock(ProtectedEnvironmentRetrievalsService.class);
    private final AgentUnifiedService agentUnifiedService= Mockito.mock(AgentUnifiedService.class);
    private final OracleConnectionChecker checker = new OracleConnectionChecker(retrievalsService, agentUnifiedService,
            resourceService, oracleBaseService);

    /**
     * 用例场景：OracleConnectionChecker provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(checker.applicable(mockSingle()));
    }

    /**
     * 用例场景：获取受保护资源环境矩阵
     * 前置条件：资源为oracle单机
     * 检查点：environment长度为1
     */
    @Test
    public void should_return_one_when_get_collect_connectable_resources_if_resource_is_single() {
        ProtectedResource protectedResource = mockSingle();
        Map<ProtectedResource, List<ProtectedEnvironment>> resources = checker.collectConnectableResources(protectedResource);
        Assert.assertEquals(resources.get(protectedResource).size(), 1);
    }

    /**
     * 用例场景：获取受保护资源环境矩阵
     * 前置条件：资源为oracle集群数据库
     * 检查点：environment长度为2
     */
    @Test
    public void should_return_more_than_one_when_get_collect_connectable_resources_if_resource_is_cluster() {
        ProtectedResource protectedResource = mockClusterDatabase();
        Mockito.when(oracleBaseService.getOracleClusterHosts(protectedResource)).thenReturn(mockCluster());
        Map<ProtectedResource, List<ProtectedEnvironment>> resources = checker.collectConnectableResources(protectedResource);
        Assert.assertEquals(resources.get(protectedResource).size(), 2);
    }

    /**
     * 用例场景：获取检查结果并更新在线状态
     * 前置条件：连通性检查成功
     * 检查点：link status为 “1”
     */
    @Test
    public void should_update_online_when_collect_action_success() {
        ProtectedResource resource = mockSingle();
        List<ActionResult> results = checker.collectActionResults(mockSuccessReport(resource), new HashMap<>());
        Assert.assertEquals(DatabaseConstants.SUCCESS_CODE, results.get(0).getCode());
    }

    private List<CheckReport<Object>> mockSuccessReport(ProtectedResource resource) {
        List<CheckReport<Object>> res = new ArrayList<>();
        CheckReport<Object> report = new CheckReport<>();
        report.setResource(resource);
        report.setResults(new ArrayList<>());

        CheckResult<Object> checkResult = new CheckResult<>();
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(DatabaseConstants.SUCCESS_CODE);
        checkResult.setResults(actionResult);
        report.getResults().add(checkResult);

        res.add(report);
        return res;
    }

    private ProtectedResource mockSingle() {
        ProtectedResource protectedResource = new ProtectedResource();

        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setEndpoint("192.168.111.164");
        agent.setPort(59522);

        protectedResource.setUuid("b76b2924d0ca4e6cbc956c89df3b521e");
        protectedResource.setSubType(ResourceSubTypeEnum.ORACLE.getType());
        protectedResource.setEnvironment(agent);

        return protectedResource;
    }

    private ProtectedResource mockClusterDatabase() {
        ProtectedResource protectedResource = new ProtectedResource();

        ProtectedEnvironment cluster = new ProtectedEnvironment();
        cluster.setUuid("f00939749e964bbdbbca9c45dfbd2a6e");
        protectedResource.setUuid("b76b2924d0ca4e6cbc956c89df3b521e");
        protectedResource.setSubType(ResourceSubTypeEnum.ORACLE_CLUSTER.getType());
        protectedResource.setEnvironment(cluster);

        return protectedResource;
    }

    private List<ProtectedEnvironment> mockCluster() {
        List<ProtectedEnvironment> agents = new ArrayList<>();

        ProtectedEnvironment agent155 = new ProtectedEnvironment();
        agent155.setEndpoint("192.168.111.155");
        agent155.setPort(59527);

        ProtectedEnvironment agent156 = new ProtectedEnvironment();
        agent156.setEndpoint("192.168.111.156");
        agent156.setPort(59528);

        agents.add(agent155);
        agents.add(agent156);
        return agents;
    }
}