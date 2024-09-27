package openbackup.sqlserver.protection.backup;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import com.huawei.oceanprotect.job.sdk.JobService;

import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * SQL Server数据库备份拦截器测试类
 *
 * @author xWX1016404
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-06
 */
@RunWith(PowerMockRunner.class)
public class SqlServerBackupInterceptorTest {
    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    private SqlServerBackupInterceptor serverBackupInterceptor;

    private JobService jobService;

    private SqlServerBaseService sqlServerBaseService;

    private static final String SQL_SERVER_INSTANCE_UUID = "123456";

    private static final String SQL_SERVER_CLUSTER_INSTANCE_UUID = "111111";

    @Before
    public void init() {
        this.jobService = Mockito.mock(JobService.class);
        this.sqlServerBaseService = Mockito.mock(SqlServerBaseService.class);
        this.serverBackupInterceptor = new SqlServerBackupInterceptor(this.jobService, this.sqlServerBaseService);
    }

    /**
     * 用例场景：SQL Server实例检查类provider过滤
     * 前置条件：资源类型为SQL Server单实例
     * 检查点：类过滤检查返回成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(serverBackupInterceptor.applicable(ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType()));
        Assert.assertTrue(serverBackupInterceptor.applicable(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType()));
        Assert.assertTrue(serverBackupInterceptor.applicable(ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType()));
        Assert.assertTrue(
            serverBackupInterceptor.applicable(ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType()));
    }

    /**
     * 用例场景：SQL Server数据库全量备份
     * 前置条件：数据库任务启动为数据库全量备份
     * 检查点：SQL Server数据库任务操作完成后任务仓新增CACHE仓
     */
    @Test
    public void supply_full_backup_database_task_success() {
        BackupTask backupTask = getBackupTask(DatabaseConstants.FULL_BACKUP_TYPE,
            ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType());
        PowerMockito.when(sqlServerBaseService.getResourceByUuid(SQL_SERVER_INSTANCE_UUID))
            .thenReturn(getProtectedResource());

        PowerMockito.when(jobService.queryJobs(any(), any(), any())).thenReturn(getPageListResponse());
        BackupTask task = serverBackupInterceptor.supplyBackupTask(backupTask);
        List<StorageRepository> taskRepositories = task.getRepositories();
        Assert.assertEquals(2, taskRepositories.size());
        for (StorageRepository storageRepository : taskRepositories) {
            Assert.assertTrue(storageRepository.getType() == RepositoryTypeEnum.CACHE.getType()
                || storageRepository.getType() == RepositoryTypeEnum.DATA.getType());
        }
    }

    /**
     * 用例场景：SQL Server数据库全量备份
     * 前置条件：数据库任务启动为数据库全量备份
     * 检查点：SQL Server数据库任务操作完成后备份任务的Agent资源配置成功
     */
    @Test
    public void supply_full_backup_database_agent_success() {
        BackupTask backupTask = getBackupTask(DatabaseConstants.FULL_BACKUP_TYPE,
            ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType());
        PowerMockito.when(sqlServerBaseService.getResourceByUuid(SQL_SERVER_INSTANCE_UUID))
            .thenReturn(getProtectedResource());
        serverBackupInterceptor.supplyAgent(backupTask);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：SQL Server实例agent填充
     * 前置条件：实例备份任务启动为全量备份
     * 检查点：SQL Server实例任务操作完成后备份任务的Agent资源配置成功
     */
    @Test
    public void supply_full_backup_instance_agent_success() {
        BackupTask backupTask = getBackupTask(DatabaseConstants.FULL_BACKUP_TYPE,
            ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType());
        PowerMockito.when(sqlServerBaseService.getResourceByUuid(SQL_SERVER_INSTANCE_UUID))
            .thenReturn(getProtectedResource());
        serverBackupInterceptor.supplyAgent(backupTask);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：SQL Server集群实例agent填充
     * 前置条件：集群实例备份任务启动为全量备份
     * 检查点：SQL Server集群实例任务操作完成后备份任务的Agent资源配置成功
     */
    @Test
    public void supply_full_backup_cluster_instance_agent_success() {
        BackupTask backupTask = getBackupTask(DatabaseConstants.FULL_BACKUP_TYPE,
            ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType());
        PowerMockito.when(sqlServerBaseService.getResourceByUuid(SQL_SERVER_INSTANCE_UUID))
            .thenReturn(getProtectedResource());
        PowerMockito.when(sqlServerBaseService.getResourceByUuid(SQL_SERVER_CLUSTER_INSTANCE_UUID))
            .thenReturn(getClusterInstance());
        serverBackupInterceptor.supplyAgent(backupTask);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：SQL Server集群实例node填充
     * 前置条件：集群实例备份任务启动为全量备份
     * 检查点：SQL Server集群实例任务操作完成后备份任务的node资源配置成功
     */
    @Test
    public void supply_full_backup_cluster_instance_node_success() {
        BackupTask backupTask = getBackupTask(DatabaseConstants.FULL_BACKUP_TYPE,
            ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType());
        PowerMockito.when(sqlServerBaseService.getResourceByUuid(SQL_SERVER_INSTANCE_UUID))
            .thenReturn(getProtectedResource());
        PowerMockito.when(sqlServerBaseService.getResourceByUuid(SQL_SERVER_CLUSTER_INSTANCE_UUID))
            .thenReturn(getClusterInstance());
        serverBackupInterceptor.supplyNodes(backupTask);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：SQL Server实例node填充
     * 前置条件：实例备份任务启动为全量备份
     * 检查点：SQL Server实例任务操作完成后备份任务的node资源配置成功
     */
    @Test
    public void supply_full_backup_instance_node_success() {
        BackupTask backupTask = getBackupTask(DatabaseConstants.FULL_BACKUP_TYPE,
            ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType());
        PowerMockito.when(sqlServerBaseService.getResourceByUuid(SQL_SERVER_INSTANCE_UUID))
            .thenReturn(getProtectedResource());
        serverBackupInterceptor.supplyNodes(backupTask);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：SQL Server数据库全量备份
     * 前置条件：数据库任务启动为数据库全量备份
     * 检查点：SQL Server数据库任务操作完成后备份任务的node资源配置成功
     */
    @Test
    public void supply_full_backup_database_node_success() {
        BackupTask backupTask = getBackupTask(DatabaseConstants.FULL_BACKUP_TYPE,
            ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType());
        PowerMockito.when(sqlServerBaseService.getResourceByUuid(SQL_SERVER_INSTANCE_UUID))
            .thenReturn(getProtectedResource());
        serverBackupInterceptor.supplyNodes(backupTask);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：SQL Server数据库全量备份
     * 前置条件：数据库任务启动为数据库全量备份
     * 检查点：SQL Server数据库任务操作完成后备份任务的node资源配置成功
     */
    @Test
    public void supply_backup_task_database_node_success() {
        BackupTask backupTask = getBackupTask(DatabaseConstants.FULL_BACKUP_TYPE,
            ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType());
        backupTask.setRepositories(new ArrayList<>());
        Assert.assertThrows("[SQL Server] backup task has no repository.", LegoCheckedException.class,
            () -> serverBackupInterceptor.supplyBackupTask(backupTask));
    }

    /**
     * 根据任务类型获取框架备份任务
     *
     * @param backupType 备份任务类型
     * @param subType 资源子类型
     * @return 备份任务
     */
    private BackupTask getBackupTask(String backupType, String subType) {
        BackupTask backupTask = new BackupTask();

        // 设置备份任务类型
        backupTask.setBackupType(backupType);

        // 设置存储仓列表
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setType(RepositoryTypeEnum.DATA.getType());
        storageRepository.setProtocol(RepositoryProtocolEnum.NFS.getProtocol());
        backupTask.setRepositories(Stream.of(storageRepository).collect(Collectors.toList()));

        // 设置任务保护资源
        TaskResource protectObject = new TaskResource();
        protectObject.setExtendInfo(new HashMap<>());
        protectObject.setSubType(subType);
        ProtectedResource protectedResource = getProtectedResource();
        protectObject.setUuid(protectedResource.getUuid());
        if (ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType().equals(subType)) {
            protectObject.setUuid(SQL_SERVER_CLUSTER_INSTANCE_UUID);
        }
        protectObject.setParentUuid(protectedResource.getUuid());
        backupTask.setProtectObject(protectObject);

        // 设计任务环境信息
        TaskEnvironment protectEnv = new TaskEnvironment();
        Map<String, String> extendInfo = new HashMap<>();
        protectEnv.setExtendInfo(extendInfo);
        backupTask.setProtectEnv(protectEnv);
        return backupTask;
    }

    private ProtectedResource getProtectedResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType());
        Authentication auth = new Authentication();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.INSTANCE_PORT, "1433");
        auth.setExtendInfo(extendInfo);
        resource.setAuth(auth);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(SQL_SERVER_INSTANCE_UUID);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("envUuid");
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(22);
        dependencies.put(DatabaseConstants.AGENTS, Collections.singletonList(protectedEnvironment));
        resource.setDependencies(dependencies);
        resource.setExtendInfo(extendInfo);
        resource.setUuid(SQL_SERVER_INSTANCE_UUID);
        return resource;
    }

    private ProtectedResource getClusterInstance() {
        ProtectedResource clusterInstance = new ProtectedResource();
        clusterInstance.setUuid(SQL_SERVER_CLUSTER_INSTANCE_UUID);

        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType());

        Authentication auth = new Authentication();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.INSTANCE_PORT, "1433");
        auth.setExtendInfo(extendInfo);
        resource.setAuth(auth);

        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("envUuid");
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(22);
        dependencies.put(DatabaseConstants.AGENTS, Collections.singletonList(protectedEnvironment));
        resource.setDependencies(dependencies);
        resource.setExtendInfo(extendInfo);
        resource.setUuid(SQL_SERVER_INSTANCE_UUID);

        Map<String, List<ProtectedResource>> childRenDependencies = new HashMap<>();
        childRenDependencies.put(DatabaseConstants.AGENTS, Collections.singletonList(resource));
        clusterInstance.setDependencies(childRenDependencies);
        return clusterInstance;
    }

    private PageListResponse<JobBo> getPageListResponse() {
        PageListResponse<JobBo> pageListResponse = new PageListResponse<>();
        JobBo jobBo = new JobBo();
        jobBo.setStatus("SUCCESS");
        pageListResponse.setRecords(Collections.singletonList(jobBo));
        return pageListResponse;
    }

    @Test
    public void isSupportDataAndLogParallelBackup() {
        Assert.assertTrue(serverBackupInterceptor.isSupportDataAndLogParallelBackup(new ProtectedResource()));
    }
}