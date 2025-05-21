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
package com.huawei.oceanprotect.k8s.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

/**
 * k8s NameSpace资源类provider
 *
 */
public class K8sNameSpaceResourceProviderTest {
    private K8sNameSpaceResourceProvider k8sNameSpaceResourceProvider;

    @Before
    public void init(){
        k8sNameSpaceResourceProvider=new K8sNameSpaceResourceProvider();
    }

    @Test
    public void test_clean_unmodifiable_fields_when_update() {
        ProtectedResource resource=new ProtectedResource();
        resource.setParentName("parentname");
        k8sNameSpaceResourceProvider.cleanUnmodifiableFieldsWhenUpdate(resource);
        Assert.assertEquals("parentname",resource.getParentName());
    }

    @Test
    public void test_applicable_success() {
        ProtectedResource object = new ProtectedResource();
        object.setSubType(ResourceSubTypeEnum.KUBERNETES_NAMESPACE_COMMON.getType());
        Assert.assertTrue(k8sNameSpaceResourceProvider.applicable(object));
    }
}