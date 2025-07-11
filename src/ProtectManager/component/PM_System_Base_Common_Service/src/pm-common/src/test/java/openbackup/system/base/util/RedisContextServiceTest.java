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

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.pack.lock.LockService;
import openbackup.system.base.util.RedisContextService;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;

import javax.annotation.Resource;

/**
 * Redis Context Service Test
 *
 */
@RunWith(MockitoJUnitRunner.class)
@SpringBootTest(classes = {RedisContextService.class, RedissonClient.class, LockService.class})
public class RedisContextServiceTest {
    private static final String LOCK = "lock";

    @Autowired
    @InjectMocks
    RedisContextService redisContextService;

    @Resource
    @Mock
    private RedissonClient redissonClient;

    @Resource
    @Mock
    private LockService lockService;

    /**
     * 测试FindProvider方法
     */
    @Test
    public void test_redis_context_service() {
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(
                        redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
                .thenReturn(map);
        try {
            String requestId = "266ea41d-adf5-480b-af50-15b940c2b846";
            redisContextService.updateStringValue(requestId, LOCK, null);
            redisContextService.update(requestId, LOCK, JSONObject.fromObject(requestId));
            redisContextService.delete(requestId);
            redisContextService.set(JSONObject.fromObject(requestId));
            redisContextService.set("request_id", "266ea41d-adf5-480b-af50-15b940c2b846");
            redisContextService.set("request_id", null);
            redisContextService.set("request_id", map);
            redisContextService.get("request_id");
        } catch (LegoCheckedException ex) {
            ex.printStackTrace();
        }
    }
}
