/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.util;

import openbackup.system.base.util.IdUtil;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.UUID;

/**
 * IdUtil test
 *
 * @author w00607005
 * @since 2023-07-07
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {UUID.class})
@PowerMockIgnore( {"javax.management.*", "jdk.internal.reflect.*"})
public class IdUtilTest {
    @Test
    public void test_random_uuid() throws Exception {
        // run the test
        String result = IdUtil.randomUUID();

        // verify the results
        Assert.assertNotNull(result);
    }

    @Test
    public void test_simple_uuid() throws Exception {
        // run the test
        String result = IdUtil.simpleUUID();

        // verify the results
        Assert.assertNotNull(result);
    }
}