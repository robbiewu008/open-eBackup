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
package openbackup.ndmp.protection.access.common;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.ndmp.protection.access.constant.NdmpConstant;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.HashMap;

/**
 * NdmpCommonTest
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(NdmpCommon.class)
public class NdmpCommonTest {

    @Test
    public void test_set_name() {
        ProtectedResource resource = new ProtectedResource();
        resource.setName("/tenantName/file");
        resource.setExtendInfo(new HashMap<>());
        NdmpCommon.setNdmpNames(resource);
        Assert.assertNotNull(resource.getExtendInfoByKey(NdmpConstant.FULL_NAME));
        Assert.assertNotNull(resource.getExtendInfoByKey(NdmpConstant.TENANT_NAME));
        resource.setName("/file");
        NdmpCommon.setNdmpNames(resource);
        Assert.assertNotNull(resource.getExtendInfoByKey(NdmpConstant.FULL_NAME));
    }


}
