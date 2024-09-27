/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
 * @author twx1009756
 * @since 2021-03-17
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
