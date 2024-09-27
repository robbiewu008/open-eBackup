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
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.concurrent.TimeUnit;

/**
 * 功能描述
 *
 * @author w00504341
 * @since 2021-07-21
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {TimeoutUtils.class})
public class TimeoutUtilsTest {
    @Autowired
    private TimeoutUtils timeoutUtils;

    @Test
    public void checkTimeoutFalse() {
        boolean res = timeoutUtils.checkTimeout("123", true);
        Assert.assertFalse(res);
        timeoutUtils.destroy("123");
    }

    @Test
    public void checkTimeoutTrue() throws Exception {
        timeoutUtils.checkTimeout("123", true);
        timeoutUtils.setTimeout(0);

        TimeUnit.MILLISECONDS.sleep(10L);

        boolean res = timeoutUtils.checkTimeout("123", true);
        Assert.assertTrue(res);
        timeoutUtils.destroy("123");
    }
}
