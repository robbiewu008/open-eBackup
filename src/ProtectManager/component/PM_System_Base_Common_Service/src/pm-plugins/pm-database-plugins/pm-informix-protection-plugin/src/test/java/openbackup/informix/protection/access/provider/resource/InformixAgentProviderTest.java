package openbackup.informix.protection.access.provider.resource;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.informix.protection.access.service.InformixService;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;

/**
 * {@link InformixAgentProvider 测试类}
 *
 * @author dwx1009286
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-25
 */
public class InformixAgentProviderTest {
    private final InformixService informixService = PowerMockito.mock(InformixService.class);
    private final InformixAgentProvider agentProvider = new InformixAgentProvider(informixService);

    /**
     * 用例场景：informix填充Agent成功
     * 前置条件：资源类型为informix（Informix-singleInstance）
     * 检查点：填充成功
     */
    @Test
    public void select_agent_success() {
        AgentSelectParam agentSelectParam = getAgentSelectParam();
        PowerMockito.when(informixService.getAgentsByInstanceResource(any())).thenReturn(new ArrayList<>());
        Assert.assertNotNull(agentProvider.getSelectedAgents(agentSelectParam));
    }

    /**
     * 用例场景：当为集群的时候，填充Agent成功
     * 前置条件：资源类型为Informix单节点（Informix-singleInstance）
     * 检查点：填充成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(agentProvider.applicable(getAgentSelectParam()));
    }

    private AgentSelectParam getAgentSelectParam() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.getType());
        return AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .parameters(new HashMap<>())
            .build();
    }
}
