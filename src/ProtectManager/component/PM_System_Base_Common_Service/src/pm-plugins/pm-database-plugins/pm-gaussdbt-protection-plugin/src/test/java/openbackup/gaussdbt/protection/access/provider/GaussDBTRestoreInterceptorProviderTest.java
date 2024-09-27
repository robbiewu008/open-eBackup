/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.gaussdbt.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTConstant;
import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.reflect.Whitebox;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * GaussDBT恢复拦截器Provider测试类
 *
 * @author mwx776342
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-16
 */
public class GaussDBTRestoreInterceptorProviderTest {
    private final ProviderManager providerManager = PowerMockito.mock(ProviderManager.class);

    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final ResourceConnectionCheckProvider resourceConnectionCheckProvider = PowerMockito.mock(
        ResourceConnectionCheckProvider.class);

    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private final SlaQueryService slaQueryService = PowerMockito.mock(SlaQueryService.class);

    private final GaussDBTRestoreInterceptorProvider gaussDBTRestoreInterceptorProvider
        = new GaussDBTRestoreInterceptorProvider(providerManager, resourceService, copyRestApi, slaQueryService);

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入GAUSSDBT宏
     * 检查点：是否返回true
     */
    @Test
    public void applicable_restore_interceptor_provider_success() {
        Assert.assertTrue(gaussDBTRestoreInterceptorProvider.applicable(ResourceSubTypeEnum.GAUSSDBT.getType()));
    }

    /**
     * 用例场景：恢复资源不存在，抛出异常
     * 前置条件：恢复资源不存在
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_restore_resource_is_not_exist() {
        RestoreTask task = getRestoreTask();
        PowerMockito.when(resourceService.getResourceById(anyBoolean(), anyString())).thenReturn(Optional.empty());
        Assert.assertThrows(LegoCheckedException.class, () -> gaussDBTRestoreInterceptorProvider.initialize(task));
    }

    /**
     * 用例场景：创建agents到恢复任务中成功
     * 前置条件：agents存在且nodes中的信息存在
     * 检查点：回填成功
     * @throws Exception MethodNotFoundException
     */
    @Test
    public void build_task_agents_success() throws Exception {
        ProtectedResource protectedResource = new ProtectedResource();
        PowerMockito.when(resourceService.getResourceById(anyBoolean(), anyString()))
            .thenReturn(Optional.of(protectedResource));
        PowerMockito.when(providerManager.findProvider(any(), any()))
            .thenReturn(resourceConnectionCheckProvider);
        ResourceCheckContext checkContext = new ResourceCheckContext();
        Map<ProtectedResource, List<ProtectedEnvironment>> map = new HashMap<>();
        List<ProtectedEnvironment> environments = new ArrayList<>();
        ProtectedEnvironment environment = getProtectedEnvironment();
        environment.setEndpoint("127.0.0.1");
        environment.setPort(59526);
        environments.add(environment);
        map.put(environment, environments);
        checkContext.setResourceConnectableMap(map);
        PowerMockito.when(resourceConnectionCheckProvider.checkConnection(any())).thenReturn(checkContext);
        RestoreTask restoreTask = getRestoreTask();
        Whitebox.invokeMethod(gaussDBTRestoreInterceptorProvider, "buildRestoreAgents", restoreTask);
        Assert.assertEquals(1, restoreTask.getAgents().size());
    }

    /**
     * 用例场景：创建repositories到恢复任务中成功
     * 前置条件：repositories存在且为时间点恢复
     * 检查点：回填成功
     * @throws Exception MethodNotFoundException
     */
    @Test
    public void build_task_repositories_success() throws Exception {
        RestoreTask task = getRestoreTask();
        List<StorageRepository> list = new ArrayList<>();
        list.add(new StorageRepository());
        task.setRepositories(list);
        Map<String, String> advanceParams = new HashMap<>();
        advanceParams.put(GaussDBTConstant.RESTORE_TIME_STAMP_KEY, String.valueOf(System.currentTimeMillis()));
        task.setAdvanceParams(advanceParams);
        Whitebox.invokeMethod(gaussDBTRestoreInterceptorProvider, "buildRestoreRepositories", task);
        Assert.assertEquals(2, task.getRepositories().size());
        Assert.assertEquals(RepositoryTypeEnum.LOG.getType(), (int) task.getRepositories().get(1).getType());
    }

    /**
     * 用例场景：回填restoreLocation到恢复任务中成功
     * 前置条件：restoreLocation从界面下发
     * 检查点：回填成功
     * @throws Exception MethodNotFoundException
     */
    @Test
    public void build_task_restore_location_success() throws Exception {
        RestoreTask task = getRestoreTask();
        task.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("uuid");
        task.setTargetObject(taskResource);
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.empty());
        Whitebox.invokeMethod(gaussDBTRestoreInterceptorProvider, "buildRestoreAdvanceParams", task);
        Assert.assertEquals(RestoreLocationEnum.ORIGINAL.getLocation(),
            task.getAdvanceParams().get(GaussDBTConstant.TARGET_LOCATION_KEY));
        Assert.assertEquals("true", task.getAdvanceParams().get(DatabaseConstants.MULTI_POST_JOB));
    }

    /**
     * 用例场景：回填targetEnv中的nodes到恢复任务中
     * 前置条件：targetEnv为空
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_restore_targetEnv_is_empty() {
        RestoreTask task = getRestoreTask();
        Assert.assertThrows(LegoCheckedException.class,
            () -> Whitebox.invokeMethod(gaussDBTRestoreInterceptorProvider, "buildRestoreTargetEvn", task));
    }

    /**
     * 用例场景：回填targetEnv中的nodes到恢复任务中
     * 前置条件：targetEnv不存在
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_restore_targetEnv_is_not_exist() {
        RestoreTask task = getRestoreTask();
        task.setTargetEnv(new TaskEnvironment());
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.empty());
        Assert.assertThrows(LegoCheckedException.class,
            () -> Whitebox.invokeMethod(gaussDBTRestoreInterceptorProvider, "buildRestoreTargetEvn", task));
    }

    /**
     * 用例场景：回填targetEnv中的nodes到恢复任务中
     * 前置条件：targetEnv在数据库中存在
     * 检查点：回填成功
     */
    @Test
    public void build_task_restore_targetEnv_nodes_success() throws Exception {
        RestoreTask task = getRestoreTask();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid("test_uuid");
        task.setTargetEnv(taskEnvironment);
        ProtectedEnvironment environment = getProtectedEnvironment();
        String nodeString
            = "[{\"uuid\":\"8644ded8-7168-4e17-b288-1187aeb5c626\",\"name\":\"GaussDBT_Single_08\","
            + "\"type\":\"DataBase\",\"subType\":\"GaussDBT\",\"endpoint\":\"8.40.146.110\",\"role\":1,"
            + "\"extendInfo\":{\"status\":\"ONLINE\"}}]";
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(GaussDBTConstant.NODES_KEY, nodeString);
        environment.setExtendInfo(extendInfo);
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(environment));
        Whitebox.invokeMethod(gaussDBTRestoreInterceptorProvider, "buildRestoreTargetEvn", task);
        Assert.assertEquals(1, task.getTargetEnv().getNodes().size());
    }

    /**
     * 用例场景：回填副本恢复类型到恢复任务中成功
     * 前置条件：副本正常，下发任务正常
     * 检查点：回填成功-LocalRestore
     * @throws Exception MethodNotFoundException
     */
    @Test
    public void build_task_restore_mode_with_LocalRestore_type_success() throws Exception {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        RestoreTask restoreTask = getRestoreTask();
        restoreTask.setCopyId("test_copy");
        Whitebox.invokeMethod(gaussDBTRestoreInterceptorProvider, "buildRestoreMode", restoreTask);
        Assert.assertEquals(RestoreModeEnum.LOCAL_RESTORE.getMode(), restoreTask.getRestoreMode());
    }

    /**
     * 用例场景：回填副本恢复类型到恢复任务中成功
     * 前置条件：副本正常，下发任务正常
     * 检查点：回填成功-DownloadRestore
     * @throws Exception MethodNotFoundException
     */
    @Test
    public void build_task_restore_mode_with_DownloadRestore_type_success() throws Exception {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value());
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        RestoreTask restoreTask = getRestoreTask();
        restoreTask.setCopyId("test_copy");
        Whitebox.invokeMethod(gaussDBTRestoreInterceptorProvider, "buildRestoreMode", restoreTask);
        Assert.assertEquals(RestoreModeEnum.DOWNLOAD_RESTORE.getMode(), restoreTask.getRestoreMode());
    }

    /**
     * 用例场景：恢复时候同一个资源只允许下发一个恢复任务
     * 前置条件：副本正常，下发任务正常
     * 检查点：返回资源锁成功
     */
    @Test
    public void try_lock_with_same_resource_success_when_restore() {
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setTargetObject(new TaskResource());
        restoreTask.getTargetObject().setUuid("test_uuid");
        List<LockResourceBo> lockResources = gaussDBTRestoreInterceptorProvider.getLockResources(restoreTask);
        Assert.assertEquals(1, lockResources.size());
        Assert.assertEquals(lockResources.get(0).getLockType(), LockType.WRITE);
    }

    private RestoreTask getRestoreTask() {
        RestoreTask restoreTask = new RestoreTask();
        TaskEnvironment gaussDBT = new TaskEnvironment();
        gaussDBT.setUuid("target_uuid");
        restoreTask.setTargetEnv(gaussDBT);
        return restoreTask;
    }

    private ProtectedEnvironment getProtectedEnvironment() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setName("gaussDB");
        Authentication authentication = new Authentication();
        authentication.setAuthType(1);
        authentication.setAuthKey("omm");
        protectedEnvironment.setAuth(authentication);
        protectedEnvironment.setType(ResourceTypeEnum.DATABASE.getType());
        protectedEnvironment.setSubType(ResourceSubTypeEnum.GAUSSDBT.getType());
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("deployType", "0");
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("test_uuid");
        protectedEnvironment.setDependencies(Collections.singletonMap("agents", Collections.singletonList(resource)));
        protectedEnvironment.setExtendInfo(extendInfo);
        return protectedEnvironment;
    }
}