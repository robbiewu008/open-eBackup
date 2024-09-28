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
package openbackup.system.base.controller;

import static org.mockito.ArgumentMatchers.anyString;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import openbackup.system.base.controller.SecretController;
import openbackup.system.base.service.ConfigMapServiceImpl;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureWebMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.mock.web.MockHttpServletResponse;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.request.MockHttpServletRequestBuilder;
import org.springframework.test.web.servlet.request.MockMvcRequestBuilders;
import org.springframework.test.web.servlet.result.MockMvcResultHandlers;

/**
 * SecretControllerTest
 *
 */
@RunWith(SpringRunner.class)
@AutoConfigureWebMvc
@AutoConfigureMockMvc
@SpringBootTest(classes = {SecretController.class})
public class SecretControllerTest {
    private static final String REDIS_AUTH_PWD = "test_pwd";
    private static final String URL = "/v1/secret/redis";

    @Autowired
    private MockMvc mockMvc;

    @MockBean
    private ConfigMapServiceImpl configMapService;

    /**
     * 用例场景：获取common-secret中Redis密码
     * 前置条件：服务正常
     * 检查点：返回加密的密码
     */
    @Test
    public void get_encrypt_redis_pwd_success() throws Exception {
        PowerMockito.when(configMapService.getValueFromSecretByKey(anyString())).thenReturn(REDIS_AUTH_PWD);
        MockHttpServletRequestBuilder requestBuilder = MockMvcRequestBuilders.get(URL);
        MockHttpServletResponse mockResponse = mockMvc.perform(requestBuilder)
            .andExpect(status().isOk())
            .andDo(MockMvcResultHandlers.print())
            .andReturn()
            .getResponse();
        Assert.assertEquals(REDIS_AUTH_PWD, mockResponse.getContentAsString());
    }
}
