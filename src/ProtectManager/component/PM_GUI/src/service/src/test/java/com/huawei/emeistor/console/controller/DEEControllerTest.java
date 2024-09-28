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

import java.io.IOException;

import javax.servlet.ServletOutputStream;
import javax.servlet.http.HttpServletResponse;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;

import com.huawei.emeistor.console.controller.request.DownloadAbnormalRequest;
import com.huawei.emeistor.console.controller.request.ExportSuspectFileReportRequest;
import com.huawei.emeistor.console.service.impl.CaptchaServiceImpl;
import com.huawei.emeistor.console.service.impl.SecurityPolicyServiceImpl;
import com.huawei.emeistor.console.service.impl.SessionServiceImpl;
import com.huawei.emeistor.console.service.impl.UserServiceImpl;
import com.huawei.emeistor.console.util.EncryptorRestClient;
import com.huawei.emeistor.console.util.RequestUtil;
import com.huawei.emeistor.console.util.SHA256Encryptor;
import com.huawei.emeistor.console.util.TimeoutUtils;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.mock.web.MockHttpServletResponse;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.multipart.MultipartFile;

/**
 * DEE转发测试类
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {
    DEEController.class, SessionServiceImpl.class, SecurityPolicyServiceImpl.class, RestTemplate.class,
    RequestUtil.class, UserServiceImpl.class, CaptchaServiceImpl.class, TimeoutUtils.class
})
public class DEEControllerTest {
    /**
     * 预期异常
     */
    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @MockBean(name = "restTemplate")
    private RestTemplate restTemplate;

    @MockBean(name = "userRestTemplate")
    private RestTemplate userRestTemplate;

    @MockBean
    private RedissonClient redissonClient;

    @MockBean
    private EncryptorRestClient encryptorRestClient;

    @MockBean
    private SHA256Encryptor sha256Encryptor;

    @Autowired
    private DEEController deeController;

    @MockBean(name = "response")
    private HttpServletResponse response;

    @Test
    public void create_model_info_success() throws IOException {
        MultipartFile file = PowerMockito.mock(MultipartFile.class);
        ResponseEntity<Object> responseObjectEntity = new ResponseEntity(null, HttpStatus.OK);
        PowerMockito.when(restTemplate.postForEntity(anyString(), any(), eq(Object.class)))
            .thenReturn(responseObjectEntity);
        ServletOutputStream stream = PowerMockito.mock(ServletOutputStream.class);
        PowerMockito.when(response.getOutputStream()).thenReturn(stream);
        deeController.addModelInfo(file);
        verify(restTemplate).postForEntity(anyString(), any(), eq(Object.class));
    }

    /**
     * 用例场景：后端屏蔽上传防勒索模型接口
     * 前置条件：
     * 检查点：处理无异常
     */
    @Test
    public void create_model_info_404() {
        MultipartFile file = PowerMockito.mock(MultipartFile.class);
        ResponseEntity<Object> responseObjectEntity = new ResponseEntity(null, HttpStatus.NOT_FOUND);
        PowerMockito.when(restTemplate.postForEntity(anyString(), any(), eq(Object.class)))
                .thenReturn(responseObjectEntity);
        deeController.addModelInfo(file);
        verify(restTemplate).postForEntity(anyString(), any(), eq(Object.class));
        verify(response).setStatus(HttpStatus.NOT_FOUND.value());
    }

    /**
     * 用例场景：下载事中侦测报告成功
     * 前置条件：
     * 检查点：状态码200
     */
    @Test
    public void download_io_detect_report_success() {
        HttpServletResponse response = new MockHttpServletResponse();
        ExportSuspectFileReportRequest request = new ExportSuspectFileReportRequest();
        request.setDeviceId("deviceId");
        request.setVstoreId("vstoreId");
        request.setFileSystemName("fileSystemName");
        request.setSnapShotName("snapShotName");
        request.setLang("en");
        deeController.downloadIoDetectReport(request, response);
        verify(restTemplate).execute(anyString(), any(), any(), any());
        Assert.assertEquals(HttpStatus.OK.value(), response.getStatus());
    }

    /**
     * 用例场景：下载事中侦测报告失败
     * 前置条件：
     * 检查点：状态码不为200
     */
    @Test
    public void download_io_detect_report_failed() {
        HttpServletResponse response = new MockHttpServletResponse();
        response.setStatus(HttpStatus.BAD_REQUEST.value());
        ExportSuspectFileReportRequest request = new ExportSuspectFileReportRequest();
        request.setDeviceId("deviceId");
        request.setVstoreId("vstoreId");
        request.setFileSystemName("fileSystemName");
        request.setSnapShotName("snapShotName");
        request.setLang("en");
        deeController.downloadIoDetectReport(request, response);
        verify(restTemplate).execute(anyString(), any(), any(), any());
        Assert.assertNotEquals(HttpStatus.OK.value(), response.getStatus());
    }

    /**
     * 用例场景：下载事中侦测报告成功
     * 前置条件：
     * 检查点：状态码200
     */
    @Test
    public void download_abnormal_report_success() {
        HttpServletResponse response = new MockHttpServletResponse();
        DownloadAbnormalRequest request = new DownloadAbnormalRequest();
        request.setCopyId("c524");
        deeController.downloadAbnormalReport(request, response);
        verify(restTemplate).execute(anyString(), any(), any(), any());
        Assert.assertEquals(HttpStatus.OK.value(), response.getStatus());
    }
}
