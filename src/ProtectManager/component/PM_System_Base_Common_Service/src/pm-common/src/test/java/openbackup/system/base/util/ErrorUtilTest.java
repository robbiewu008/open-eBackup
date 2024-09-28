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
import openbackup.system.base.util.ErrorUtil;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.net.UnknownHostException;

/**
 * ErrorUtilTest
 *
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
