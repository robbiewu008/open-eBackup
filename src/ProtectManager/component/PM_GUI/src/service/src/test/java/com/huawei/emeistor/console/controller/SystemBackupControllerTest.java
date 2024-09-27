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
import static org.mockito.Mockito.verify;

import com.huawei.emeistor.console.bean.Token;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.service.impl.SecurityPolicyServiceImpl;
import com.huawei.emeistor.console.service.impl.SessionServiceImpl;
import com.huawei.emeistor.console.util.EncryptorRestClient;
import com.huawei.emeistor.console.util.RequestUtil;
import com.huawei.emeistor.console.util.SHA256Encryptor;
import com.huawei.emeistor.console.util.TimeoutUtils;

import org.apache.http.entity.ContentType;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.mock.web.MockMultipartFile;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.util.MultiValueMap;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import javax.servlet.ServletOutputStream;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * 管理数据备份功能测试
 *
 * @author w00504341
 * @since 2021-03-15
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {SystemBackupController.class, SessionServiceImpl.class, RestTemplate.class,
    RequestUtil.class, SecurityPolicyServiceImpl.class, TimeoutUtils.class})
public class SystemBackupControllerTest {
    @Autowired
    private SystemBackupController systemBackupController;

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

    /**
     * 用例场景：文件上传成功
     * 前置条件：A8000服务正常，SystemBackup功能可用
     * 检查点：上传文件成功
     */
    @Test
    public void upload_backup_success() throws IOException {
        MockMultipartFile multipartFile = new MockMultipartFile("test.zip", "test.zip",
            ContentType.APPLICATION_JSON.getMimeType(), new byte[0]);
        HttpServletRequest servletRequest = PowerMockito.mock(HttpServletRequest.class);
        HttpServletResponse servletResponse = PowerMockito.mock(HttpServletResponse.class);
        PowerMockito.when(servletRequest.getHeader(anyString())).thenReturn("123");
        ServletOutputStream stream = PowerMockito.mock(ServletOutputStream.class);
        PowerMockito.when(servletResponse.getOutputStream()).thenReturn(stream);
        ResponseEntity<Map> responseEntity = getResponseMapEntity("123");
        ResponseEntity<Object> responseObjectEntity = getResponseObjectEntity();
        PowerMockito.when(restTemplate.exchange(anyString(), any(), any(), eq(Map.class)))
            .thenReturn(responseEntity);
        PowerMockito.when(restTemplate.postForEntity(anyString(), any(), eq(Object.class)))
            .thenReturn(responseObjectEntity);
        systemBackupController.uploadBackup(servletRequest, servletResponse, multipartFile, "",true);
        verify(restTemplate).postForEntity(anyString(), any(), eq(Object.class));
    }

    /**
     * 用例场景：文件名非法，上传失败
     * 前置条件：A8000服务正常，SystemBackup功能可用
     * 检查点：上传文件失败
     */
    @Test(expected = LegoCheckedException.class)
    public void should_throw_LegoCheckedException_when_file_name_invalid() throws IOException {
        MultipartFile file = PowerMockito.mock(MultipartFile.class);
        HttpServletRequest servletRequest = PowerMockito.mock(HttpServletRequest.class);
        HttpServletResponse servletResponse = PowerMockito.mock(HttpServletResponse.class);
        PowerMockito.when(servletRequest.getHeader(anyString())).thenReturn("123");
        PowerMockito.when(file.getOriginalFilename()).thenReturn("../test.zip");
        PowerMockito.when(file.getSize()).thenReturn(1024L);
        ServletOutputStream stream = PowerMockito.mock(ServletOutputStream.class);
        PowerMockito.when(servletResponse.getOutputStream()).thenReturn(stream);
        ResponseEntity<Map> responseEntity = getResponseMapEntity("123");
        ResponseEntity<Object> responseObjectEntity = getResponseObjectEntity();
        PowerMockito.when(restTemplate.exchange(anyString(), any(), any(), eq(Map.class)))
            .thenReturn(responseEntity);
        PowerMockito.when(restTemplate.postForEntity(anyString(), any(), eq(Object.class)))
            .thenReturn(responseObjectEntity);
        systemBackupController.uploadBackup(servletRequest, servletResponse, file, "",true);
    }

    /**
     * 用例场景：文件不存在，下载失败
     * 前置条件：A8000服务正常，SystemBackup功能可用
     * 检查点：下载文件失败
     */
    @Test
    public void should_download_success() {
        Map<String, String> map = new HashMap<>();
        MultiValueMap<String, String> headers = getHeaders();
        map.put("id", "123");
        HttpServletResponse servletResponse = PowerMockito.mock(HttpServletResponse.class);
        ResponseEntity<Map> responseEntity = new ResponseEntity(map, headers, HttpStatus.OK);
        PowerMockito.when(restTemplate.exchange(anyString(), any(), any(), eq(Map.class))).thenReturn(responseEntity);
        PowerMockito.when(restTemplate.execute(anyString(), any(), any(), any())).thenReturn(Object.class);
        systemBackupController.downloadBackup(servletResponse, 1L);
    }

    /**
     * 用例场景：文件为空，下载失败
     * 前置条件：A8000服务正常，SystemBackup功能可用
     * 检查点：下载文件失败
     */
    @Test(expected = LegoCheckedException.class)
    public void should_throw_LegoCheckedException_download_backup_file_when_body_is_empty() throws Exception {
        Map<String, String> map = new HashMap<>();
        MultiValueMap<String, String> headers = getHeaders();
        HttpServletResponse servletResponse = PowerMockito.mock(HttpServletResponse.class);
        ResponseEntity<Map> responseEntity = new ResponseEntity(map, headers, HttpStatus.OK);
        PowerMockito.when(restTemplate.exchange(anyString(), any(), any(), eq(Map.class)))
            .thenReturn(responseEntity);
        systemBackupController.downloadBackup(servletResponse, 1L);
    }

    private MultiValueMap<String, String> getHeaders() {
        MediaType contentType = new MediaType(ConfigConstant.CONTENT_TYPE);
        MediaType disType = new MediaType(ConfigConstant.CONTENT_DISPOSITION);
        MultiValueMap<String, String> headers = new HttpHeaders();
        headers.add("Content-Type", contentType.toString());
        headers.add("Content-Disposition", disType.toString());
        return headers;
    }

    private ResponseEntity<Map> getResponseMapEntity(String id) {
        Map<String, String> map = new HashMap<>();
        map.put("id", id);
        MultiValueMap<String, String> headers = getHeaders();
        ResponseEntity<Map> responseEntity = new ResponseEntity(map, headers, HttpStatus.OK);
        return responseEntity;
    }

    private ResponseEntity<Object> getResponseObjectEntity() {
        Token token = new Token();
        token.setToken("sadfgasfasf");
        ResponseEntity<Object> responseObjectEntity = new ResponseEntity(token, HttpStatus.OK);
        return responseObjectEntity;
    }
}