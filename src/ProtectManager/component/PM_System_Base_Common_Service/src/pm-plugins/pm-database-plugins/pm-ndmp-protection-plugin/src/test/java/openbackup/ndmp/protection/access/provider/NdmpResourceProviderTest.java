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
package openbackup.ndmp.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

/**
 * 资源接入类实现
 *
 */
public class NdmpResourceProviderTest {
    private NdmpResourceProvider ndmpResourceProvider;

    @Before
    public void init() throws IllegalAccessException {
        ndmpResourceProvider = new NdmpResourceProvider();
    }

    /**
     * 用例场景：检查类provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.NDMP_BACKUPSET.getType());
        Assert.assertTrue(ndmpResourceProvider.applicable(protectedResource));
    }

    /**
     * 用例场景：设置路径
     * 前置条件：复制校验
     * 检查点：复制校验路径设置成功
     */
    @Test
    public void beforeCreate() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setName("path");
        ndmpResourceProvider.beforeCreate(protectedResource);
        Assert.assertEquals("path", protectedResource.getPath());
    }

    /**
     * 将ResourceFeature对象的isSupportedLanFree属性置为false表示不支持lanfree并返回
     *
     * @return 资源是否支持lanfree
     */
    @Test
    public void getResourceFeature() {
        Assert.assertFalse(ndmpResourceProvider.getResourceFeature().isSupportedLanFree());
    }
}
