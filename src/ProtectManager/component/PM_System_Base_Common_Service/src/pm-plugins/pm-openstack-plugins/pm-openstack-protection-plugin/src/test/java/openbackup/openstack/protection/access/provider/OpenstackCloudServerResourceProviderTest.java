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
package openbackup.openstack.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.openstack.protection.access.provider.OpenstackCloudServerResProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

/**
 * 功能描述: OpenstackCloudServerResourceProviderTest
 *
 */
public class OpenstackCloudServerResourceProviderTest {
    private static OpenstackCloudServerResProvider openstackCloudServerResProvider;

    @BeforeClass
    public static void init() {
        openstackCloudServerResProvider = new OpenstackCloudServerResProvider();
    }

    /**
     * 用例场景：OpenStack云主机插件类型判断正确 <br/>
     * 前置条件：流程正常 <br/>
     * 检查点：返回结果为True
     */
    @Test
    public void test_applicable_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.getType());
        boolean openStackCloudServer = openstackCloudServerResProvider.applicable(resource);
        Assert.assertTrue(openStackCloudServer);
    }

    /**
     * 用例场景：Openstack不支持lanFree场景 <br/>
     * 前置条件：无 <br/>
     * 检查点：返回结果为false
     */
    @Test
    public void test_get_resource_feature_success() {
        ResourceFeature resourceFeature = openstackCloudServerResProvider.getResourceFeature();
        Assert.assertFalse(resourceFeature.isSupportedLanFree());
    }
}