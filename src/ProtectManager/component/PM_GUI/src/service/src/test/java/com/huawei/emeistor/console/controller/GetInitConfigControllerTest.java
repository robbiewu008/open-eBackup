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

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;

import com.huawei.emeistor.console.util.RequestUtil;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.http.HttpHeaders;
import org.springframework.http.ResponseEntity;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;

import javax.servlet.http.HttpServletResponse;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {GetInitConfigController.class, RequestUtil.class})
public class GetInitConfigControllerTest {
    @InjectMocks
    private GetInitConfigController getInitConfigController;

    @Mock
    private HttpServletResponse response;

    @Mock
    private RestTemplate restTemplate;

    @Mock
    private RequestUtil requestUtil;

    @Test
    public void test_get_init_config_success() throws IOException {
        MultipartFile multipartFile = PowerMockito.mock(MultipartFile.class);
        HttpHeaders headers = new HttpHeaders();
        PowerMockito.when(requestUtil.getForwardHeaderAndValidCsrf()).thenReturn(headers);
        ResponseEntity<Object> responseEntity = PowerMockito.mock(ResponseEntity.class);
        PowerMockito.when(responseEntity.getStatusCodeValue()).thenReturn(200);
        PowerMockito.when(restTemplate.postForEntity(anyString(), any(), eq(Object.class))).thenReturn(responseEntity);
        getInitConfigController.getInitConfig(multipartFile);
    }
}
