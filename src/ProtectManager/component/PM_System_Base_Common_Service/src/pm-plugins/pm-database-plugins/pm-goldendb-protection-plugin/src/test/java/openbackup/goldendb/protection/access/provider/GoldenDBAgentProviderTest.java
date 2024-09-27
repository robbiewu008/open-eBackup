package openbackup.goldendb.protection.access.provider;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.when;

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.goldendb.protection.access.service.GoldenDbService;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.assertj.core.util.Lists;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-29
 */
@RunWith(PowerMockRunner.class)
public class GoldenDBAgentProviderTest {
    @Mock
    private GoldenDbService mockGoldenDbService;

    private GoldenDBAgentProvider goldenDBAgentProvider;

    @Before
    public void setUp() {
        goldenDBAgentProvider = new GoldenDBAgentProvider(mockGoldenDbService);
    }

    /**
     * 用例场景：备份场景查询agent成功
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void backup_select_success() {
        // 设置dependencies
        HashMap<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setUuid("11111");
        resource.setEndpoint("10.10.10.10");
        resource.setPort(123);
        list.add(resource);

        ProtectedEnvironment resource2 = new ProtectedEnvironment();
        resource2.setUuid("22222");
        resource2.setEndpoint("10.10.10.11");
        resource2.setPort(124);
        list.add(resource2);
        dependencies.put("agents", list);

        // 设置mock返回值
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid("uuid");
        environment.setRootUuid("00000");
        environment.setEndpoint("8.8.8.8");
        environment.setPort(66);
        environment.setDependencies(dependencies);

        when(mockGoldenDbService.getResourceById(anyString())).thenReturn(environment);
        when(mockGoldenDbService.getEnvironmentById(anyString())).thenReturn(environment);

        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(environment)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .build();

        List<Endpoint> endpointList = goldenDBAgentProvider.getSelectedAgents(agentSelectParam);
        Endpoint endpoint1 = new Endpoint("11111", "10.10.10.10", 123);
        Endpoint endpoint2 = new Endpoint("22222", "10.10.10.11", 124);

        List<Endpoint> agents = Lists.newArrayList(endpoint1, endpoint2, endpoint1, endpoint2);

        assertThat(endpointList).usingRecursiveComparison().isEqualTo(agents);
    }

    /**
     * 用例场景：过滤
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        ProtectedResource nodeResource1 = new ProtectedResource();
        nodeResource1.setSubType(ResourceSubTypeEnum.GOLDENDB_CLUSETER_INSTANCE.getType());
        AgentSelectParam agentSelectParam = AgentSelectParam.builder().resource(nodeResource1).build();
        boolean applicable = goldenDBAgentProvider.applicable(agentSelectParam);
        Assert.assertTrue(applicable);
    }
}
