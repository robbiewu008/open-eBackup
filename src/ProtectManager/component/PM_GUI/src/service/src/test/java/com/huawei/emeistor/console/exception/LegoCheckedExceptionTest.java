/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
 */

package com.huawei.emeistor.console.exception;

import com.huawei.emeistor.console.contant.CommonErrorCode;

import org.junit.Assert;
import org.junit.Test;
import org.springframework.http.HttpStatus;

/**
 * 功能描述: LegoCheckedExceptionTest
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2023-2-28
 */
public class LegoCheckedExceptionTest {
    @Test
    public void testBuildLegoCheckedException() {
        Assert.assertEquals("test", new LegoCheckedException("test").getMessage());
        Assert.assertEquals("test", new LegoCheckedException("test", new RuntimeException()).getMessage());
        Assert.assertEquals("code",
                new LegoCheckedException(HttpStatus.INTERNAL_SERVER_ERROR, "code", "test").getEmeiStorErrCode());
        Assert.assertEquals(CommonErrorCode.SYSTEM_ERROR,
                new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR).getErrorCode());
        Assert.assertEquals(CommonErrorCode.SYSTEM_ERROR,
                new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, new String[]{}).getErrorCode());
        Assert.assertEquals(CommonErrorCode.SYSTEM_ERROR,
                new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, new String[]{}, new RuntimeException())
                        .getErrorCode());
        Assert.assertEquals(CommonErrorCode.SYSTEM_ERROR,
                new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, new String[]{}, "message").getErrorCode());
        Assert.assertEquals(CommonErrorCode.SYSTEM_ERROR,
                new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, new RuntimeException()).getErrorCode());
        Assert.assertEquals(CommonErrorCode.SYSTEM_ERROR,
                new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "info").getErrorCode());
        Assert.assertEquals(CommonErrorCode.SYSTEM_ERROR,
                new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "info", new RuntimeException()).getErrorCode());
    }
}