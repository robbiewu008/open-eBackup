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
package openbackup.system.base.util;

import openbackup.system.base.util.OrderNoUtil;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

/**
 * 功能描述
 *
 * @author w00607005
 * @since 2023-07-07
 */
@RunWith(PowerMockRunner.class)
@PowerMockIgnore( {"javax.management.*", "jdk.internal.reflect.*"})
@PrepareForTest(value = {DateTimeFormatter.class, LocalDateTime.class})
public class OrderNoUtilTest {
    @Test
    public void test_get_task_no() throws Exception {
        // run the test
        String result = OrderNoUtil.getTaskNo();

        // verify the results
        Assert.assertNotNull(result);
    }
}