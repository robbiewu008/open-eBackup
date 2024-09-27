package openbackup.access.framework.resource.service.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.testdata.MockEntity;

import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.plugin.DefaultPluginConfigManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
 
/**
 * 功能描述: UnifiedResourceScanProviderTest
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-08
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({UnifiedResourceScanProvider.class, EnvironmentLinkStatusHelper.class})
public class UnifiedResourceScanProviderTest {
    private static UnifiedResourceScanProvider scanProvider;
    private static AgentUnifiedService agentService;
    private static ResourceService resourceService;
    private static ProtectedEnvironmentRetrievalsService envRetrievalsService;

    @Rule
    public ExpectedException exceptionRule = ExpectedException.none();

    @BeforeClass
    public static void init() {
        PluginConfigManager pluginConfigManager = new DefaultPluginConfigManager();
        pluginConfigManager.init();
        agentService = PowerMockito.mock(AgentUnifiedService.class);
        resourceService = PowerMockito.mock(ResourceService.class);
        envRetrievalsService = PowerMockito.mock(ProtectedEnvironmentRetrievalsService.class);
        scanProvider = new UnifiedResourceScanProvider(pluginConfigManager, agentService, resourceService,
                envRetrievalsService);
    }
 
    @Test
    public void test_scan_resource_success() {
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any())).thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
        ProtectedEnvironment agent = MockEntity.mockAgentResource();
        Optional<ProtectedResource> optAgent = Optional.of(agent);
        PowerMockito.when(resourceService.getResourceById(agent.getUuid())).thenReturn(optAgent);
 
        ProtectedResource resource = new ProtectedResource();
        resource.setType("Namespace");
        resource.setSubType("KubernetesNamespace");
        resource.setName("TestNamespace");
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(1);
        response.setRecords(Collections.singletonList(resource));
        PageListResponse<ProtectedResource> emptyResponse = new PageListResponse<>(0, new ArrayList<>());
        PowerMockito.when(agentService.getDetailPageList(any(), any(), any(), any()))
                .thenReturn(response).thenReturn(emptyResponse);
        ProtectedEnvironment environment = MockEntity.mockEnvironmentWithDependency();

        Map<ProtectedResource, List<ProtectedEnvironment>> map = new HashMap<>();
        map.put(environment, Collections.singletonList(agent));
        PowerMockito.when(envRetrievalsService.collectConnectableResources(environment)).thenReturn(map);

        List<ProtectedResource> resources = scanProvider.scan(environment);
        Assert.assertEquals(1, resources.size());
        ProtectedResource result = resources.get(0);
        Assert.assertEquals("Namespace", result.getType());
        Assert.assertEquals("KubernetesNamespace", result.getSubType());
        Assert.assertEquals("TestEnv" + File.separator + "TestNamespace", resource.getPath());
        Assert.assertEquals("Test-User-Id", resource.getUserId());
        Assert.assertEquals("Test-User-Name", resource.getAuthorizedUser());
    }

    @Test
    public void test_scan_resource_fail_when_agent_is_empty() {
        exceptionRule.expect(LegoCheckedException.class);
        exceptionRule.expectMessage("Resource scan failed");
        ProtectedEnvironment environment = MockEntity.mockEnvironmentWithDependency();
        Map<ProtectedResource, List<ProtectedEnvironment>> map = new HashMap<>();
        PowerMockito.when(envRetrievalsService.collectConnectableResources(environment)).thenReturn(map);
        scanProvider.scan(environment);
    }
}