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

import openbackup.dameng.protection.access.DamengTestDataUtil;

import openbackup.dameng.protection.access.service.DamengService;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.restore.RestoreFeature;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.List;

/**
 * 功能描述
 *
 */
public class DamengClusterRestoreInterceptorTest {
    private final DamengService damengService = PowerMockito.mock(DamengService.class);

    private final DamengClusterRestoreInterceptor damengClusterRestoreInterceptor = new DamengClusterRestoreInterceptor(
        damengService);

    /**
     * 用例场景：dameng环境检查类过滤
     * 前置条件：无
     * 检查点：过滤成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(damengClusterRestoreInterceptor.applicable(ResourceSubTypeEnum.DAMENG_CLUSTER.getType()));
    }

    /**
     * 用例场景：dameng集群恢复参数设置
     * 前置条件：无
     * 检查点：恢复参数是否设置成功
     */
    @Test
    public void intercept_success() {
        RestoreTask restoreTask = damengClusterRestoreInterceptor.initialize(DamengTestDataUtil.buildRestoreTask());
        Assert.assertTrue(restoreTask.getTargetEnv().getExtendInfo()
            .get(DatabaseConstants.DEPLOY_TYPE)
            .equals(DatabaseDeployTypeEnum.AP.getType()));
    }

    /**
     * 用例场景：dameng集群设置恢复时是否检查环境状态
     * 前置条件：无
     * 检查点：设置不检查成功
     */
    @Test
    public void get_restore_feature_success() {
        RestoreFeature restoreFeature = damengClusterRestoreInterceptor.getRestoreFeature();
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
        List<LockResourceBo> lockResources = damengClusterRestoreInterceptor.getLockResources(restoreTask);
        Assert.assertEquals(lockResources.get(0).getLockType(), LockType.WRITE);
    }
}
