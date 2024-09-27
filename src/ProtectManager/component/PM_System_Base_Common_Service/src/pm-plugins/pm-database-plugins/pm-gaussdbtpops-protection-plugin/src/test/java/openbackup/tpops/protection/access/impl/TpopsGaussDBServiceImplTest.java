/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tpops.protection.access.impl;

import openbackup.access.framework.resource.service.provider.UnifiedClusterResourceIntegrityChecker;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;

import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.tpops.protection.access.constant.TpopsGaussDBConstant;
import openbackup.tpops.protection.access.service.TpopsGaussDBService;
import openbackup.tpops.protection.access.service.impl.TpopsGaussDBServiceImpl;

import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 实现类测试
 *
 * @author x30021699
 * @since 2023-05-09
 */
@RunWith(SpringRunner.class)
@PrepareForTest(TpopsGaussDBService.class)
@SpringBootTest(classes = {EnvironmentLinkStatusHelper.class})
public class TpopsGaussDBServiceImplTest {
    @Mock
    private ResourceService resourceService;

    @Mock
    private ProviderManager providerManager;

    @MockBean
    private MemberClusterService memberClusterService;

    @Mock
    private ResourceConnectionCheckProvider resourceConnectionCheckProvider;

    @Mock
    private UnifiedClusterResourceIntegrityChecker clusterIntegrityChecker;

    @Mock
    private TaskRepositoryManager taskRepositoryManager;

    @Mock
    private AgentUnifiedService agentUnifiedService;

    private TpopsGaussDBServiceImpl tpopsGaussDBServiceImpl;

    @Before
    public void setUp() {
        tpopsGaussDBServiceImpl = new TpopsGaussDBServiceImpl(resourceService, providerManager,
            resourceConnectionCheckProvider, clusterIntegrityChecker, taskRepositoryManager);
    }

    /**
     * 用例场景：查询注册资源
     * 前置条件：已有资源
     * 检  查  点：查询成功
     */
    @Test
    public void testGetAppEnvResponse() {
        ProtectedEnvironment protectedResource = new ProtectedEnvironment();
        protectedResource.setUuid("fc963582-3750-4dce-acf6-ce828a7355ab");
        protectedResource.setExtendInfoByKey("status", "OFFLINE");
        protectedResource.setSubType(ResourceSubTypeEnum.HCS_GAUSSDB_PROJECT.getType());
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.any()))
            .thenReturn(Optional.of(protectedResource));
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setSubType(ResourceSubTypeEnum.HCS_GAUSSDB_PROJECT.getType());
        CheckResult<AppEnvResponse> checkResult = new CheckResult<>();
        checkResult.setData(appEnvResponse);
        PowerMockito.when(clusterIntegrityChecker.generateCheckResult(ArgumentMatchers.any())).thenReturn(checkResult);
        AppEnvResponse response = tpopsGaussDBServiceImpl.getAppEnvResponse(Lists.newArrayList(protectedResource),
            protectedResource);
        Assert.assertNotNull(response);
    }

    /**
     * 用例场景：查询资源
     * 前置条件：已有资源
     * 检  查  点：查询成功
     */
    @Test
    public void testGetExistingGaussDbResources() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("fc963582-3750-4dce-acf6-ce828a7355ab");
        protectedResource.setExtendInfoByKey("status", "OFFLINE");
        protectedResource.setSubType(ResourceSubTypeEnum.HCS_GAUSSDB_INSTANCE.getType());
        protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
        PowerMockito.when(
                resourceService.query(ArgumentMatchers.anyInt(), ArgumentMatchers.anyInt(), ArgumentMatchers.anyMap()))
            .thenReturn(new PageListResponse<>(1, Collections.singletonList(protectedResource)));
        List<ProtectedResource> existingGaussDbResources = tpopsGaussDBServiceImpl.getExistingGaussDbResources(
            ResourceSubTypeEnum.HCS_GAUSSDB_INSTANCE.getType(), new HashMap<>());
        Assert.assertEquals("OFFLINE", existingGaussDbResources.get(0).getExtendInfoByKey("status"));
    }

    /**
     * 用例场景：更新資源状态
     * 前置条件：已有资源
     * 检  查  点：更新成功
     */
    @Test
    public void testUpdateResourceLinkStatus() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("fc963582-3750-4dce-acf6-ce828a7355ab");
        protectedResource.setExtendInfoByKey("status", "OFFLINE");
        protectedResource.setSubType(ResourceSubTypeEnum.HCS_GAUSSDB_INSTANCE.getType());
        protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
        tpopsGaussDBServiceImpl.updateResourceLinkStatus("fc963582-3750-4dce-acf6-ce828a7355ab", "ONLINE");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：参数设置，目录格式
     * 前置条件：无
     * 检  查  点：设置副本状态，目录格式
     */
    @Test
    public void testModifyBackupTaskParam() {
        BackupTask backupTask = new BackupTask();
        StorageRepository repository = new StorageRepository();
        repository.setType(RepositoryTypeEnum.DATA.getType());
        ArrayList<StorageRepository> repositories = new ArrayList<>();
        repositories.add(repository);
        backupTask.setRepositories(repositories);
        TaskResource taskResource = new TaskResource();
        taskResource.setRootUuid("123");
        taskResource.setExtendInfo(new HashMap<>());
        backupTask.setProtectObject(taskResource);

        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setExtendInfo(new HashMap<>());
        TaskEnvironment node = new TaskEnvironment();
        node.setName("NODE");
        taskEnvironment.setNodes(Collections.singletonList(node));
        backupTask.setProtectEnv(taskEnvironment);
        ProtectedEnvironment protectedResource = new ProtectedEnvironment();
        protectedResource.setName("test");
        Authentication authentication = new Authentication();
        authentication.setAuthKey("key");
        protectedResource.setAuth(authentication);
        protectedResource.setExtendInfo(new HashMap<>());
        List<ProtectedResource> resources = new ArrayList<>();
        protectedResource.setUuid("22222");
        resources.add(protectedResource);
        Map<String, List<ProtectedResource>> resourceMap = new HashMap<>();
        resourceMap.put(TpopsGaussDBConstant.GAUSSDB_AGENTS, resources);
        protectedResource.setDependencies(resourceMap);
        backupTask.setAdvanceParams((Map<String, String>) new HashMap<>().put("56", "67"));
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.any()))
            .thenReturn(Optional.of(protectedResource));
        tpopsGaussDBServiceImpl.modifyBackupTaskParam(backupTask);
        Assert.assertEquals(1, backupTask.getCopyFormat());
    }

    /**
     * 用例场景：参数设置，设置日志仓库，检查成功
     * 前置条件：无
     * 检  查  点：设置日志仓库，检查成功
     */
    @Test
    public void testModifyBackupTaskParamLOG() {
        BackupTask backupTask = new BackupTask();
        backupTask.setBackupType(DatabaseConstants.LOG_BACKUP_TYPE);
        StorageRepository repository = new StorageRepository();
        repository.setType(RepositoryTypeEnum.DATA.getType());
        ArrayList<StorageRepository> repositories = new ArrayList<>();
        repositories.add(repository);
        backupTask.setRepositories(repositories);
        TaskResource taskResource = new TaskResource();
        taskResource.setRootUuid("123");
        taskResource.setExtendInfo(new HashMap<>());
        backupTask.setProtectObject(taskResource);

        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setExtendInfo(new HashMap<>());
        TaskEnvironment node = new TaskEnvironment();
        node.setName("NODE");
        taskEnvironment.setNodes(Collections.singletonList(node));
        backupTask.setProtectEnv(taskEnvironment);
        ProtectedEnvironment protectedResource = new ProtectedEnvironment();
        protectedResource.setName("test");
        Authentication authentication = new Authentication();
        authentication.setAuthKey("key");
        protectedResource.setAuth(authentication);
        protectedResource.setExtendInfo(new HashMap<>());
        List<ProtectedResource> resources = new ArrayList<>();
        protectedResource.setUuid("22222");
        resources.add(protectedResource);
        Map<String, List<ProtectedResource>> resourceMap = new HashMap<>();
        resourceMap.put(TpopsGaussDBConstant.GAUSSDB_AGENTS, resources);
        protectedResource.setDependencies(resourceMap);
        backupTask.setAdvanceParams((Map<String, String>) new HashMap<>().put("56", "67"));
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.any()))
            .thenReturn(Optional.of(protectedResource));
        tpopsGaussDBServiceImpl.modifyBackupTaskParam(backupTask);
        Integer type = backupTask.getRepositories().get(0).getType();
        Assert.assertEquals(1, type.intValue());
    }

    /**
     * 用例场景：参数设置
     * 前置条件：无
     * 检  查  点：节点通过
     */
    @Test
    public void testSupplyNodes() {
        ProtectedEnvironment protectedResource = new ProtectedEnvironment();
        protectedResource.setName("test");
        Authentication authentication = new Authentication();
        authentication.setAuthKey("key");
        protectedResource.setAuth(authentication);
        protectedResource.setExtendInfo(new HashMap<>());
        List<ProtectedResource> resources = new ArrayList<>();
        protectedResource.setUuid("22222");
        resources.add(protectedResource);
        Map<String, List<ProtectedResource>> resourceMap = new HashMap<>();
        resourceMap.put(TpopsGaussDBConstant.GAUSSDB_AGENTS, resources);
        protectedResource.setDependencies(resourceMap);
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.any()))
            .thenReturn(Optional.of(protectedResource));
        List<TaskEnvironment> taskEnvironments = tpopsGaussDBServiceImpl.supplyNodes("123");
        Assert.assertTrue(true);
    }
}