/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.saphana.protection.access.provider.backup;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.saphana.protection.access.constant.SapHanaConstants;
import openbackup.saphana.protection.access.service.SapHanaResourceService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.reflect.Whitebox;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * {@link SapHanaDatabaseBackupProvider Test}
 *
 * @author wWX1013713
 * @version [DataBackup 1.5.0]
 * @since 2023-05-19
 */
public class SapHanaDatabaseBackupProviderTest {
    private final SapHanaResourceService hanaResourceService = PowerMockito.mock(SapHanaResourceService.class);

    private final SapHanaDatabaseBackupProvider provider = new SapHanaDatabaseBackupProvider(hanaResourceService);

    /**
     * 用例场景：根据subType检测Provider
     * 前置条件：subType为SAPHANA-database
     * 检查点：返回true
     */
    @Test
    public void applicable_sap_hana_database_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.SAPHANA_DATABASE.getType()));
    }

    /**
     * 用例场景：拦截备份任务成功
     * 前置条件：备份任务参数正确
     * 检查点：填充完预期的数据
     */
    @Test
    public void intercept_backup_task_success() {
        BackupTask backupTask = new BackupTask();
        backupTask.setProtectEnv(new TaskEnvironment());
        backupTask.setTaskId("taskId");
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("uuid");
        backupTask.setProtectObject(taskResource);
        backupTask.setBackupType(DatabaseConstants.FULL_BACKUP_TYPE);
        StorageRepository repository = new StorageRepository();
        repository.setType(RepositoryTypeEnum.DATA.getType());
        List<StorageRepository> repositories = new ArrayList<>();
        repositories.add(repository);
        backupTask.setRepositories(repositories);
        String tmpNodesInfo = "[{\"endpoint\": \"127.0.0.1\",\"port\": 22},{\"endpoint\": \"127.0.0.2\",\"port\": 22}]";
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("uuid");
        resource.setParentUuid("inst_uuid");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(SapHanaConstants.NODES, tmpNodesInfo);
        resource.setExtendInfo(extendInfo);
        PowerMockito.doReturn(resource).when(hanaResourceService).getResourceById("uuid");
        PowerMockito.doReturn(mockInstResource(false)).when(hanaResourceService).getResourceById("inst_uuid");
        provider.initialize(backupTask);
        Assert.assertEquals(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat(), backupTask.getCopyFormat());
    }

    /**
     * 用例场景：设置保护环境信息成功
     * 前置条件：保护对象为系统数据库
     * 检查点：填充成功
     */
    @Test
    public void set_protect_env_success_when_type_is_ap_cluster() throws Exception {
        BackupTask backupTask = new BackupTask();
        TaskResource protectObject = new TaskResource();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(SapHanaConstants.SAP_HANA_DB_TYPE, SapHanaConstants.SYSTEM_DB_TYPE);
        protectObject.setExtendInfo(extendInfo);
        backupTask.setProtectObject(protectObject);
        backupTask.setProtectEnv(new TaskEnvironment());
        List<Endpoint> agentList = new ArrayList<>();
        agentList.add(new Endpoint("127.0.0.1", 22));
        agentList.add(new Endpoint("127.0.0.2", 22));
        Whitebox.invokeMethod(provider, "setProtectEnvExtendInfo", backupTask, agentList);
        Assert.assertEquals(DatabaseDeployTypeEnum.AP.getType(),
            backupTask.getProtectEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE));
    }

    /**
     * 用例场景：设置保护环境信息成功
     * 前置条件：保护对象为租户数据库
     * 检查点：填充成功
     */
    @Test
    public void set_protect_env_success_when_type_is_distributed_cluster() throws Exception {
        BackupTask backupTask = new BackupTask();
        TaskResource protectObject = new TaskResource();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(SapHanaConstants.SAP_HANA_DB_TYPE, SapHanaConstants.TENANT_DB_TYPE);
        protectObject.setExtendInfo(extendInfo);
        backupTask.setProtectObject(protectObject);
        backupTask.setProtectEnv(new TaskEnvironment());
        List<Endpoint> agentList = new ArrayList<>();
        agentList.add(new Endpoint("127.0.0.1", 22));
        agentList.add(new Endpoint("127.0.0.2", 22));
        Whitebox.invokeMethod(provider, "setProtectEnvExtendInfo", backupTask, agentList);
        Assert.assertEquals(DatabaseDeployTypeEnum.DISTRIBUTED.getType(),
            backupTask.getProtectEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE));
    }

    /**
     * 用例场景：备份任务设置持久仓库列表
     * 前置条件：执行日志备份，但实例未开启日志备份
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckException_if_exec_log_backup_but_inst_disabled_log_bak_when_setRepositories() {
        BackupTask backupTask = new BackupTask();
        backupTask.setBackupType(DatabaseConstants.LOG_BACKUP_TYPE);
        StorageRepository repository = new StorageRepository();
        repository.setType(RepositoryTypeEnum.DATA.getType());
        List<StorageRepository> repositories = new ArrayList<>();
        repositories.add(repository);
        backupTask.setRepositories(repositories);
        PowerMockito.doReturn(mockInstResource(false)).when(hanaResourceService).getResourceById(any());
        Assert.assertThrows(LegoCheckedException.class,
            () -> Whitebox.invokeMethod(provider, "setRepositories", backupTask, mockDbResource()));
    }

    /**
     * 用例场景：设置存储仓
     * 前置条件：实例开启了日志备份，备份类型为全量备份
     * 检查点：包含数据仓，添加日志仓和缓存仓
     */
    @Test
    public void set_repositories_success() throws Exception {
        BackupTask backupTask = new BackupTask();
        backupTask.setBackupType(DatabaseConstants.FULL_BACKUP_TYPE);
        StorageRepository repository = new StorageRepository();
        repository.setType(RepositoryTypeEnum.DATA.getType());
        List<StorageRepository> repositories = new ArrayList<>();
        repositories.add(repository);
        backupTask.setRepositories(repositories);
        PowerMockito.doReturn(mockInstResource(true)).when(hanaResourceService).getResourceById(any());
        Whitebox.invokeMethod(provider, "setRepositories", backupTask, mockDbResource());
        Assert.assertEquals(3, backupTask.getRepositories().size());
    }

    private ProtectedResource mockDbResource() {
        ProtectedResource dbResource = new ProtectedResource();
        return dbResource;
    }

    private ProtectedResource mockInstResource(boolean enableLogBak) {
        ProtectedResource instResource = new ProtectedResource();
        instResource.setExtendInfoByKey(SapHanaConstants.ENABLE_LOG_BACKUP, String.valueOf(false));
        if (enableLogBak) {
            instResource.setExtendInfoByKey(SapHanaConstants.ENABLE_LOG_BACKUP, String.valueOf(true));
        }
        return instResource;
    }
}
