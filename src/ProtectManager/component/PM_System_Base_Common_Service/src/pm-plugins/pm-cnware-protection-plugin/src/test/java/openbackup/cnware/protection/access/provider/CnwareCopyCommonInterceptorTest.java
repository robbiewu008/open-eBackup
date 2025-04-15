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

import openbackup.cnware.protection.access.provider.CnwareCopyCommonInterceptor;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;

/**
 * CnwareCopyCommonInterceptor
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(CnwareCopyCommonInterceptor.class)
@AutoConfigureMockMvc
public class CnwareCopyCommonInterceptorTest {
    @Test
    public void test_applicable() {
        CnwareCopyCommonInterceptor provider = new CnwareCopyCommonInterceptor();
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.CNWARE_VM.getType()));
    }
}