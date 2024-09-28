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

import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.controller.request.SSOConfigRequest;
import com.huawei.emeistor.console.service.SessionService;
import com.huawei.emeistor.console.util.DownloadUtil;
import com.huawei.emeistor.console.util.EncryptorRestClient;
import com.huawei.emeistor.console.util.RequestUtil;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.util.MultiValueMap;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.multipart.MultipartFile;

/**
 * SSO功能配置测试
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {
    SSOConfigController.class, RestTemplate.class, RequestUtil.class
})
public class SSOConfigControllerTest {

    @Autowired
    private SSOConfigController ssoConfigController;

    @MockBean
    private RestTemplate restTemplate;

    @MockBean
    private DownloadUtil downloadUtil;

    @MockBean
    private SessionService sessionService;

    @MockBean
    private EncryptorRestClient encryptorRestClient;

    @Test
    public void test_save_sso_config_success() {
        MultipartFile file = PowerMockito.mock(MultipartFile.class);
        SSOConfigRequest ssoConfigRequest = new SSOConfigRequest();
        ssoConfigRequest.setConfigName("test01");
        ssoConfigRequest.setFile(file);
        ssoConfigRequest.setDescription("Test");
        ResponseEntity<Object> responseEntity = new ResponseEntity<>(HttpStatus.OK);
        PowerMockito.when(restTemplate.postForEntity(anyString(), any(), any())).thenReturn(responseEntity);
        ssoConfigController.createSSOConfig(ssoConfigRequest);
    }

    @Test
    public void test_update_sso_config_success() {
        MultipartFile file = PowerMockito.mock(MultipartFile.class);
        SSOConfigRequest ssoConfigRequest = new SSOConfigRequest();
        ssoConfigRequest.setConfigName("test01");
        ssoConfigRequest.setFile(file);
        ssoConfigRequest.setDescription("Test");
        MultiValueMap<String, String> headers =  new HttpHeaders();
        MediaType contentType = new MediaType(ConfigConstant.CONTENT_TYPE);
        MediaType disType = new MediaType(ConfigConstant.CONTENT_DISPOSITION);
        headers.add("Content-Type", contentType.toString());
        headers.add("Content-Disposition", disType.toString());
        ResponseEntity<Object> responseEntity = new ResponseEntity<>(headers, HttpStatus.OK);
        PowerMockito.when(restTemplate.exchange(anyString(), any(), any(), eq(Object.class)))
            .thenReturn(responseEntity);
        ssoConfigController.updateSSOConfig(ssoConfigRequest);
    }

    @Test
    public void test_download_metadata() {
        PowerMockito.doNothing().when(downloadUtil).download(anyString());
        ssoConfigController.getMetadata();
    }
}
