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
package openbackup.dameng.protection.access.interceptor;

import static org.mockito.ArgumentMatchers.any;

import openbackup.dameng.protection.access.DamengTestDataUtil;
import openbackup.dameng.protection.access.constant.DamengConstant;
import openbackup.dameng.protection.access.service.DamengService;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.restore.RestoreFeature;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.utils.AgentDtoUtil;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

import java.util.Collections;
import java.util.List;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({AgentDtoUtil.class})
public class DamengSingleNodeRestoreInterceptorTest {
    private final DamengService damengService = PowerMockito.mock(DamengService.class);

    private final AgentUnifiedService agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);

    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private final DamengSingleNodeRestoreInterceptor damengSingleNodeRestoreInterceptor
        = new DamengSingleNodeRestoreInterceptor(damengService, agentUnifiedService, copyRestApi);

    /**
     * 用例场景：dameng环境检查类过滤
     * 前置条件：无
     * 检查点：过滤成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(
            damengSingleNodeRestoreInterceptor.applicable(ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType()));
    }

    /**
     * 用例场景：dameng原位置恢复参数设置
     * 前置条件：原位置
     * 检查点：恢复参数是否设置成功
     */
    @Test
    public void intercept_success_if_target_location_is_original() {
        PowerMockito.when(damengService.getEnvironmentById(any())).thenReturn(buildProtectedEnvironment());
        PowerMockito.when(damengService.getAgentEndpoint(any())).thenReturn(buildEndpoint());
        PowerMockito.mockStatic(AgentDtoUtil.class);
        PowerMockito.when(AgentDtoUtil.toTaskEnvironment(any())).thenReturn(new TaskEnvironment());
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        RestoreTask restoreTask = damengSingleNodeRestoreInterceptor.initialize(DamengTestDataUtil.buildRestoreTask());
        Assert.assertTrue(restoreTask.getTargetEnv().getExtendInfo()
            .get(DatabaseConstants.DEPLOY_TYPE)
            .equals(DatabaseDeployTypeEnum.SINGLE.getType()));
    }

    /**
     * 用例场景：dameng新位置恢复参数设置
     * 前置条件：新位置
     * 检查点：恢复参数是否设置成功
     */
    @Test
    public void intercept_success_if_target_location_is_new() {
        PowerMockito.when(damengService.getEnvironmentById(any())).thenReturn(buildProtectedEnvironment());
        PowerMockito.when(damengService.getAgentEndpoint(any())).thenReturn(buildEndpoint());
        PowerMockito.mockStatic(AgentDtoUtil.class);
        PowerMockito.when(AgentDtoUtil.toTaskEnvironment(any())).thenReturn(new TaskEnvironment());
        RestoreTask restoreTask = DamengTestDataUtil.buildRestoreTask();
        restoreTask.setTargetLocation(RestoreLocationEnum.NEW);
        RestoreTask task = damengSingleNodeRestoreInterceptor.initialize(restoreTask);
        Assert.assertTrue(task.getTargetEnv().getExtendInfo()
            .get(DamengConstant.DB_PATH)
            .equals("/dbPath"));
    }

    /**
     * 用例场景：dameng表空间恢复原机原位置校验
     * 前置条件：1.原位置 2.表空间恢复
     * 检查点：恢复参数是否设置成功
     */
    @Test
    public void intercept_success_if_tablespace_restore_location_is_original() {
        PowerMockito.when(damengService.getEnvironmentById(any())).thenReturn(buildProtectedEnvironment());
        PowerMockito.when(damengService.getAgentEndpoint(any())).thenReturn(buildEndpoint());
        PowerMockito.mockStatic(AgentDtoUtil.class);
        PowerMockito.when(AgentDtoUtil.toTaskEnvironment(any())).thenReturn(new TaskEnvironment());
        RestoreTask restoreTask = DamengTestDataUtil.buildRestoreTask();
        restoreTask.setRestoreType(RestoreTypeEnum.FLR.getType());
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        RestoreTask task = damengSingleNodeRestoreInterceptor.initialize(restoreTask);
        Assert.assertTrue(task.getTargetEnv().getExtendInfo()
            .get(DatabaseConstants.DEPLOY_TYPE)
            .equals(DatabaseDeployTypeEnum.SINGLE.getType()));
    }

    /**
     * 用例场景：dameng表空间恢复新位置校验
     * 前置条件：1.新位置 2.表空间恢复
     * 检查点：抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_tablespace_restore_location_is_new() {
        RestoreTask restoreTask = DamengTestDataUtil.buildRestoreTask();
        restoreTask.setTargetLocation(RestoreLocationEnum.NEW);
        restoreTask.setRestoreType(RestoreTypeEnum.FLR.getType());
        Assert.assertThrows(LegoCheckedException.class, () -> damengSingleNodeRestoreInterceptor.initialize(restoreTask));
    }

    /**
     * 用例场景：dameng归档副本表空间恢复校验
     * 前置条件：1.归档副本 2.表空间恢复
     * 检查点：抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_tablespace_restore_and_copy_generated_by_archive() {
        RestoreTask restoreTask = DamengTestDataUtil.buildRestoreTask();
        restoreTask.setRestoreType(RestoreTypeEnum.FLR.getType());
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value());
        copy.setIsReplicated(false);
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        Assert.assertThrows(LegoCheckedException.class, () -> damengSingleNodeRestoreInterceptor.initialize(restoreTask));
    }

    /**
     * 用例场景：dameng单机设置恢复时是否检查环境状态
     * 前置条件：无
     * 检查点：设置不检查成功
     */
    @Test
    public void get_restore_feature_success() {
        RestoreFeature restoreFeature = damengSingleNodeRestoreInterceptor.getRestoreFeature();
        Assert.assertFalse(restoreFeature.isShouldCheckEnvironmentIsOnline());
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
        List<LockResourceBo> lockResources = damengSingleNodeRestoreInterceptor.getLockResources(restoreTask);
        Assert.assertEquals(lockResources.get(0).getLockType(), LockType.WRITE);
    }

    /**
     * 用例场景：细粒度恢复下发恢复任务时 对subObjects参数进行封装
     * 前置条件：构造restoreTask结构体
     * 检查点: 返回期望的subObjects格式
     */
    @Test
    public void supply_subObjects_success() throws Exception {
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setRestoreType(RestoreTypeEnum.FLR.getType());
        TaskResource taskResource = new TaskResource();
        taskResource.setName("SPACE7938");
        restoreTask.setSubObjects(Collections.singletonList(taskResource));
        Whitebox.invokeMethod(damengSingleNodeRestoreInterceptor, "updateSubObjects", restoreTask);
        Assert.assertNull(restoreTask.getSubObjects().get(0).getName());
        Assert.assertEquals("SPACE7938", restoreTask.getSubObjects().get(0).getExtendInfo().get(DatabaseConstants.NAME));
    }

    private ProtectedEnvironment buildProtectedEnvironment() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("127.0.0.1");
        environment.setUuid("uuid");
        return environment;
    }

    private Endpoint buildEndpoint() {
        Endpoint endpoint = new Endpoint();
        endpoint.setId("uuid");
        endpoint.setIp("127.0.0.1");
        endpoint.setPort(5236);
        return endpoint;
    }
}
