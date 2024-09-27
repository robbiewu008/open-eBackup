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
package openbackup.system.base.config;

import openbackup.system.base.config.RedissonClientConfig;

import org.junit.Assert;
import org.junit.Test;
import org.springframework.test.util.ReflectionTestUtils;

/**
 * RedissonClientConfigTest
 */
public class RedissonClientConfigTest {

    /**
     * 用例场景：域名底座正常转为IP底座
     * 前置条件：域名底座
     * 检查点：转换成功
     */
    @Test
    public void testConvertServerAddress() {
        String serverAddress = "rediss://localhost:6369";
        RedissonClientConfig redissionClientConfig = new RedissonClientConfig();
        Object convertedServerAddress = ReflectionTestUtils.invokeMethod(redissionClientConfig, "convertServerAddress",
            serverAddress);
        Assert.assertEquals("rediss://127.0.0.1:6369", convertedServerAddress);
    }
}