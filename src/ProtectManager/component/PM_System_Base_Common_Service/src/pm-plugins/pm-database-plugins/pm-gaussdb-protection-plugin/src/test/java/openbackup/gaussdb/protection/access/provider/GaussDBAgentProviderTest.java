package openbackup.gaussdb.protection.access.provider;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.any;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.gaussdb.protection.access.constant.GaussDBConstant;
import openbackup.gaussdb.protection.access.interceptor.MockInterceptorParameter;

import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.assertj.core.util.Lists;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-29
 */
@RunWith(PowerMockRunner.class)
public class GaussDBAgentProviderTest {
    private final ProtectedEnvironmentRetrievalsService protectedResourceChecker = Mockito.mock(
        ProtectedEnvironmentRetrievalsService.class);

    private GaussDBAgentProvider gaussDBAgentProvider;

    private ResourceService resourceService;

    @Before
    public void init() {
        resourceService = PowerMockito.mock(ResourceService.class);
        gaussDBAgentProvider = new GaussDBAgentProvider(protectedResourceChecker, resourceService);
    }

    /**
     * 用例场景：备份场景查询agent成功
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void backup_select_success() {
        ProtectedResource resource = MockInterceptorParameter.getProtectedEnvironment();

        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(GaussDBConstant.GAUSSDB_AGENTS, MockInterceptorParameter.getProtectedEnvironment2());
        resource.setDependencies(dependencies);

        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .build();
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(resource));
        List<Endpoint> endpointList = gaussDBAgentProvider.getSelectedAgents(agentSelectParam);

        List<Endpoint> agents = MockInterceptorParameter.getEndpointFromProtectedEnvironment2();

        assertThat(endpointList).usingRecursiveComparison().isEqualTo(agents);
    }

    /**
     * 用例场景：回復场景查询agent成功
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void restore_select_success() {
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap = new HashMap<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("uuid");
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(8088);
        protectedEnvironment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());

        List<ProtectedEnvironment> protectedEnvironments = Lists.newArrayList(protectedEnvironment);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("1");
        protectedResourceMap.put(protectedResource, protectedEnvironments);

        Mockito.when(protectedResourceChecker.collectConnectableResources(Mockito.anyString()))
            .thenReturn(protectedResourceMap);
        ProtectedEnvironment taskEnvironment = new ProtectedEnvironment();
        taskEnvironment.setUuid("env");

        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(taskEnvironment)
            .jobType(JobTypeEnum.RESTORE.getValue())
            .build();
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(taskEnvironment));
        List<Endpoint> endpointList = gaussDBAgentProvider.getSelectedAgents(agentSelectParam);

        Endpoint endpoint = new Endpoint("uuid", "127.0.0.1", 8088);
        assertThat(endpointList).usingRecursiveComparison().isEqualTo(Lists.newArrayList(endpoint));
    }

    /**
     * 用例场景：扫描场景查询agent成功
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void scan_select_success() {
        ProtectedResource resource = MockInterceptorParameter.getProtectedEnvironment();

        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(GaussDBConstant.GAUSSDB_AGENTS, MockInterceptorParameter.getProtectedResource2());
        resource.setDependencies(dependencies);

        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.RESOURCE_SCAN.getValue())
            .build();
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(resource));
        List<Endpoint> endpointList = gaussDBAgentProvider.getSelectedAgents(agentSelectParam);
        List<Endpoint> agents = MockInterceptorParameter.getEndpointFromProtectedEnvironment2();

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
        nodeResource1.setSubType(ResourceSubTypeEnum.HCS_GAUSSDB_INSTANCE.getType());
        AgentSelectParam agentSelectParam = AgentSelectParam.builder().resource(nodeResource1).build();
        boolean applicable = gaussDBAgentProvider.applicable(agentSelectParam);
        Assert.assertTrue(applicable);
    }
}
