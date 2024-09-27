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

import com.huawei.emeistor.console.controller.request.KerberosCreateReq;
import com.huawei.emeistor.console.controller.request.KerberosUpdateReq;
import com.huawei.emeistor.console.controller.response.KerberosIDResp;
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

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-09-18
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {KerberosController.class, RequestUtil.class})
public class KerberosControllerTest {
    @InjectMocks
    private KerberosController kerberosController;

    @Mock
    private RestTemplate restTemplate;

    @Mock
    private RequestUtil requestUtil;

    @Test
    public void test_create_kerberos_success() {
        HttpHeaders headers = new HttpHeaders();
        PowerMockito.when(requestUtil.getForwardHeaderAndValidCsrf()).thenReturn(headers);
        ResponseEntity<KerberosIDResp> responseEntity = PowerMockito.mock(ResponseEntity.class);
        KerberosIDResp kerberosIDResp = new KerberosIDResp();
        PowerMockito.when(responseEntity.getBody()).thenReturn(kerberosIDResp);
        PowerMockito.when(restTemplate.exchange(anyString(), any(), any(), eq(KerberosIDResp.class)))
            .thenReturn(responseEntity);
        KerberosCreateReq kerberosCreateReq = new KerberosCreateReq();
        kerberosCreateReq.setCreateModel("testModel");
        kerberosCreateReq.setName("testName");
        kerberosCreateReq.setPrincipalName("principal");
        kerberosCreateReq.setPassword("password");
        MultipartFile krb5File = PowerMockito.mock(MultipartFile.class);
        MultipartFile keytabFile = PowerMockito.mock(MultipartFile.class);
        kerberosController.createKerberos(kerberosCreateReq, krb5File, keytabFile);
    }

    @Test
    public void test_update_kerberos_success() {
        HttpHeaders headers = new HttpHeaders();
        PowerMockito.when(requestUtil.getForwardHeaderAndValidCsrf()).thenReturn(headers);
        ResponseEntity<KerberosIDResp> responseEntity = PowerMockito.mock(ResponseEntity.class);
        KerberosIDResp kerberosIDResp = new KerberosIDResp();
        PowerMockito.when(responseEntity.getBody()).thenReturn(kerberosIDResp);
        PowerMockito.when(restTemplate.exchange(anyString(), any(), any(), eq(KerberosIDResp.class)))
            .thenReturn(responseEntity);
        KerberosUpdateReq kerberosUpdateReq = new KerberosUpdateReq();
        kerberosUpdateReq.setCreateModel("testModel");
        kerberosUpdateReq.setName("testName");
        kerberosUpdateReq.setPrincipalName("principal");
        kerberosUpdateReq.setPassword("password");
        MultipartFile krb5File = PowerMockito.mock(MultipartFile.class);
        MultipartFile keytabFile = PowerMockito.mock(MultipartFile.class);
        kerberosController.updateKerberos("testKerberosId",kerberosUpdateReq, krb5File, keytabFile);
    }
}
