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
package com.huawei.emeistor.console.listener;

import static org.mockito.ArgumentMatchers.any;

import com.alibaba.fastjson.JSON;
import com.huawei.emeistor.console.bean.SessionInfo;
import com.huawei.emeistor.console.service.SessionService;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.RedissonBucket;
import org.redisson.api.RBucket;
import org.redisson.api.RedissonClient;
import org.springframework.boot.DefaultApplicationArguments;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.List;

/**
 * 理升级问题，解决之前序列化到redis中的session数据，将其转换为json字符串保存 单元测试
 *
 * @author t30028453
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-28
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {SessionService.class, RedissonClient.class})
public class UpgradeOnLineSessionListenerTest {
    @MockBean
    private SessionService sessionService;

    @MockBean
    private RedissonClient redissonClient;
    @MockBean
    private DefaultApplicationArguments arguments;

    /**
     * 用例场景：序列化到redis中的session数据，将其转换为json字符串保存成功
     * 前置条件：mock
     * 检查点：序列化到redis中的session数据，将其转换为json字符串保存成功
     */
    @Test
    public void should_run_successful() throws Exception {
        // init
        UpgradeOnLineSessionListener listener = new UpgradeOnLineSessionListener(sessionService, redissonClient);
        List<String> list = new ArrayList<>();
        list.add("123");
        RBucket<Object> rBucket = PowerMockito.mock(RedissonBucket.class);
        SessionInfo sessionInfo = new SessionInfo();
        sessionInfo.setSessionId("111111");
        // mock
        PowerMockito.when(sessionService.getOnlineSessionIdList()).thenReturn(list);
        PowerMockito.when(redissonClient.getBucket(any())).thenReturn(rBucket);
        PowerMockito.when(rBucket.isExists()).thenReturn(true);
        PowerMockito.when(rBucket.get()).thenReturn(sessionInfo);

        listener.run(arguments);
        Assert.assertEquals("{\"expireTime\":0,\"sessionId\":\"111111\"}", JSON.toJSONString(rBucket.get()));
    }
}