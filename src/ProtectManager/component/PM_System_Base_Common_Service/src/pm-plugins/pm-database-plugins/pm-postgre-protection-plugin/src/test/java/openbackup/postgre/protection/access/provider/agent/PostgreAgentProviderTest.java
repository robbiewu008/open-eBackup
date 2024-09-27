package openbackup.postgre.protection.access.provider.agent;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.postgre.protection.access.service.PostgreInstanceService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * {@link PostgreAgentProvider} 测试类
 *
 * @author lWX776769
 * @version [DataBackup 1.5.0]
 * @since 2023/7/27
 */
public class PostgreAgentProviderTest {
    private final PostgreInstanceService postgreInstanceService = PowerMockito.mock(PostgreInstanceService.class);

    private PostgreAgentProvider provider = new PostgreAgentProvider(postgreInstanceService);

    /**
     * 用例场景：Postgre资源类型匹配
     * 前置条件：资源类型为Postgre实例和Postgre集群实例
     * 检查点：过滤通过返回true，否则返回false
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(provider.applicable(getAgentSelectParam(ResourceSubTypeEnum.POSTGRE_INSTANCE.getType())));
        Assert.assertTrue(
            provider.applicable(getAgentSelectParam(ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType())));
        Assert.assertFalse(provider.applicable(getAgentSelectParam(ResourceSubTypeEnum.DB2.getType())));
    }

    /**
     * 用例场景：Postgre资源的agents获取
     * 前置条件：参数正确，是Postgre资源
     * 检查点：参数设置正确
     */
    @Test
    public void should_return_agents_when_execute_postgre_select() {
        PowerMockito.when(postgreInstanceService.getAgentsByInstanceResource(any())).thenReturn(mockAgents());
        List<Endpoint> agents = provider.getSelectedAgents(getAgentSelectParam(ResourceSubTypeEnum.POSTGRE_INSTANCE.getType()));
        Assert.assertEquals(IsmNumberConstant.ONE, agents.size());
    }

    private AgentSelectParam getAgentSelectParam(String subType) {
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid(UUIDGenerator.getUUID());
        resource.setSubType(subType);
        return AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .parameters(new HashMap<>())
            .build();
    }

    private List<Endpoint> mockAgents() {
        Endpoint endpoint = new Endpoint();
        endpoint.setId(UUIDGenerator.getUUID());
        endpoint.setIp("127.0.0.1");
        endpoint.setPort(59521);
        List<Endpoint> agents = new ArrayList<>();
        agents.add(endpoint);
        return agents;
    }
}