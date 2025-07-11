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
package openbackup.informix.protection.access.interceptor;

import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.informix.protection.access.provider.copy.InformixCopyDeleteInterceptor;
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
public class InformixCopyDeleteInterceptorTest {

    private InformixCopyDeleteInterceptor informixCopyDeleteInterceptor;

    private CopyRestApi copyRestApi;

    private ResourceService resourceService;

    @Before
    public void init() {
        copyRestApi = Mockito.mock(CopyRestApi.class);
        resourceService = Mockito.mock(ResourceService.class);
        informixCopyDeleteInterceptor = new InformixCopyDeleteInterceptor(copyRestApi, resourceService);
    }

    /**
     * 用例场景：OB 下发备份任务 applicable 校验
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void check_applicable_success() {
        boolean applicable = informixCopyDeleteInterceptor.applicable(ResourceSubTypeEnum.INFORMIX_CLUSTER_INSTANCE.getType());
        Assert.assertTrue(applicable);
    }
}
