package openbackup.mysql.resources.access.interceptor;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.enums.MysqlResourceSubTypeEnum;
import openbackup.mysql.resources.access.enums.MysqlRoleEnum;
import openbackup.mysql.resources.access.service.MysqlBaseService;

import com.google.common.collect.ImmutableMap;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * MySQL数据库备份拦截器实现类 测试类
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/5/31
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({MysqlDbBackupInterceptor.class})
public class MysqlDbBackupInterceptorTest {
    @InjectMocks
    private MysqlDbBackupInterceptor mysqlDbBackupInterceptor;

    @Mock
    private MysqlBaseService mysqlBaseService;

    private final AgentUnifiedService agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);

    @Before
    public void setup() {
        mysqlDbBackupInterceptor = new MysqlDbBackupInterceptor(mysqlBaseService, agentUnifiedService);
    }

    /**
     * 用例场景：下发全量备份时，经过应用拦截器后的备份任务
     * 前置条件：1. 全量备份
     * 检 查 点：1. 检查仓库信息是否正确
     */
    @Test
    public void supply_BackupTask_success_if_job_is_full_backup_job() {
        BackupTask fullBackupTask = getBackupTask("fullBackup", MysqlResourceSubTypeEnum.MYSQL_DATABASE);
        ProtectedResource protectedResource = mockGetResource(fullBackupTask);
        BackupTask suppliedBackupTask = mysqlDbBackupInterceptor.supplyBackupTask(fullBackupTask);
        assert suppliedBackupTask.getRepositories().size() == 2;
        assert RepositoryTypeEnum.DATA.getType() == suppliedBackupTask.getRepositories().get(0).getType();
        assert RepositoryTypeEnum.CACHE.getType() == suppliedBackupTask.getRepositories().get(1).getType();
        assert fullBackupTask.getProtectObject().getAuth().getAuthKey().equals(protectedResource.getAuth().getAuthKey());
    }

    /**
     * 用例场景：下发全量备份时，经过应用拦截器后的备份任务
     * 前置条件：1. mysql资源是数据库
     * 检 查 点：1. 不报错
     */
    @Test
    public void supply_nodes_success_if_mysql_is_database() {
        BackupTask task = getBackupTask("fullBackup", MysqlResourceSubTypeEnum.MYSQL_DATABASE);
        ProtectedResource protectedResource = new ProtectedResource();
        String instanceIp = "8.40.99.111";
        protectedResource.setExtendInfoByKey(MysqlConstants.INSTANCE_IP, instanceIp);
        PowerMockito.when(mysqlBaseService.getResource(task.getProtectObject().getParentUuid())).thenReturn(protectedResource);
        mysqlDbBackupInterceptor.supplyNodes(task);
        Assert.assertEquals(instanceIp, task.getProtectObject().getExtendInfo().get(MysqlConstants.INSTANCE_IP));
    }

    /**
     * 用例场景：下发全量备份时，连通性检查
     * 前置条件：1. 连通性检查通过
     * 检 查 点：1. 不报错 2. 可用的agent数量正确
     */
    @Test
    public void check_connection_success() {
        BackupTask task = getBackupTask("fullBackup", MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE);
        Endpoint endpoint = new Endpoint();
        endpoint.setIp("8.40.100.120");
        List<Endpoint> endpoints = Arrays.asList(endpoint);
        task.setAgents(endpoints);

        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("123");
        List<ProtectedResource> protectedResources = Arrays.asList(protectedResource);
        PowerMockito.when(mysqlBaseService.getSingleInstanceByClusterInstance(any())).thenReturn(protectedResources);

        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setEndpoint("8.40.100.120");
        PowerMockito.when(mysqlBaseService.getAgentBySingleInstanceUuid(any())).thenReturn(protectedEnvironment);

        AppEnvResponse appEnvResponse = new AppEnvResponse();
        Map<String, String> ex = new HashMap<>();
        ex.put("status", "0");
        appEnvResponse.setExtendInfo(ex);
        PowerMockito.when(agentUnifiedService.getClusterInfo(any(), any())).thenReturn(appEnvResponse);
        mysqlDbBackupInterceptor.checkConnention(task);
        Assert.assertEquals(1, task.getAgents().size());
    }

    @Test
    public void check_connection_for_eapp() {
        BackupTask task = getBackupTask("fullBackup", MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE);
        task.getProtectEnv().getExtendInfo().put(DatabaseConstants.CLUSTER_TYPE, MysqlConstants.EAPP);
        mysqlDbBackupInterceptor.checkConnention(task);
        Assert.assertNotNull(mysqlDbBackupInterceptor);
    }

    /**
     * 用例场景：下发全量备份时，经过应用拦截器后的备份任务
     * 前置条件：1. mysql资源是数据库
     * 检 查 点：1. 不报错
     */
    @Test
    public void supply_nodes_success_if_mysql_is_cluster_instance() {
        BackupTask task = getBackupTask("fullBackup", MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE);
        List<ProtectedResource> singleInstanceResources = new ArrayList<>();
        PowerMockito.when(mysqlBaseService.getSingleInstanceByClusterInstance(any())).thenReturn(singleInstanceResources);
        mysqlDbBackupInterceptor.supplyNodes(task);
        Mockito.verify(mysqlBaseService, Mockito.times(1)).getSingleInstanceByClusterInstance(any());
    }

    /**
     * 用例场景：下发日志备份时，经过应用拦截器后的备份任务
     * 前置条件：1. 日志备份
     * 检 查 点：1. 检查仓库信息是否正确
     */
    @Test
    public void supply_BackupTask_success_if_job_is_log_backup_job() {
        BackupTask fullBackupTask = getBackupTask("logBackup", MysqlResourceSubTypeEnum.MYSQL_DATABASE);

        ProtectedResource protectedResource = mockGetResource(fullBackupTask);

        BackupTask suppliedBackupTask = mysqlDbBackupInterceptor.supplyBackupTask(fullBackupTask);
        assert suppliedBackupTask.getRepositories().size() == 2;
        assert RepositoryTypeEnum.LOG.getType() == suppliedBackupTask.getRepositories().get(0).getType();
        assert RepositoryTypeEnum.CACHE.getType() == suppliedBackupTask.getRepositories().get(1).getType();
        assert DatabaseDeployTypeEnum.SINGLE.getType().equals(fullBackupTask.getProtectEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE));
        assert fullBackupTask.getProtectObject().getAuth().getAuthKey().equals(protectedResource.getAuth().getAuthKey());
    }

    @Test
    public void supply_BackupTask_for_eapp() {
        BackupTask fullBackupTask = getBackupTask("fullBackup", MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE);
        fullBackupTask.getProtectEnv().getExtendInfo().put(DatabaseConstants.CLUSTER_TYPE, MysqlConstants.EAPP);
        mockGetResource(fullBackupTask);
        BackupTask suppliedBackupTask = mysqlDbBackupInterceptor.supplyBackupTask(fullBackupTask);
        Assert.assertEquals(DatabaseDeployTypeEnum.SHARDING.getType(),
            suppliedBackupTask.getProtectEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE));
    }

    /**
     * 用例场景：下发数据库资源的备份任务时，经过应用拦截器后的备份任务
     * 前置条件：1. 日志备份
     * 检 查 点：1. 检查仓库信息是否正确
     */
    @Test
    public void supply_Agent_success_if_job_is_database_backup_job() {
        BackupTask fullBackupTask = getBackupTask("logBackup", MysqlResourceSubTypeEnum.MYSQL_DATABASE);

        ProtectedResource protectedResource = new ProtectedResource();
        HashMap<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.CLUSTER_TYPE,MysqlConstants.PXC);
        protectedResource.setExtendInfo(extendInfo);
        PowerMockito.when(mysqlBaseService.getResource(any())).thenReturn(protectedResource);

        ProtectedEnvironment agentEnv = new ProtectedEnvironment();
        agentEnv.setEndpoint("8.40.99.101");
        agentEnv.setPort(8081);
        PowerMockito.when(mysqlBaseService.getAgentBySingleInstanceUuid(fullBackupTask.getProtectObject().getParentUuid()))
                .thenReturn(agentEnv);

        Endpoint endpoint = new Endpoint();
        endpoint.setPort(agentEnv.getPort());
        endpoint.setIp(agentEnv.getEndpoint());
        PowerMockito.when(mysqlBaseService.getAgentEndpoint(agentEnv)).thenReturn(endpoint);

        mysqlDbBackupInterceptor.supplyAgent(fullBackupTask);
        Assert.assertEquals(1, fullBackupTask.getAgents().size());
        Assert.assertEquals(endpoint.getPort(), fullBackupTask.getAgents().get(0).getPort());
        Assert.assertEquals(endpoint.getIp(), fullBackupTask.getAgents().get(0).getIp());
    }

    /**
     * 用例场景：下发数据库资源的备份任务时，经过应用拦截器后的备份任务
     * 前置条件：1. 日志备份
     * 检 查 点：1. 检查仓库信息是否正确
     */
    @Test
    public void supply_Agent_success_if_job_is_cluster_instance_backup_job() {
        BackupTask fullBackupTask = getBackupTask("logBackup", MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE);
        ProtectedResource protectedResourceOne = new ProtectedResource();
        protectedResourceOne.setUuid(UUID.randomUUID().toString());
        HashMap<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.ROLE, "1");
        protectedResourceOne.setExtendInfo(extendInfo);
        List<ProtectedResource> singleInstanceResources = Arrays.asList(protectedResourceOne);

        PowerMockito.when(mysqlBaseService.getSingleInstanceByClusterInstance(fullBackupTask.getProtectObject().getUuid())).thenReturn(singleInstanceResources);

        ProtectedEnvironment agentEnv = new ProtectedEnvironment();
        agentEnv.setEndpoint("8.40.99.101");
        agentEnv.setPort(8081);

        Endpoint endpoint = new Endpoint();
        endpoint.setPort(agentEnv.getPort());
        endpoint.setIp(agentEnv.getEndpoint());
        PowerMockito.when(mysqlBaseService.getResource(fullBackupTask.getProtectObject().getUuid())).thenReturn(new ProtectedResource());
        PowerMockito.when(mysqlBaseService.supplyExtendInfo(any(), any())).thenReturn(new HashMap<>());
        PowerMockito.when(mysqlBaseService.getAgentEndpoint(agentEnv)).thenReturn(endpoint);

        PowerMockito.when(mysqlBaseService.getAgentBySingleInstanceUuid(protectedResourceOne.getUuid())).thenReturn(agentEnv);
        mysqlDbBackupInterceptor.supplyAgent(fullBackupTask);
        Assert.assertEquals(1, fullBackupTask.getAgents().size());
        Assert.assertEquals((int) agentEnv.getPort(), fullBackupTask.getAgents().get(0).getPort());
        Assert.assertEquals(agentEnv.getEndpoint(), fullBackupTask.getAgents().get(0).getIp());
    }

    /**
     * 用例场景：下发集群实例资源的备份任务时，经过应用拦截器后的备份任务
     * 前置条件：1. 日志备份 2. agent不是主节点
     * 检 查 点：1. agent是空的
     */
    @Test
    public void supply_Agent_success_if_job_is_cluster_instance_backup_job_and_no_master_agent() {
        BackupTask fullBackupTask = getBackupTask("logBackup", MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE);
        ProtectedResource protectedResourceOne = new ProtectedResource();
        protectedResourceOne.setUuid(UUID.randomUUID().toString());
        final Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.ROLE, MysqlRoleEnum.SLAVE.getRole());
        protectedResourceOne.setExtendInfo(extendInfo);
        List<ProtectedResource> singleInstanceResources = Arrays.asList(protectedResourceOne);

        PowerMockito.when(mysqlBaseService.getSingleInstanceByClusterInstance(fullBackupTask.getProtectObject().getUuid())).thenReturn(singleInstanceResources);

        ProtectedEnvironment agentEnv = new ProtectedEnvironment();
        agentEnv.setEndpoint("8.40.99.101");
        agentEnv.setPort(8081);

        Endpoint endpoint = new Endpoint();
        endpoint.setPort(agentEnv.getPort());
        endpoint.setIp(agentEnv.getEndpoint());
        PowerMockito.when(mysqlBaseService.getResource(fullBackupTask.getProtectObject().getUuid())).thenReturn(new ProtectedResource());
        PowerMockito.when(mysqlBaseService.supplyExtendInfo(any(), any())).thenReturn(new HashMap<>());
        PowerMockito.when(mysqlBaseService.getAgentEndpoint(agentEnv)).thenReturn(endpoint);

        PowerMockito.when(mysqlBaseService.getAgentBySingleInstanceUuid(protectedResourceOne.getUuid())).thenReturn(agentEnv);
        mysqlDbBackupInterceptor.supplyAgent(fullBackupTask);
        Assert.assertEquals(0, fullBackupTask.getAgents().size());
    }

    private BackupTask getBackupTask(String backupType, MysqlResourceSubTypeEnum subType) {
        BackupTask backupTask = new BackupTask();
        backupTask.setBackupType(backupType);
        List<StorageRepository> repositories = new ArrayList<>();
        StorageRepository dataStorageRepository = new StorageRepository();
        dataStorageRepository.setType(RepositoryTypeEnum.DATA.getType());
        repositories.add(dataStorageRepository);
        backupTask.setRepositories(repositories);

        TaskResource objectTask = new TaskResource();
        objectTask.setUuid(UUID.randomUUID().toString());
        objectTask.setSubType(subType.getType());
        objectTask.setParentUuid("111");
        backupTask.setProtectObject(objectTask);

        TaskEnvironment taskEnvironment = new TaskEnvironment();
        Map<String, String> taskEnvironmentExtendInfo = new HashMap<>();
        taskEnvironment.setExtendInfo(taskEnvironmentExtendInfo);
        backupTask.setProtectEnv(taskEnvironment);
        return backupTask;
    }

    private ProtectedResource mockGetResource(BackupTask backupTask) {
        ProtectedResource protectedResource = new ProtectedResource();
        Authentication authentication = new Authentication();
        authentication.setAuthKey(UUID.randomUUID().toString());
        protectedResource.setAuth(authentication);
        protectedResource.setExtendInfo(ImmutableMap.of("charset","utf8mb4"));
        PowerMockito.when(mysqlBaseService.getResource(backupTask.getProtectObject().getParentUuid())).thenReturn(protectedResource);
        return protectedResource;
    }

    @Test
    public void isSupportDataAndLogParallelBackup() {
        Assert.assertTrue(mysqlDbBackupInterceptor.isSupportDataAndLogParallelBackup(new ProtectedResource()));
    }
}
