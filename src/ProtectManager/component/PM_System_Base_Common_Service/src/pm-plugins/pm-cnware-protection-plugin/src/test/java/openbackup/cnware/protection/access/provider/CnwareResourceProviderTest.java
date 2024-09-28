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
package openbackup.cnware.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import org.junit.Assert;
import org.junit.Test;

/**
 * CNware AlwaysOnProvider
 *
 */
public class CnwareResourceProviderTest {
    private final CnwareResourceProvider cnwareResourceProvider = new CnwareResourceProvider();

    /**
     * 用例场景：SQL Server 清理固有属性
     * 前置条件：传入parentName和parentUuid
     * 检查点：parentName返回不为空允许更改,parentUuid返回为空不允许更改
     */
    @Test
    public void clean_unmodifiable_fields_when_update_success() {
        ProtectedResource protectedResource = mockObject();
        cnwareResourceProvider.cleanUnmodifiableFieldsWhenUpdate(protectedResource);
        Assert.assertEquals("cnware_testName",protectedResource.getParentName());
        Assert.assertNull(protectedResource.getParentUuid());
    }

    /**
     * 用例场景：Cnware备份插件类型判断正确 <br/>
     * 前置条件：流程正常 <br/>
     * 检查点：返回结果为True
     */
    @Test
    public void test_applicable_success() {
        boolean cNwareVm = cnwareResourceProvider.applicable(mockObject());
        Assert.assertTrue(cNwareVm);
    }

    private ProtectedResource mockObject() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setParentUuid("111");
        protectedResource.setParentName("cnware_testName");
        protectedResource.setSubType("CNwareVm");
        return protectedResource;
    }
}
