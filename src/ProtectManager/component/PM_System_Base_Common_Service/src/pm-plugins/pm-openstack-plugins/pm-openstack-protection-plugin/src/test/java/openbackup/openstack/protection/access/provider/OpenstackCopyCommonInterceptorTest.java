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

import openbackup.openstack.protection.access.provider.OpenstackCopyCommonInterceptor;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;

/**
 * OpenstackCopyCommonInterceptor
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(OpenstackCopyCommonInterceptor.class)
@AutoConfigureMockMvc
public class OpenstackCopyCommonInterceptorTest {

    @Test
    public void applicable() {
        OpenstackCopyCommonInterceptor provider = new OpenstackCopyCommonInterceptor();
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.getType()));
    }
}
