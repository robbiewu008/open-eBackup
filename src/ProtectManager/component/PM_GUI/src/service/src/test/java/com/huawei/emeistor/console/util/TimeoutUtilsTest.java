/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
