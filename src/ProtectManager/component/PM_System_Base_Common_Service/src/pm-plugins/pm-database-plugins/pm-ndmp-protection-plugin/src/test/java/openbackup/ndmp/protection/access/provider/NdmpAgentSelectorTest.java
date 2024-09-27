package openbackup.ndmp.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.ndmp.protection.access.service.NdmpService;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * NdmpAgentSelectorTest
 *
 * @author c30035089
 * @since 2024-08-30
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({EnvironmentLinkStatusHelper.class})
public class NdmpAgentSelectorTest {
    @InjectMocks
    private NdmpAgentSelector ndmpAgentSelector;

    @Mock
    private DefaultProtectAgentSelector defaultSelector;

    @Mock
    private ResourceService resourceService;

    @Mock
    private NdmpService ndmpService;

    @BeforeClass
    public static void init() {
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    @Test
    public void test_applicable_success() {
        Assert.assertTrue(ndmpAgentSelector.applicable("NDMP"));
    }

    @Test
    public void test_select_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType("NDMP-BackupSet");
        Map<String, String> parameters = new HashMap<>();
        parameters.put("copy_agent", "e8db4593-e68a-4898-90e8-26a08f79be91");

        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setUuid("e8db4593-e68a-4898-90e8-26a08f79be91");
        agent.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        agent.setEndpoint("192.168.99.11");
        agent.setPort(5955);
        PowerMockito.when(resourceService.getBasicResourceById("e8db4593-e68a-4898-90e8-26a08f79be91")).thenReturn(Optional.of(agent));

        List<Endpoint> agents = ndmpAgentSelector.select(protectedResource, parameters);
        Assert.assertEquals(1, agents.size());
        Assert.assertEquals(agents.get(0).getIp(), agent.getEndpoint());
    }
}
