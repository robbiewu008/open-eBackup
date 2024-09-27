/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.opengauss.resources.access.interceptor;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.opengauss.resources.access.provider.OpenGaussMockData;

import openbackup.opengauss.resources.access.service.OpenGaussAgentService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.Collections;
import java.util.List;

/**
 * OpenGaussRestoreInterceptorProvider环境恢复测试类
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-15
 */
public class OpenGaussRestoreInterceptorProviderTest {
    private OpenGaussRestoreInterceptorProvider openGaussRestoreProvider;
    private OpenGaussAgentService openGaussAgentService;
    private CopyRestApi copyRestApi;
    private ResourceService resourceService;

    @Before
    public void init() {
        openGaussAgentService = PowerMockito.mock(OpenGaussAgentService.class);
        copyRestApi = PowerMockito.mock(CopyRestApi.class);
        resourceService = PowerMockito.mock(ResourceService.class);
        openGaussRestoreProvider = new OpenGaussRestoreInterceptorProvider(openGaussAgentService, copyRestApi,
            resourceService);
    }

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入资源类型
     * 检查点：是否返回true
     */
    @Test
    public void applicable_open_gauss_restore_interceptor_provider_success() {
        Assert.assertFalse(openGaussRestoreProvider.applicable(ResourceSubTypeEnum.GAUSSDBT.getType()));
        Assert.assertTrue(openGaussRestoreProvider.applicable(ResourceSubTypeEnum.OPENGAUSS_INSTANCE.getType()));
    }

    /**
     * 用例场景：特性开关，用于判断是否需要对目标资源是否在线进行校验
     * 前置条件：无
     * 检查点：是否返回false
     */
    @Test
    public void get_restore_feature_success() {
        Assert.assertFalse(openGaussRestoreProvider.getRestoreFeature().isShouldCheckEnvironmentIsOnline());
    }

    /**
     * 用例场景：执行恢复任务填充恢复模式、nodes节点信息否符合预期
     * 前置条件：恢复参数信息正确
     * 检查点：1、检查恢复模式参数回填是否符合预期。2、nodes节点信息否符合预期
     */
    @Test
    public void should_return_ok_when_intercept() {
        Endpoint endpoint = new Endpoint();
        endpoint.setIp("8.3.3.6");
        endpoint.setPort(8963);
        PowerMockito.when(openGaussAgentService.getAgentEndpoint(any()))
            .thenReturn(Collections.singletonList(endpoint));

        PowerMockito.when(openGaussAgentService.buildEnvironmentNodes(any()))
            .thenReturn(OpenGaussMockData.mockTaskEnvironmentContainsNodes());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(PowerMockito.mock(Copy.class));
        RestoreTask restoreTask = openGaussRestoreProvider.initialize(OpenGaussMockData.buildRestoreTaskEnvNodeInfo());
        List<TaskEnvironment> nodes = restoreTask.getTargetEnv().getNodes();
        Assert.assertEquals(1, nodes.size());
        Assert.assertEquals("uuid_node_1111", nodes.get(0).getUuid());
        Assert.assertEquals("node_1", nodes.get(0).getName());
        Assert.assertEquals(RestoreModeEnum.LOCAL_RESTORE.getMode(), restoreTask.getRestoreMode());
        Assert.assertEquals("/opengauss_cluster/openGauss-instance-name/newName",
            restoreTask.getTargetObject().getTargetLocation());
        Assert.assertEquals(SpeedStatisticsEnum.UBC.getType(),
            restoreTask.getAdvanceParams().get(TaskUtil.SPEED_STATISTICS));
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

        Copy copy = new Copy();
        copy.setUuid("uuid");
        copy.setResourceName("databae_test_1");
        copy.setResourceSubType(ResourceSubTypeEnum.OPENGAUSS_DATABASE.getType());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);

        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        ProtectedResource dataBaseProtectedResource = new ProtectedResource();
        dataBaseProtectedResource.setUuid("database_test_uuid_1");
        response.setRecords(Collections.singletonList(dataBaseProtectedResource));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(response);
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        List<LockResourceBo> lockResources = openGaussRestoreProvider.getLockResources(restoreTask);
        Assert.assertEquals(1, lockResources.size());
        Assert.assertEquals(lockResources.get(0).getLockType(), LockType.WRITE);
        Assert.assertEquals("database_test_uuid_1", lockResources.get(0).getId());
    }
}