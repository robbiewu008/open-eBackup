package openbackup.kingbase.protection.access.provider.resource;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.ClusterEnvironmentService;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.Ignore;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * {@link KingBaseClusterProvider} 测试类
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-22
 */
public class KingBaseClusterProviderTest {
    private final ProtectedEnvironmentService environmentService = PowerMockito.mock(ProtectedEnvironmentService.class);

    private final ClusterEnvironmentService clusterEnvironmentService = PowerMockito.mock(
        ClusterEnvironmentService.class);

    private final InstanceResourceService instanceResourceService = PowerMockito.mock(InstanceResourceService.class);

    private final KingBaseClusterProvider provider = new KingBaseClusterProvider(environmentService,
        clusterEnvironmentService, instanceResourceService);

    /**
     * 用例场景：KingBase集群适配器
     * 前置条件：输入资源类型
     * 检查点：是否返回true
     */
    @Test
    public void applicable_kingbase_backup_cluster_provider_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.KING_BASE_CLUSTER.getType()));
        Assert.assertFalse(provider.applicable(ResourceSubTypeEnum.GAUSSDBT.getType()));
    }

    /**
     * 用例场景：kingbase集群健康检查
     * 前置条件：节点在线
     * 检查点: 检查成功
     */
    @Test
    @Ignore
    public void health_check_success() {
        ProtectedEnvironment protectedEnvironment = buildEnv(LinkStatusEnum.ONLINE.getStatus().toString());
        provider.validate(protectedEnvironment);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：kingbase集群健康检查
     * 前置条件：节点离线
     * 检查点: 检查失败
     */
    @Test
    @Ignore
    public void should_throw_LegoCheckedException_if_host_offline_when_health_check() {
        ProtectedEnvironment protectedEnvironment = buildEnv(LinkStatusEnum.OFFLINE.getStatus().toString());
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> provider.validate(protectedEnvironment));
        Assert.assertEquals("Cluster host is offLine.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.HOST_OFFLINE, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：修改集群时检查kingbase集群节点信息
     * 前置条件：集群信息
     * 检查点: 无异常抛出
     */
    @Test
    public void execute_update_check_success() {
        PowerMockito.when(environmentService.getEnvironmentById(any()))
            .thenReturn(buildEnv(LinkStatusEnum.ONLINE.getStatus().toString()));
        ProtectedEnvironment protectedEnvironment = buildEnv(LinkStatusEnum.ONLINE.getStatus().toString());
        protectedEnvironment.setUuid(UUID.randomUUID().toString());
        provider.register(protectedEnvironment);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：创建集群时检查kingbase集群节点信息
     * 前置条件：集群信息
     * 检查点: 无异常抛出
     */
    @Test
    public void execute_register_check_success() {
        PowerMockito.when(environmentService.getEnvironmentById(any()))
            .thenReturn(buildEnv(LinkStatusEnum.ONLINE.getStatus().toString()));
        ProtectedEnvironment protectedEnvironment = buildEnv(LinkStatusEnum.ONLINE.getStatus().toString());
        provider.register(protectedEnvironment);
        Assert.assertTrue(true);
    }

    private ProtectedEnvironment buildEnv(String status) {
        ProtectedEnvironment host = new ProtectedEnvironment();
        host.setUuid(UUIDGenerator.getUUID());
        host.setLinkStatus(status);
        ProtectedEnvironment hostTwo = BeanTools.copy(host, ProtectedEnvironment::new);
        hostTwo.setUuid(UUIDGenerator.getUUID());
        List<ProtectedResource> resources = new ArrayList<>();
        resources.add(host);
        resources.add(hostTwo);
        Map<String, List<ProtectedResource>> agents = new HashMap<>();
        agents.put(DatabaseConstants.AGENTS, resources);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setDependencies(agents);
        return environment;
    }
}