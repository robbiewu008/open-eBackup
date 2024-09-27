/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

package openbackup.oracle.interceptor;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyList;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.oracle.constants.OracleConstants;
import openbackup.oracle.provider.OracleAgentProvider;
import openbackup.oracle.service.OracleBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * oracle数据库备份拦截器实现类 测试类
 *
 * @version [OceanProtect DataBackup 1.3.0]
 * @author c30038333
 * @since 2023-01-05
 */
public class OracleBackupInterceptorTest {
    private final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);
    private final OracleBaseService oracleBaseService = Mockito.mock(OracleBaseService.class);
    private final CopyRestApi copyRestApi = Mockito.mock(CopyRestApi.class);
    private final EncryptorService encryptorService = Mockito.mock(EncryptorService.class);
    private final ResourceService resourceService = Mockito.mock(ResourceService.class);
    private final OracleAgentProvider oracleAgentProvider = Mockito.mock(OracleAgentProvider.class);
    private final JobService jobService = Mockito.mock(JobService.class);
    private final OracleBackupInterceptor oracleBackupInterceptor = new OracleBackupInterceptor(oracleBaseService,
            agentUnifiedService, copyRestApi, encryptorService, resourceService, oracleAgentProvider, jobService);

    /**
     * 用例场景：下发全量备份时，经过应用拦截器后的备份任务
     * 前置条件：1. 全量备份
     * 检 查 点：1. 检查仓库信息是否正确
     */
    @Test
    public void supply_fullBackup_BackupTask_success() {
        BackupTask fullBackupTask = getBackupTask("fullBackup", ResourceSubTypeEnum.ORACLE);
        fullBackupTask.getProtectEnv().setSubType(ResourceSubTypeEnum.ORACLE.getType());
        ProtectedResource protectedResource = mockGetResource(fullBackupTask);
        Mockito.when(oracleBaseService.getResource(any())).thenReturn(protectedResource);
        BackupTask suppliedBackupTask = oracleBackupInterceptor.supplyBackupTask(fullBackupTask);
        Assert.assertEquals(2, suppliedBackupTask.getRepositories().size());
        Assert.assertEquals(RepositoryTypeEnum.DATA.getType(), (int) suppliedBackupTask.getRepositories().get(0).getType());
        Assert.assertEquals(RepositoryTypeEnum.CACHE.getType(), (int) suppliedBackupTask.getRepositories().get(1).getType());
        Assert.assertEquals(fullBackupTask.getProtectObject().getAuth().getAuthKey(), protectedResource.getAuth().getAuthKey());
        Assert.assertEquals("false", suppliedBackupTask.getAdvanceParams()
                .get(OracleConstants.IS_CHECK_BACKUP_JOB_TYPE));
        Assert.assertTrue(suppliedBackupTask.getRepositories().get(0).getExtendInfo()
                .containsKey(OracleConstants.NEED_DELETE_DTREE));
    }

    /**
     * 用例场景：下发全量存储快照备份时，经过应用拦截器后的备份任务
     * 前置条件：1. 全量存储快照备份
     * 检 查 点：1. 检查任务是否能正常下发
     */
    @Test
    public void supply_fullBackup_storage_BackupTask_success() {
        BackupTask fullBackupTask = getStorageSnapshotBackupTask("fullBackup", ResourceSubTypeEnum.ORACLE);
        fullBackupTask.getProtectEnv().setSubType(ResourceSubTypeEnum.ORACLE.getType());
        ProtectedResource protectedResource = mockGetResource(fullBackupTask);
        Mockito.when(oracleBaseService.getResource(any())).thenReturn(protectedResource);
        Mockito.when(oracleBaseService.getEnvironmentById(anyString())).thenReturn(getAgentEnvironment("linux"));
        BackupTask suppliedBackupTask = oracleBackupInterceptor.supplyBackupTask(fullBackupTask);
        Assert.assertEquals(2, suppliedBackupTask.getRepositories().size());
        Assert.assertEquals(RepositoryTypeEnum.DATA.getType(), (int) suppliedBackupTask.getRepositories().get(0).getType());
        Assert.assertEquals(RepositoryTypeEnum.CACHE.getType(), (int) suppliedBackupTask.getRepositories().get(1).getType());
        Assert.assertEquals(fullBackupTask.getProtectObject().getAuth().getAuthKey(), protectedResource.getAuth().getAuthKey());
        Assert.assertEquals("false", suppliedBackupTask.getAdvanceParams()
            .get(OracleConstants.IS_CHECK_BACKUP_JOB_TYPE));
        Assert.assertTrue(suppliedBackupTask.getRepositories().get(0).getExtendInfo()
            .containsKey(OracleConstants.NEED_DELETE_DTREE));
    }

    /**
     * 用例场景：下发全量存储快照备份时，经过应用拦截器后的备份任务
     * 前置条件：1. 全量存储快照备份
     * 检 查 点：1. 检查任务是否能正常下发
     */
    @Test
    public void supply_agent_storage_BackupTask_success() {
        BackupTask fullBackupTask = getStorageSnapshotBackupTask("fullBackup", ResourceSubTypeEnum.ORACLE);
        fullBackupTask.getProtectEnv().setSubType(ResourceSubTypeEnum.ORACLE.getType());
        ProtectedResource protectedResource = mockGetResource(fullBackupTask);
        Mockito.when(oracleBaseService.getResource(any())).thenReturn(protectedResource);
        Mockito.when(oracleBaseService.getEnvironmentById(anyString())).thenReturn(getAgentEnvironment("linux"));
        Mockito.when(oracleAgentProvider.getSelectedAgents(any())).thenReturn(getEndpoints("linux"));
        oracleBackupInterceptor.supplyAgent(fullBackupTask);
        // 存储快照备份选择的agent不能为windows类型
        Mockito.when(oracleBaseService.getEnvironmentById(anyString())).thenReturn(getAgentEnvironment("windows"));
        Mockito.when(oracleAgentProvider.getSelectedAgents(any())).thenReturn(getEndpoints("windows"));
        Assert.assertThrows(LegoCheckedException.class, ()->oracleBackupInterceptor.supplyAgent(fullBackupTask));
    }

    private List<Endpoint> getEndpoints(String osType) {
        List<Endpoint> endpoints = new ArrayList<>();
        Endpoint endpoint = new Endpoint();
        endpoint.setAgentOS(osType);
        endpoints.add(endpoint);
        return endpoints;
    }

    @Test
    public void test_applicable_success() {
        Assert.assertTrue(oracleBackupInterceptor.applicable(ResourceSubTypeEnum.ORACLE.getType()));
        Assert.assertTrue(oracleBackupInterceptor.applicable(ResourceSubTypeEnum.ORACLE_CLUSTER.getType()));
    }

    /**
     * 用例场景：下发全量备份时，连通性检查
     * 前置条件：1. 连通性检查通过
     * 检 查 点：1. 不报错 2. 可用的agent数量正确
     */
    @Test
    public void check_connection_success() {
        BackupTask task = getBackupTask("fullBackup", ResourceSubTypeEnum.ORACLE_CLUSTER);
        Endpoint endpoint = new Endpoint();
        endpoint.setIp("192.168.100.106");
        List<Endpoint> endpoints = new ArrayList<>();
        endpoints.add(endpoint);
        task.setAgents(endpoints);

        ProtectedEnvironment cluster  = new ProtectedEnvironment();
        cluster.setUuid(UUID.randomUUID().toString());

        ProtectedEnvironment agent = new ProtectedEnvironment();
        cluster.setDependencies(new HashMap<>());
        cluster.getDependencies().put(DatabaseConstants.AGENTS, Collections.singletonList(agent));

        ProtectedResource clusterDatabase = new ProtectedResource();
        clusterDatabase.setParentUuid(cluster.getUuid());
        clusterDatabase.setUuid(UUID.randomUUID().toString());

        Mockito.when(oracleBaseService.getResource(any())).thenReturn(clusterDatabase);
        Mockito.when(oracleBaseService.getEnvironmentById(clusterDatabase.getParentUuid())).thenReturn(cluster);

        AgentBaseDto response = new AgentBaseDto();
        response.setErrorCode("0");
        PowerMockito.when(agentUnifiedService.checkApplication(clusterDatabase, agent)).thenReturn(response);
        oracleBackupInterceptor.checkConnention(task);
        Assert.assertEquals(1, task.getAgents().size());
    }

    /**
     * 用例场景：下发全量备份时，连通性检查
     * 前置条件：1. 所有节点都离线
     * 检 查 点：1. 抛出LegoCheckedException
     */
    @Test
    public void should_throw_LegoCheckedException_if_all_agent_offline() {
        BackupTask task = getBackupTask("fullBackup", ResourceSubTypeEnum.ORACLE_CLUSTER);
        task.setAgents(Collections.emptyList());
        ProtectedEnvironment cluster  = new ProtectedEnvironment();
        cluster.setUuid(UUID.randomUUID().toString());
        cluster.setDependencies(new HashMap<>());
        cluster.getDependencies().put(DatabaseConstants.AGENTS, Collections.emptyList());
        ProtectedResource clusterDatabase = new ProtectedResource();
        clusterDatabase.setParentUuid(cluster.getUuid());
        clusterDatabase.setUuid(UUID.randomUUID().toString());
        Mockito.when(oracleBaseService.getResource(any())).thenReturn(clusterDatabase);
        Mockito.when(oracleBaseService.getEnvironmentById(clusterDatabase.getParentUuid())).thenReturn(cluster);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> oracleBackupInterceptor.checkConnention(task));
        Assert.assertEquals(CommonErrorCode.AGENT_NETWORK_ERROR, exception.getErrorCode());
    }

    /**
     * 用例场景：下发日志备份时，经过应用拦截器后的备份任务
     * 前置条件：1. 日志备份
     * 检 查 点：1. 检查仓库信息是否正确
     */
    @Test
    public void supply_log_backup_job_BackupTask_success() {
        BackupTask fullBackupTask = getBackupTask("logBackup", ResourceSubTypeEnum.ORACLE);
        fullBackupTask.getProtectEnv().setSubType(ResourceSubTypeEnum.ORACLE.getType());
        fullBackupTask.getProtectObject().setVersion(OracleConstants.VERSION_12_1);
        ProtectedResource protectedResource = mockGetResource(fullBackupTask);
        PowerMockito.when(oracleBaseService.getResource(any())).thenReturn(protectedResource);
        BackupTask suppliedBackupTask = oracleBackupInterceptor.supplyBackupTask(fullBackupTask);
        assert suppliedBackupTask.getRepositories().size() == 2;
        assert RepositoryTypeEnum.LOG.getType() == suppliedBackupTask.getRepositories().get(0).getType();
        assert RepositoryTypeEnum.CACHE.getType() == suppliedBackupTask.getRepositories().get(1).getType();
        assert fullBackupTask.getProtectObject().getAuth().getAuthKey()
                .equals(protectedResource.getAuth().getAuthKey());
        Assert.assertEquals("true", suppliedBackupTask.getAdvanceParams()
                .get(OracleConstants.IS_CHECK_BACKUP_JOB_TYPE));
        System.out.println(suppliedBackupTask.getRepositories().get(0)
                .getExtendInfo());
        Assert.assertTrue(suppliedBackupTask.getRepositories().get(0).getExtendInfo()
                .containsKey(OracleConstants.FILE_HANDLE_BYTE_ALIGNMENT_SWITCH));
    }

    /**
     * 用例场景：下发数据库资源的备份任务时，经过应用拦截器后的备份任务
     * 前置条件：1. 日志备份
     * 检 查 点：1. 检查仓库信息是否正确
     */
    @Test
    public void supply_database_backup_job_success() {
        BackupTask fullBackupTask = getBackupTask("logBackup", ResourceSubTypeEnum.ORACLE);
        Endpoint endpoint = new Endpoint();
        endpoint.setPort(8088);
        endpoint.setIp("127.0.0.1");
        PowerMockito.when(oracleAgentProvider.getSelectedAgents(any()))
                .thenReturn(Collections.singletonList(endpoint));
        oracleBackupInterceptor.supplyAgent(fullBackupTask);
        Assert.assertEquals(1, fullBackupTask.getAgents().size());
        Assert.assertEquals(endpoint.getPort(), fullBackupTask.getAgents().get(0).getPort());
        Assert.assertEquals(endpoint.getIp(), fullBackupTask.getAgents().get(0).getIp());
    }


    /**
     * 用例场景：根据备份类型获取对应的副本格式
     * 前置条件：无
     * 检 查 点：副本格式是否正确
     */
    @Test
    public void test_obtain_copy_format_success() {
        BackupTask logBackupTack = new BackupTask();
        logBackupTack.setBackupType(DatabaseConstants.LOG_BACKUP_TYPE);
        Assert.assertEquals(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat(),
                oracleBackupInterceptor.obtainFormat(logBackupTack).getAsInt());
        BackupTask dataBackupTack = new BackupTask();
        dataBackupTack.setBackupType(DatabaseConstants.FULL_BACKUP_TYPE);
        Assert.assertEquals(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat(),
                oracleBackupInterceptor.obtainFormat(dataBackupTack).getAsInt());
    }

    /**
     * 用例场景：Windows主机下发备份任务
     * 前置条件：1. Windows主机
     * 检 查 点：1. 检查仓库协议是否为CIFS
     */
    @Test
    public void should_send_CIFS_repository_when_OSType_is_windows() {
        BackupTask fullBackupTask = getBackupTask("fullBackup", ResourceSubTypeEnum.ORACLE);
        fullBackupTask.getProtectEnv().setSubType(ResourceSubTypeEnum.ORACLE.getType());
        ProtectedResource protectedResource = mockGetResource(fullBackupTask);
        Mockito.when(oracleBaseService.getResource(any())).thenReturn(protectedResource);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setOsType("windows");
        Mockito.when(oracleBaseService.getEnvironmentById(any())).thenReturn(environment);
        Mockito.doAnswer(arguments -> {
            List<StorageRepository> argument = arguments.getArgument(0);
            argument.get(0).setProtocol(RepositoryProtocolEnum.CIFS.getProtocol());
            return null;
        }).when(oracleBaseService).repositoryAdaptsWindows(anyList(), any());
        BackupTask suppliedBackupTask = oracleBackupInterceptor.supplyBackupTask(fullBackupTask);
        Assert.assertEquals(2, suppliedBackupTask.getRepositories().size());
        Assert.assertEquals(RepositoryProtocolEnum.CIFS.getProtocol(),
                (int) suppliedBackupTask.getRepositories().get(0).getProtocol());
    }

    private BackupTask getBackupTask(String backupType, ResourceSubTypeEnum subType) {
        BackupTask backupTask = new BackupTask();
        backupTask.setBackupType(backupType);
        backupTask.setAdvanceParams(new HashMap<>());
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

    private BackupTask getStorageSnapshotBackupTask(String backupType, ResourceSubTypeEnum subType) {
        BackupTask backupTask = new BackupTask();
        backupTask.setBackupType(backupType);
        Map<String, String> advanceParams = new HashMap<>();
        advanceParams.put("storage_snapshot_flag", "true");
        advanceParams.put("agents", "agents1;agents2");
        backupTask.setAdvanceParams(advanceParams);
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

    private ProtectedEnvironment getAgentEnvironment(String osType) {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setOsType(osType);
        return environment;
    }

    private ProtectedResource mockGetResource(BackupTask backupTask) {
        ProtectedResource protectedResource = new ProtectedResource();
        Authentication authentication = new Authentication();
        authentication.setAuthKey(UUID.randomUUID().toString());
        protectedResource.setAuth(authentication);
        PowerMockito.when(oracleBaseService.getResource(backupTask.getProtectObject().getParentUuid()))
                .thenReturn(protectedResource);
        return protectedResource;
    }

    private List<ProtectedResource> getChildren(){
        List<ProtectedResource> children=new ArrayList<>();
        Endpoint endpoint = new Endpoint();
        endpoint.setIp("192.168.100.106");
        ProtectedEnvironment protectedEnvironment=new ProtectedEnvironment();
        protectedEnvironment.setEndpoint("192.168.100.106");
        protectedEnvironment.setPort(8081);
        protectedEnvironment.setUuid("1234321");
        List<ProtectedResource> agents=new ArrayList<>();
        agents.add(protectedEnvironment);

        ProtectedResource resource1 = new ProtectedResource();
        resource1.setUuid("123456");
        resource1.setDependencies(new HashMap<>());
        resource1.getDependencies().put(DatabaseConstants.AGENTS,agents);
        resource1.setEnvironment(protectedEnvironment);
        children.add(resource1);

        ProtectedResource resource2 = new ProtectedResource();
        resource2.setUuid("654321");
        resource2.setDependencies(new HashMap<>());
        resource2.getDependencies().put(DatabaseConstants.AGENTS,agents);
        resource1.setEnvironment(protectedEnvironment);
        children.add(resource2);
        return children;
    }
}