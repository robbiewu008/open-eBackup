/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.util;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.util.ErrorUtil;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.net.UnknownHostException;

/**
 * ErrorUtilTest
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-25
 */
public class ErrorUtilTest {
    /**
     * 用例场景：判断是否为连接异常
     * 前置条件：任务抛出异常
     * 检查点：返回true or false
     */
    @Test
    public void judgment_retryable_exception_success() {
        Assert.assertTrue(ErrorUtil.isRetryableException(PowerMockito.mock(UnknownHostException.class)));
        Assert.assertFalse(ErrorUtil.isRetryableException(PowerMockito.mock(LegoCheckedException.class)));
    }
}
