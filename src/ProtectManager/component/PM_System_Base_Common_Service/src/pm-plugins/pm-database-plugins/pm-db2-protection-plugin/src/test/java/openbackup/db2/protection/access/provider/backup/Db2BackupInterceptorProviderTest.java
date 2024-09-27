/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.db2.protection.access.provider.backup;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.protection.service.SanClientService;
import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.backup.v2.PostBackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.service.InstanceProtectionService;
import openbackup.db2.protection.access.constant.Db2Constants;
import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;
import openbackup.db2.protection.access.service.Db2Service;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.reflect.Whitebox;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * {@link Db2BackupInterceptorProvider} 测试类
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-01-06
 */
public class Db2BackupInterceptorProviderTest {
    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final InstanceProtectionService instanceProtectionService = PowerMockito.mock(
        InstanceProtectionService.class);

    private final Db2Service db2Service = PowerMockito.mock(Db2Service.class);

    private final JobService jobService = PowerMockito.mock(JobService.class);

    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private SanClientService sanClientService = PowerMockito.mock(SanClientService.class);

    private ProtectedEnvironmentService protectedEnvironmentService = PowerMockito.mock(ProtectedEnvironmentService.class);

    private Db2BackupInterceptorProvider db2BackupInterceptorProvider = new Db2BackupInterceptorProvider(
        resourceService, instanceProtectionService, db2Service, jobService, copyRestApi);

    @Before
    public void init() {
        db2BackupInterceptorProvider.setSanClientService(sanClientService);
        db2BackupInterceptorProvider.setProtectedEnvironmentService(protectedEnvironmentService);
    }

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入资源
     * 检查点：是否返回true
     */
    @Test
    public void applicable_db2_cluster_instance_provider_success() {
        Assert.assertTrue(db2BackupInterceptorProvider.applicable(ResourceSubTypeEnum.DB2_DATABASE.getType()));
        Assert.assertTrue(db2BackupInterceptorProvider.applicable(ResourceSubTypeEnum.DB2_TABLESPACE.getType()));
    }

    /**
     * 用例场景：备份环境设置存储仓库
     * 前置条件：全量备份
     * 检查点：是否设置data、meta、cache仓库
     */
    @Test
    public void should_return_data_cache_repository_if_full_backup() {
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.DB2_TABLESPACE.getType());
        backupTask.setBackupType(DatabaseConstants.FULL_BACKUP_TYPE);
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.ofNullable(mockResource()));
        BackupTask returnTask = db2BackupInterceptorProvider.initialize(backupTask);
        Assert.assertEquals(String.valueOf(RepositoryTypeEnum.DATA.getType()),
            returnTask.getRepositories().get(IsmNumberConstant.ZERO).getType().toString());
        Assert.assertEquals(String.valueOf(RepositoryTypeEnum.META.getType()),
            returnTask.getRepositories().get(IsmNumberConstant.ONE).getType().toString());
        Assert.assertEquals(String.valueOf(RepositoryTypeEnum.CACHE.getType()),
            returnTask.getRepositories().get(IsmNumberConstant.TWO).getType().toString());
    }

    /**
     * 用例场景：备份环境设置存储仓库
     * 前置条件：日志备份
     * 检查点：是否设置data、log、meta、cache仓库
     */
    @Test
    public void should_return_data_log_cache_repository_if_log_backup() {
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.DB2_DATABASE.getType());
        backupTask.setBackupType(DatabaseConstants.LOG_BACKUP_TYPE);
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.ofNullable(mockResource()));
        BackupTask returnTask = db2BackupInterceptorProvider.initialize(backupTask);
        Assert.assertEquals(String.valueOf(RepositoryTypeEnum.DATA.getType()),
            returnTask.getRepositories().get(IsmNumberConstant.ZERO).getType().toString());
        Assert.assertEquals(String.valueOf(RepositoryTypeEnum.LOG.getType()),
            returnTask.getRepositories().get(IsmNumberConstant.ONE).getType().toString());
        Assert.assertEquals(String.valueOf(RepositoryTypeEnum.META.getType()),
            returnTask.getRepositories().get(IsmNumberConstant.TWO).getType().toString());
        Assert.assertEquals(String.valueOf(RepositoryTypeEnum.CACHE.getType()),
            returnTask.getRepositories().get(IsmNumberConstant.THREE).getType().toString());
    }

    /**
     * 用例场景：备份环境设置部署类型
     * 前置条件：保护对象的环境是DPF
     * 检查点：是否返回AP部署
     */
    @Test
    public void should_return_single_deploy_if_single_instance() {
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.DB2_DATABASE.getType());
        backupTask.getProtectEnv()
            .getExtendInfo()
            .put(DatabaseConstants.CLUSTER_TYPE, Db2ClusterTypeEnum.DPF.getType());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.ofNullable(mockResource()));
        BackupTask returnTask = db2BackupInterceptorProvider.initialize(backupTask);
        Assert.assertEquals(DatabaseDeployTypeEnum.AP.getType(),
            returnTask.getProtectEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE));
    }

    /**
     * 用例场景：备份环境设置部署类型
     * 前置条件：保护对象为环境伟powerHA集群
     * 检查点：是否返回共享部署
     */
    @Test
    public void should_return_AP_deploy_if_cluster_instance() {
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.DB2_DATABASE.getType());
        backupTask.getProtectEnv()
            .getExtendInfo()
            .put(DatabaseConstants.CLUSTER_TYPE, Db2ClusterTypeEnum.POWER_HA.getType());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.ofNullable(mockResource()));
        BackupTask returnTask = db2BackupInterceptorProvider.initialize(backupTask);
        Assert.assertEquals(DatabaseDeployTypeEnum.SHARDING.getType(),
            returnTask.getProtectEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE));
    }

    /**
     * 用例场景：增量差异备份交叉转全量
     * 前置条件：增量差异备份交叉
     * 检查点：是否返回全量备份
     */
    @Test
    public void should_return_full_backup_if_incr_and_diff_backup_intersect_when_get_backup_type() {
        PowerMockito.when(jobService.queryJobs(any(), any(), any())).thenReturn(mockPageListResponse());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.ofNullable(mockResource()));
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.DB2_DATABASE.getType());
        backupTask.setBackupType("diffBackup");
        BackupTask resultTask = db2BackupInterceptorProvider.initialize(backupTask);
        Assert.assertEquals("fullBackup", resultTask.getBackupType());
    }

    /**
     * 用例场景：备份后置处理
     * 前置条件：hadr数据库备份
     * 检查点：无异常抛出
     */
    @Test
    public void execute_backup_post_process_success() {
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(mockResource()));
        db2BackupInterceptorProvider.finalize(mockPostBackupTask());
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：lanfree备份时候设置保护对象数据量大小
     * 前置条件：lanfree备份
     * 检查点：是否设置datasize值
     */
    @Test
    public void should_return_datasize_if_scan_client_backup_when_if_fill_datasize() {
        PowerMockito.when(sanClientService.getAgentsNotConfiguredSanclient(any())).thenReturn(new String[0]);
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(mockResource()));
        PowerMockito.when(db2Service.queryDatabaseSize(any())).thenReturn("10");
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.DB2_DATABASE.getType());
        db2BackupInterceptorProvider.initialize(backupTask);
        Assert.assertEquals("10", backupTask.getProtectObject().getExtendInfo().get(Db2Constants.DATA_SIZE_KEY));
    }

    /**
     * 用例场景：备份时检查是否有离线的agent
     * 前置条件：有离线的agent
     * 检查点：抛出异常抛出
     */
    @Test
    public void should_throwLegoCheckedException_when_has_offline_agents() {
        BackupTask backupTask = mockBackupTask(ResourceSubTypeEnum.DB2_CLUSTER_INSTANCE.getType());
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        HashMap<String, String> map = new HashMap<>();
        map.put(DatabaseConstants.CLUSTER_TYPE, Db2ClusterTypeEnum.DPF.getType());
        taskEnvironment.setExtendInfo(map);
        backupTask.setProtectEnv(taskEnvironment);
        Endpoint endpoint = new Endpoint();
        endpoint.setId("agentId");
        backupTask.setAgents(Collections.singletonList(endpoint));
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById(anyString())).thenReturn(environment);
        Assert.assertThrows(LegoCheckedException.class,
            () -> Whitebox.invokeMethod(db2BackupInterceptorProvider, "checkAgentsStatus", backupTask));
    }

    private PostBackupTask mockPostBackupTask() {
        ProtectedObject protectedObject = new ProtectedObject();
        protectedObject.setResourceId(UUIDGenerator.getUUID());
        PostBackupTask postBackupTask = new PostBackupTask();
        postBackupTask.setProtectedObject(protectedObject);
        return postBackupTask;
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
        backupTask.setAdvanceParams(new HashMap<>());
        return backupTask;
    }

    private ProtectedResource mockResource() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(50000);
        List<ProtectedResource> agents = new ArrayList<>();
        agents.add(protectedEnvironment);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(DatabaseConstants.AGENTS, agents);
        ProtectedResource resource = new ProtectedResource();
        resource.setDependencies(dependencies);
        return resource;
    }

    private PageListResponse<JobBo> mockPageListResponse() {
        JobBo jobBoOne = new JobBo();
        jobBoOne.setExtendStr("{\"backupType\":\"difference_increment\"}");
        jobBoOne.setStartTime(123L);
        JobBo jobBoTwo = new JobBo();
        jobBoTwo.setExtendStr("{\"backupType\":\"full\"}");
        jobBoTwo.setStartTime(100L);
        List<JobBo> resourceJobs = new ArrayList<>();
        resourceJobs.add(jobBoOne);
        resourceJobs.add(jobBoTwo);
        PageListResponse<JobBo> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(resourceJobs);
        return pageListResponse;
    }

    @Test
    public void isSupportDataAndLogParallelBackup() {
        Assert.assertTrue(db2BackupInterceptorProvider.isSupportDataAndLogParallelBackup(new ProtectedResource()));
    }
}