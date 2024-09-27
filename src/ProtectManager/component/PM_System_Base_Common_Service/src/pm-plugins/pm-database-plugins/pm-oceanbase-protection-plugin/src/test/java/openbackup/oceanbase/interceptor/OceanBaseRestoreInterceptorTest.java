/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.oceanbase.interceptor;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyString;
import static org.powermock.api.mockito.PowerMockito.when;

import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.oceanbase.OceanBaseTest;

import openbackup.oceanbase.provider.OceanBaseAgentProvider;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.List;
import java.util.Optional;

/**
 * 功能描述
 *
 * @author x30028756
 * @since 2023-08-08
 */
@RunWith(PowerMockRunner.class)
public class OceanBaseRestoreInterceptorTest extends OceanBaseTest {
    private OceanBaseRestoreInterceptor restoreInterceptor;

    private ResourceService resourceService;

    private CopyRestApi copyRestApi;

    private OceanBaseAgentProvider oceanBaseAgentProvider;

    private DeployTypeService deployTypeService;

    @Override
    @Before
    public void init() {
        super.init();
        resourceService = Mockito.mock(ResourceService.class);
        deployTypeService = Mockito.mock(DeployTypeService.class);
        copyRestApi = Mockito.mock(CopyRestApi.class);
        oceanBaseAgentProvider = new OceanBaseAgentProvider(oceanBaseService);
        restoreInterceptor = new OceanBaseRestoreInterceptor(resourceService, copyRestApi, oceanBaseAgentProvider,
            oceanBaseService, deployTypeService);
    }

    /**
     * 用例场景：OB 下发集群恢复 applicable 校验
     * 前置条件：无
     * 检查点：类过滤成功
     */
    @Test
    public void check_OceanBase_cluster_applicable_success() {
        boolean applicable = restoreInterceptor.applicable(ResourceSubTypeEnum.OCEAN_BASE_CLUSTER.getType());
        Assert.assertTrue(applicable);
    }

    /**
     * 用例场景：OB 下发租户集任务 applicable 校验
     * 前置条件：无
     * 检查点：类过滤成功
     */
    @Test
    public void check_OceanBase_tenant_applicable_success() {
        boolean applicable = restoreInterceptor.applicable(ResourceSubTypeEnum.OCEAN_BASE_TENANT.getType());
        Assert.assertTrue(applicable);
    }

    /**
     * 用例场景：下发恢复任务时 针对资源进行锁定
     * 前置条件：构造restoreTask结构体
     * 检查点: 成功返回加锁列表
     */
    @Test
    public void restore_getLock_resources() {
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setTargetObject(new TaskResource());
        restoreTask.getTargetObject().setUuid("test_uuid");
        List<LockResourceBo> lockResources = restoreInterceptor.getLockResources(restoreTask);
        Assert.assertEquals(lockResources.get(0).getLockType(), LockType.WRITE);
    }

    /**
     * 用例场景：参数错误
     * 前置条件：无
     * 检查点：检查报错
     */
    @Test
    public void test_intercept() {
        mockGetEvnById();
        Copy copy = getCopy();
        when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        RestoreTask task = new RestoreTask();
        task.setTargetEnv(BeanTools.copy(mockProtectedEnvironment(), new TaskEnvironment()));
        PowerMockito.when(resourceService.getResourceById(anyBoolean(), anyString()))
            .thenReturn(Optional.of(mockProtectedResource()));
        RestoreTask restoreTask = restoreInterceptor.initialize(task);
        Assert.assertEquals(restoreTask.getRestoreMode(), RestoreModeEnum.LOCAL_RESTORE.getMode());
        Assert.assertEquals(restoreTask.getAgents().size(), 3);
        Assert.assertEquals(restoreTask.getTargetEnv().getNodes().size(), 3);
    }

    private Copy getCopy() {
        Copy copy = new Copy();
        copy.setStatus("Normal");
        copy.setResourceId("uuid0");
        copy.setGeneration(1);
        copy.setTimestamp(Long.toString(System.currentTimeMillis()));
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        return copy;
    }
}
