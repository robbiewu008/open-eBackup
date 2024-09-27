/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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