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
package com.huawei.emeistor.console.controller;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.mock.web.MockHttpServletResponse;

import java.io.IOException;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-09-22
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {
    ErrorPageController.class
})
public class ErrorPageControllerTest {
    @InjectMocks
    private ErrorPageController errorPageController;

    @Test
    public void test_not_found() throws IOException {
        MockHttpServletResponse mockHttpServletResponse = new MockHttpServletResponse();
        errorPageController.notFound(mockHttpServletResponse);
    }
}
