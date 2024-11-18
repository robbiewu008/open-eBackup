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
package openbackup.postgre.protection.access.provider.resource;

import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
public class PostgresCopyDeleteInterceptorTest {
    private PostgreCopyDeleteInterceptor copyDeleteInterceptor;

    private CopyRestApi copyRestApi;

    private ResourceService resourceService;

    @Before
    public void init() {
        copyRestApi = Mockito.mock(CopyRestApi.class);
        resourceService = Mockito.mock(ResourceService.class);
        copyDeleteInterceptor = new PostgreCopyDeleteInterceptor(copyRestApi, resourceService);
    }

    /**
     * 用例场景：pg 下发备份任务 applicable 校验
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void check_applicable_success() {
        boolean applicable = copyDeleteInterceptor.applicable(ResourceSubTypeEnum.OCEAN_BASE_CLUSTER.getType());
        Assert.assertTrue(applicable);
    }

    /**
     * 用例场景：初始化仓库成功
     * 前置条件：OceanProtect服务正常
     * 检查点：检查仓库类型
     */
    @Test
    public void check_supply_backup_task_success() {
        boolean shouldSupplyAgent = copyDeleteInterceptor.shouldSupplyAgent(new DeleteCopyTask(), new CopyInfoBo());
        Assert.assertFalse(shouldSupplyAgent);
    }
}