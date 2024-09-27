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
package openbackup.data.access.framework.core.config;

import static org.mockito.ArgumentMatchers.anyString;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import openbackup.data.access.framework.core.config.DataProtectionAccessExceptionAdvice;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.ErrorResponse;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.controller.SecretController;
import openbackup.system.base.service.ConfigMapServiceImpl;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureWebMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.MessageSource;
import org.springframework.mock.web.MockHttpServletResponse;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.request.MockHttpServletRequestBuilder;
import org.springframework.test.web.servlet.request.MockMvcRequestBuilders;
import org.springframework.test.web.servlet.result.MockMvcResultHandlers;
import org.springframework.test.web.servlet.setup.MockMvcBuilders;

import java.lang.reflect.Field;

/**
 * DataProtectionAccessExceptionAdviceTest
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-29
 */
@RunWith(SpringRunner.class)
@AutoConfigureWebMvc
@AutoConfigureMockMvc
@SpringBootTest(classes = {SecretController.class})
public class DataProtectionAccessExceptionAdviceTest {
    private static final String URL = "/v1/secret/redis";
    private final DataProtectionAccessExceptionAdvice advice = new DataProtectionAccessExceptionAdvice();

    @Autowired
    private MockMvc mockMvc;

    @Autowired
    private SecretController controller;

    @MockBean
    private ConfigMapServiceImpl configMapService;

    @MockBean
    private MessageSource messageSource;

    @Before
    public void setUp() throws IllegalAccessException {
        mockMvc = MockMvcBuilders.standaloneSetup(controller).setControllerAdvice(advice).build();
        Field messageSourceField = PowerMockito.field(DataProtectionAccessExceptionAdvice.class, "messageSource");
        messageSourceField.set(advice, messageSource);
        PowerMockito.when(messageSource.getMessage(CommonErrorCode.ACCESS_DENIED + "", null, null)).thenReturn("");
    }

    /**
     * 用例场景：全局捕获异常处理
     * 前置条件：Controller业务抛出DataProtectionAccessException
     * 检查点：切面捕获并处理异常返回ErrorResponse
     */
    @Test
    public void handle_LegoCheckedException_success_when_controller_throw_DataProtectionAccessException_with_errorCode()
        throws Exception {
        PowerMockito.doThrow(new DataProtectionAccessException(CommonErrorCode.ACCESS_DENIED, new String[] {}))
            .when(configMapService).getValueFromSecretByKey(anyString());
        MockHttpServletRequestBuilder requestBuilder = MockMvcRequestBuilders.get(URL);
        MockHttpServletResponse response = mockMvc.perform(requestBuilder)
            .andExpect(status().isForbidden()).andDo(MockMvcResultHandlers.print()).andReturn().getResponse();
        ErrorResponse errorResponse = JSONObject.cast(response.getContentAsString(), ErrorResponse.class);
        Assert.assertEquals(CommonErrorCode.ACCESS_DENIED + "", errorResponse.getErrorCode());
    }
}
