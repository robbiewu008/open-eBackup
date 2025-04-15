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

import static org.mockito.ArgumentMatchers.any;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;

import org.apache.commons.io.FileUtils;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.client.RestTemplate;

import java.net.URI;
import java.security.KeyStore;
import java.util.Arrays;
import java.util.Enumeration;

import javax.servlet.http.HttpServletRequest;

/**
 * KeyToolUtilTest
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {KeyToolUtil.class, FileUtils.class, KeyStore.class})
public class ForwardOtherNodeUtilTest {
    private static final String type = "PKCS12";
    private final InfrastructureRestApi infrastructureRestApi = PowerMockito.mock(InfrastructureRestApi.class);
    private final RestTemplate restTemplate = PowerMockito.mock(RestTemplate.class);
    @InjectMocks
    private ForwardOtherNodeUtil forwardOtherNodeUtil;

    /**
     * 用例场景：尝试转发给其他节点成功
     * 前置条件：服务正常
     * 检查点：返回成功相等
     */
    @Test
    public void should_try_request_to_other_nodes_success() throws Exception {
        HttpServletRequest request = mockRequest();
        InfraResponseWithError infraResponseWithError = new InfraResponseWithError();
        infraResponseWithError.setData(Arrays.asList("127.0.0.1"));
        PowerMockito.when(infrastructureRestApi.getEndpoints(any())).thenReturn(infraResponseWithError);
        ResponseEntity<String> responseEntity = new ResponseEntity<>(HttpStatus.OK);
        HttpHeaders headers = new HttpHeaders();
        headers.add("internal-retry", "internal-retry");
        PowerMockito.when(restTemplate.exchange(new URI("https://127.0.0.1:30081/v1/email/test"), HttpMethod.GET, new HttpEntity<>("", headers),
            String.class)).thenReturn(responseEntity);
        LegoCheckedException exception = new LegoCheckedException(CommonErrorCode.CONNECT_LDAP_SERVER_FAILED);
        String req = forwardOtherNodeUtil.tryRequestToOtherNodes(request, exception, "", "testEmail");
        Assert.assertEquals("SUCCESS", req);
    }
    private HttpServletRequest mockRequest() {
        HttpServletRequest request =  PowerMockito.mock(HttpServletRequest.class);
        PowerMockito.when(request.getRequestURI()).thenReturn("/v1/email/test");
        PowerMockito.when(request.getMethod()).thenReturn("GET");
        Enumeration<String> list = new Enumeration<String>() {
            @Override
            public boolean hasMoreElements() {
                return false;
            }

            @Override
            public String nextElement() {
                return null;
            }
        };
        PowerMockito.when(request.getHeaderNames()).thenReturn(list);
        return request;
    }
    /**
     * 用例场景：尝试转发给其他节点失败
     * 前置条件：服务正常
     * 检查点：抛出异常
     */
    @Test
    public void should_try_request_to_other_nodes_fail_when_system_error() throws Exception {
        HttpServletRequest request = mockRequest();
        InfraResponseWithError infraResponseWithError = new InfraResponseWithError();
        infraResponseWithError.setData(Arrays.asList("127.0.0.1"));
        PowerMockito.when(infrastructureRestApi.getEndpoints(any())).thenReturn(infraResponseWithError);

        ResponseEntity<String> responseEntity = new ResponseEntity<>(HttpStatus.OK);
        HttpHeaders headers = new HttpHeaders();
        headers.add("internal-retry", "internal-retry");
        PowerMockito.when(restTemplate.exchange(new URI("https://127.0.0.1:30081/v1/email/test"), HttpMethod.GET, new HttpEntity<>("", headers),
            String.class)).thenReturn(responseEntity);
        LegoCheckedException exception = new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR);
        Assert.assertThrows(LegoCheckedException.class, ()->forwardOtherNodeUtil.tryRequestToOtherNodes(request, exception, "", "testEmail"));
    }
}
