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

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.informix.protection.access.service.InformixService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

/**
 * {@link InformixSingleInstanceProvider Test}
 *
 * @author dwx1009286
 * @version [DataBackup 1.5.0]
 * @since 2023-05-17
 */
public class InformixSingleInstanceProviderTest {
    private final InformixService informixService = PowerMockito.mock(InformixService.class);
    private final InstanceResourceService instanceResourceService = PowerMockito.mock(InstanceResourceService.class);

    private final InformixSingleInstanceProvider provider =
            new InformixSingleInstanceProvider(informixService, instanceResourceService);

    /**
     * 用例场景：applicable单实例类型
     * 前置条件：类型为Informix-singleInstance
     * 检查点：是否返回true
     */
    @Test
    public void applicable_informix_singleInstance_success() {
        final ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.getType());
        Assert.assertTrue(provider.applicable(resource));
    }

    /**
     * 用例场景：创建前置检查成功
     * 前置条件：无
     * 检查点：是否调用informixService进行参数校验与封装
     */
    @Test
    public void before_create_success() {
        final ProtectedResource resource = new ProtectedResource();
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setPath("/temp/123123123");
        resource.setEnvironment(environment);
        provider.beforeCreate(resource);
        Mockito.verify(informixService, Mockito.times(1)).doSingleInstanceAction(resource, true);
    }

    /**
     * 用例场景：更新前置检查成功
     * 前置条件：无
     * 检查点：是否调用informixService进行参数校验与封装
     */
    @Test
    public void before_update_success() {
        final ProtectedResource resource = new ProtectedResource();
        provider.beforeUpdate(resource);
        Mockito.verify(informixService, Mockito.times(1)).doSingleInstanceAction(resource, false);
    }
}