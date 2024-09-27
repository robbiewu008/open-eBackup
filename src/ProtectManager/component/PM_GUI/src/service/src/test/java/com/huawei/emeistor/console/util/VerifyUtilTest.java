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
