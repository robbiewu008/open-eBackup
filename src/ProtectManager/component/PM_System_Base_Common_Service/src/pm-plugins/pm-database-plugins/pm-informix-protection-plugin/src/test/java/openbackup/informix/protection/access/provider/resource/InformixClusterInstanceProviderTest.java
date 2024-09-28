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
package openbackup.informix.protection.access.provider.resource;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.informix.protection.access.service.InformixService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

/**
 * {@link InformixClusterInstanceProvider Test}
 *
 */
public class InformixClusterInstanceProviderTest {
    private static final InformixService informixService = PowerMockito.mock(InformixService.class);

    private static final InformixClusterInstanceProvider provider = new InformixClusterInstanceProvider(informixService);

    /**
     * 用例场景：applicable集群实例类型
     * 前置条件：类型为Informix-clusterInstance
     * 检查点：是否返回true
     */
    @Test
    public void applicable_informix_clusterInstance_success() {
        final ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.INFORMIX_CLUSTER_INSTANCE.getType());
        Assert.assertTrue(provider.applicable(resource));
    }

    /**
     * 用例场景：创建前置检查成功
     * 前置条件：无
     * 检查点：是否调用informixService进行参数校验与封装
     */
    @Test
    public void check_success() {
        ProtectedResource resource = new ProtectedResource();
        provider.check(resource);
        Mockito.verify(informixService, Mockito.times(1)).doClusterInstanceAction(resource, true);
    }

    /**
     * 用例场景：更新检查成功
     * 前置条件：无
     * 检查点：是否调用informixService进行参数校验与封装
     */
    @Test
    public void update_check_success() {
        ProtectedResource resource = new ProtectedResource();
        provider.updateCheck(resource);
        Mockito.verify(informixService, Mockito.times(1)).doClusterInstanceAction(resource, false);
    }
}
