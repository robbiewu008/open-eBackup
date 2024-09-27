package openbackup.mysql.resources.access.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Ignore;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;

/**
 * {@link MysqlAgentProvider} 测试类
 *
 * @author lWX776769
 * @version [DataBackup 1.5.0]
 * @since 2023/7/27
 */
public class MysqlAgentProviderTest {
    private final MysqlBaseService mysqlBaseService = PowerMockito.mock(MysqlBaseService.class);

    private MysqlAgentProvider provider = new MysqlAgentProvider(mysqlBaseService);

    /**
     * 用例场景：mysql资源类型匹配
     * 前置条件：资源类型为mysql数据库、实例、集群实例
     * 检查点：过滤通过返回true，否则返回false
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(
            provider.applicable(getAgentSelectParam(ResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.getType())));
        Assert.assertTrue(
            provider.applicable(getAgentSelectParam(ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType())));
        Assert.assertTrue(provider.applicable(getAgentSelectParam(ResourceSubTypeEnum.MYSQL_DATABASE.getType())));
        Assert.assertFalse(provider.applicable(getAgentSelectParam(ResourceSubTypeEnum.DB2.getType())));
    }

    /**
     * 用例场景：mysql集群实例的agents获取
     * 前置条件：参数正确，是mysql集群实例
     * 检查点：参数设置正确
     */
    @Test
    public void should_return_agents_if_cluster_instance_when_execute_select() {
        PowerMockito.when(mysqlBaseService.getResource(any())).thenReturn(mockClusterInstance());
        List<Endpoint> endpointList = provider.getSelectedAgents(getAgentSelectParam(ResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.getType()));
        Assert.assertTrue(endpointList.isEmpty());
    }

    /**
     * 用例场景：mysql数据库的agents获取
     * 前置条件：参数正确，是mysql集群实例
     * 检查点：参数设置正确
     */
    @Test
    @Ignore
    public void should_return_agents_if_database_when_execute_select() {
        PowerMockito.when(mysqlBaseService.getResource(any())).thenReturn(mockClusterInstance());
        PowerMockito.when(mysqlBaseService.getAgentBySingleInstanceUuid(any())).thenReturn(new ProtectedEnvironment());
        List<Endpoint> endpointList = provider.getSelectedAgents(getAgentSelectParam(ResourceSubTypeEnum.MYSQL_DATABASE.getType()));
        Assert.assertEquals(endpointList.size(), 1);
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

    private ProtectedResource mockClusterInstance() {
        ProtectedResource clusterInstance = new ProtectedResource();
        clusterInstance.setUuid(UUIDGenerator.getUUID());
        clusterInstance.setVersion("8.0.32");
        return clusterInstance;
    }

    private List<Endpoint> mockAgents() {
        Endpoint agent = new Endpoint();
        agent.setId(UUIDGenerator.getUUID());
        agent.setIp("127.0.0.1");
        agent.setPort(59521);
        return Collections.singletonList(agent);
    }
}