package openbackup.access.framework.resource.service.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.access.framework.resource.testdata.MockEntity;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Optional;

/**
 * 功能描述: 通用健康检查测试代码
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-23
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({UnifiedHealthCheckProvider.class,EnvironmentLinkStatusHelper.class})
public class UnifiedHealthCheckProviderTest {
    private static AgentUnifiedService agentService;
    private static ResourceService resourceService;
    private static UnifiedHealthCheckProvider healthCheckProvider;

    @BeforeClass
    public static void init() {
        agentService = PowerMockito.mock(AgentUnifiedService.class);
        resourceService = PowerMockito.mock(ResourceService.class);
        healthCheckProvider = new UnifiedHealthCheckProvider(resourceService, agentService);
    }

    @Before
    public void before(){
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any())).thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    @Test
    public void test_health_check_success() {
        ProtectedEnvironment agent = MockEntity.mockAgentResource();
        Optional<ProtectedResource> optAgent = Optional.of(agent);
        PowerMockito.when(resourceService.getResourceById(agent.getUuid())).thenReturn(optAgent);

        AgentBaseDto response = new AgentBaseDto();
        response.setErrorCode("0");
        PowerMockito.when(agentService.checkApplication(any(), any())).thenReturn(response);

        ProtectedEnvironment environment = MockEntity.mockEnvironmentWithDependency();
        healthCheckProvider.healthCheck(environment);
        Mockito.verify(agentService,Mockito.atLeastOnce()).checkApplication(any(), any());
    }

    @Test
    public void test_env_should_offline_when_health_check_failed() {
        ProtectedEnvironment agent = MockEntity.mockAgentResource();
        Optional<ProtectedResource> optAgent = Optional.of(agent);
        PowerMockito.when(resourceService.getResourceById(agent.getUuid())).thenReturn(optAgent);

        AgentBaseDto response = new AgentBaseDto();
        response.setErrorCode("1");;
        PowerMockito.when(agentService.checkApplication(any(), any())).thenReturn(response);

        ProtectedEnvironment environment = MockEntity.mockEnvironmentWithDependency();
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> healthCheckProvider.healthCheck(environment));
        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, exception.getErrorCode());
    }
}