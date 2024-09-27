package openbackup.kingbase.protection.access.provider.backup;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.kingbase.protection.access.service.KingBaseService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * {@link KingBaseBackupInterceptorProvider} 测试类
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-22
 */
public class KingBaseBackupInterceptorProviderTest {
    private final static String endpoint = "8.40.99.125";

    private final static Integer port = 54321;

    private final KingBaseService kingBaseService = PowerMockito.mock(KingBaseService.class);

    private final KingBaseBackupInterceptorProvider provider = new KingBaseBackupInterceptorProvider(kingBaseService);

    @Before
    public void before() {
        PowerMockito.when(kingBaseService.getResourceById(any())).thenReturn(mockResource());
        PowerMockito.when(kingBaseService.getEnvNodesByInstanceResource(any())).thenReturn(mockNodes());
    }

    /**
     * 用例场景：KingBase适配器
     * 前置条件：输入资源类型
     * 检查点：是否返回true
     */
    @Test
    public void applicable_kingbase_backup_interceptor_provider_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType()));
        Assert.assertFalse(provider.applicable(ResourceSubTypeEnum.GAUSSDBT.getType()));
    }

    /**
     * 用例场景：备份环境设置部署类型
     * 前置条件：保护对象为单实例
     * 检查点：是否返回单机部署
     */
    @Test
    public void should_return_single_deploy_if_single_instance() {
        PowerMockito.when(kingBaseService.getDeployType(eq(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType())))
            .thenReturn(DatabaseDeployTypeEnum.SINGLE.getType());
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType());
        BackupTask returnTask = provider.initialize(backupTask);
        Assert.assertEquals(DatabaseDeployTypeEnum.SINGLE.getType(),
            returnTask.getProtectEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE));
    }

    /**
     * 用例场景：备份环境设置部署类型
     * 前置条件：保护对象为主备集群实例
     * 检查点：是否返回主备部署
     */
    @Test
    public void should_return_AP_deploy_if_cluster_instance() {
        PowerMockito.when(kingBaseService.getResourceById(any())).thenReturn(mockClusterResource());
        PowerMockito.when(kingBaseService.getDeployType(eq(ResourceSubTypeEnum.KING_BASE_CLUSTER_INSTANCE.getType())))
            .thenReturn(DatabaseDeployTypeEnum.AP.getType());
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.KING_BASE_CLUSTER_INSTANCE.getType());
        BackupTask returnTask = provider.initialize(backupTask);
        Assert.assertEquals(DatabaseDeployTypeEnum.AP.getType(),
            returnTask.getProtectEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE));
    }

    /**
     * 用例场景：备份环境设置存储仓库
     * 前置条件：全量备份
     * 检查点：是否设置data和cache仓库
     */
    @Test
    public void should_return_data_cache_repository_if_full_backup() {
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType());
        backupTask.setBackupType(DatabaseConstants.FULL_BACKUP_TYPE);
        BackupTask returnTask = provider.initialize(backupTask);
        Assert.assertEquals(String.valueOf(RepositoryTypeEnum.DATA.getType()),
            returnTask.getRepositories().get(IsmNumberConstant.ZERO).getType().toString());
        Assert.assertEquals(String.valueOf(RepositoryTypeEnum.CACHE.getType()),
            returnTask.getRepositories().get(IsmNumberConstant.ONE).getType().toString());
    }

    /**
     * 用例场景：备份环境设置节点信息
     * 前置条件：无
     * 检查点：节点信息是否设置
     */
    @Test
    public void execute_build_nodes_success() {
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType());
        BackupTask returnTask = provider.initialize(backupTask);
        Assert.assertEquals(IsmNumberConstant.ONE, returnTask.getProtectEnv().getNodes().size());
    }

    /**
     * 用例场景：备份环境设置集群节点信息
     * 前置条件：集群实例
     * 检查点：节点信息是否设置
     */
    @Test
    public void when_cluster_instance_execute_build_nodes_success() {
        PowerMockito.when(kingBaseService.getResourceById(any())).thenReturn(mockClusterResource());
        PowerMockito.when(kingBaseService.getEnvNodesByInstanceResource(any())).thenReturn(mockClusterNodes());
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.KING_BASE_CLUSTER_INSTANCE.getType());
        BackupTask returnTask = provider.initialize(backupTask);
        Assert.assertEquals(IsmNumberConstant.TWO, returnTask.getProtectEnv().getNodes().size());
    }

    /**
     * 用例场景：备份环境设置agents信息
     * 前置条件：无
     * 检查点：agents信息是否设置
     */
    @Test
    public void execute_build_agent_success() {
        PowerMockito.when(kingBaseService.getAgentsByInstanceResource(any())).thenReturn(mockSingleAgents());
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType());
        BackupTask returnTask = provider.initialize(backupTask);
        Assert.assertEquals(IsmNumberConstant.ONE, returnTask.getAgents().size());
    }

    /**
     * 用例场景：备份环境设置存储仓库
     * 前置条件：日志备份
     * 检查点：是否设置data、log和cache仓库
     */
    @Test
    public void should_return_data_log_cache_repository_if_log_backup() {
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType());
        backupTask.setBackupType(DatabaseConstants.LOG_BACKUP_TYPE);
        BackupTask returnTask = provider.initialize(backupTask);
        Assert.assertEquals(String.valueOf(RepositoryTypeEnum.LOG.getType()),
            returnTask.getRepositories().get(IsmNumberConstant.ZERO).getType().toString());
        Assert.assertEquals(String.valueOf(RepositoryTypeEnum.CACHE.getType()),
            returnTask.getRepositories().get(IsmNumberConstant.ONE).getType().toString());
    }

    private List<TaskEnvironment> mockClusterNodes() {
        TaskEnvironment taskEnvironmentOne = new TaskEnvironment();
        taskEnvironmentOne.setUuid(UUID.randomUUID().toString());
        taskEnvironmentOne.setEndpoint(endpoint);
        taskEnvironmentOne.setPort(port);
        TaskEnvironment taskEnvironmentTwo = BeanTools.copy(taskEnvironmentOne, TaskEnvironment::new);
        List<TaskEnvironment> nodes = new ArrayList<>();
        nodes.add(taskEnvironmentOne);
        nodes.add(taskEnvironmentTwo);
        return nodes;
    }

    private BackupTask mockBackupTask(String subType) {
        BackupTask backupTask = new BackupTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setSubType(subType);
        backupTask.setProtectObject(taskResource);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid(UUID.randomUUID().toString());
        taskEnvironment.setExtendInfo(new HashMap());
        backupTask.setProtectEnv(taskEnvironment);
        StorageRepository dataRepository = new StorageRepository();
        dataRepository.setType(RepositoryTypeEnum.DATA.getType());
        backupTask.addRepository(dataRepository);
        return backupTask;
    }

    private ProtectedResource mockResource() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setEndpoint(endpoint);
        protectedEnvironment.setPort(port);
        List<ProtectedResource> agents = new ArrayList<>();
        agents.add(protectedEnvironment);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.AGENTS, agents);
        ProtectedResource resource = new ProtectedResource();
        resource.setDependencies(dependencies);
        return resource;
    }

    private ProtectedResource mockClusterResource() {
        ProtectedResource protectedResourceOne = mockResource();
        ProtectedResource protectedResourceTwo = mockResource();
        List<ProtectedResource> children = new ArrayList<>();
        children.add(protectedResourceOne);
        children.add(protectedResourceTwo);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.CHILDREN, children);
        ProtectedResource resource = new ProtectedResource();
        resource.setDependencies(dependencies);
        return resource;
    }

    private List<TaskEnvironment> mockNodes() {
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid(UUID.randomUUID().toString());
        taskEnvironment.setEndpoint(endpoint);
        taskEnvironment.setPort(port);
        List<TaskEnvironment> nodes = new ArrayList<>();
        nodes.add(taskEnvironment);
        return nodes;
    }

    private List<Endpoint> mockSingleAgents() {
        List<Endpoint> agents = new ArrayList<>();
        Endpoint agent = new Endpoint(endpoint, port);
        agents.add(agent);
        return agents;
    }
}