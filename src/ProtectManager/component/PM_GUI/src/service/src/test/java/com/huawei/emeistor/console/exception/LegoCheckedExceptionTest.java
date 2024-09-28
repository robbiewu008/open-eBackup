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
package com.huawei.emeistor.console.exception;

import com.huawei.emeistor.console.contant.CommonErrorCode;

import org.junit.Assert;
import org.junit.Test;
import org.springframework.http.HttpStatus;

/**
 * 功能描述: LegoCheckedExceptionTest
 *
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
