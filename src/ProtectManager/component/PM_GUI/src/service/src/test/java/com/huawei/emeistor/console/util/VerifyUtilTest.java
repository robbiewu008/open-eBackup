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
package com.huawei.emeistor.console.util;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-09-14
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {VerifyUtil.class})
public class VerifyUtilTest {

    @Test
    public void test_is_empty() {
        Assert.assertTrue(VerifyUtil.isEmpty(Collections.emptyMap()));
        Assert.assertTrue(VerifyUtil.isEmpty(""));
        Assert.assertTrue(VerifyUtil.isEmpty(Collections.emptyList()));
        Assert.assertFalse(VerifyUtil.isEmpty(new Object()));
        Assert.assertFalse(VerifyUtil.isArrayObject(null));
        Assert.assertTrue(VerifyUtil.isArrayObject(Collections.singletonList("test")));
    }
}
