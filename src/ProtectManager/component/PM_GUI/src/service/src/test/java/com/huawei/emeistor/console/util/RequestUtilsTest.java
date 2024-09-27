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
package com.huawei.emeistor.console.util;

import static org.mockito.ArgumentMatchers.anyString;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.http.HttpHeaders;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpSession;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-09-14
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {RequestUtils.class})
public class RequestUtilsTest {
    @Test
    public void test_get_forward_Header_and_valid_csrf_success() {
        HttpServletRequest request = PowerMockito.mock(HttpServletRequest.class);
        PowerMockito.when(request.getHeader(anyString())).thenReturn("test");
        HttpSession httpSession = PowerMockito.mock(HttpSession.class);
        PowerMockito.when(httpSession.getAttribute(anyString())).thenReturn("test");
        PowerMockito.when(request.getSession()).thenReturn(httpSession);
        HttpHeaders forwardHeaderAndValidCsrf = RequestUtils.getForwardHeaderAndValidCsrf(request);
        Assert.assertNotNull(forwardHeaderAndValidCsrf);
    }
}
