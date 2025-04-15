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
package openbackup.tpops.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import junit.framework.TestCase;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.junit.MockitoJUnitRunner;

/**
 * 功能描述 GaussDBTpopsAlwaysOnProvider用例
 *
 */
@RunWith(MockitoJUnitRunner.class)
public class GaussDBTpopsAlwaysOnProviderTest extends TestCase {

    private GaussDBTpopsAlwaysOnProvider gaussDBTpopsAlwaysOnProvider;

    private ProtectedResource protectedResource;

    @Before
    public void init() throws IllegalAccessException {
        gaussDBTpopsAlwaysOnProvider =  new GaussDBTpopsAlwaysOnProvider();
        protectedResource = new ProtectedResource();
    }

    /**
     * 用例场景：GaussDBTPOPS 擦除可更新字段
     * 前置条件：无
     * 检查点：不可更新字段被擦除
     */
    @Test
    public void testCleanUnmodifiableFieldsWhenUpdate() {
        protectedResource.setRootUuid("test");
        gaussDBTpopsAlwaysOnProvider.cleanUnmodifiableFieldsWhenUpdate(protectedResource);
        Assert.assertNull(protectedResource.getRootUuid());
    }

    /**
     * 用例场景：GaussDBTPOPS匹配 不可更新字段provider
     * 前置条件：使用正确的subType
     * 检查点：类过滤成功
     */
    @Test
    public void testApplicableTrue() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType());
        Assert.assertTrue(
            gaussDBTpopsAlwaysOnProvider.applicable(protectedResource));
    }

    /**
     * 用例场景：GaussDBTPOPS匹配 不可更新字段provider
     * 前置条件：使用错误的subType
     * 检查点：类过滤成功
     */
    @Test
    public void testApplicableFalse() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.TPOPS_GAUSSDB_PROJECT.getType());
        Assert.assertFalse(
            gaussDBTpopsAlwaysOnProvider.applicable(protectedResource));
    }
}