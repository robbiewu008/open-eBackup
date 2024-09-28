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

import com.huawei.emeistor.console.bean.ClusterDetailInfo;
import com.huawei.emeistor.console.bean.SourceClustersParams;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.contant.ErrorResponse;
import com.huawei.emeistor.console.service.SessionService;
import com.huawei.emeistor.console.util.EncryptorRestClient;
import com.huawei.emeistor.console.util.RequestUtil;

import org.checkerframework.checker.units.qual.C;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.RedissonDeque;
import org.redisson.RedissonList;
import org.redisson.api.RList;
import org.redisson.api.RLock;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.util.MultiValueMap;
import org.springframework.web.client.RestTemplate;

import java.io.IOException;
import java.util.Map;

import javax.servlet.ServletOutputStream;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpServletResponseWrapper;

/**
 * 导出测试类
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {
    ExportRecordController.class, RestTemplate.class, RequestUtil.class, RedissonClient.class,
    EncryptorRestClient.class, SessionService.class
})
public class ExportRecordControllerTest {
    @Autowired
    private ExportRecordController exportRecordController;

    @MockBean
    private RestTemplate restTemplate;

    @MockBean
    private RedissonClient redissonClient;

    @MockBean
    private EncryptorRestClient encryptorRestClient;

    @MockBean
    private SessionService sessionService;

    @Test
    public void test_download_export_file_success() throws IOException {
        HttpServletResponse servletResponse = PowerMockito.mock(HttpServletResponse.class);
        ServletOutputStream stream = PowerMockito.mock(ServletOutputStream.class);
        PowerMockito.when(servletResponse.getOutputStream()).thenReturn(stream);
        PowerMockito.when(restTemplate.exchange(anyString(), any(), any(), eq(ClusterDetailInfo.class)))
            .thenReturn(getClusterDetailInfo());
        ResponseEntity<Object> responseObjectEntity = new ResponseEntity(null, HttpStatus.OK);
        PowerMockito.when(restTemplate.postForEntity(anyString(), any(), eq(Object.class)))
            .thenReturn(responseObjectEntity);
        RLock rLock = PowerMockito.mock(RLock.class);
        PowerMockito.when(redissonClient.getLock(any())).thenReturn(rLock);
        RList<Object> readFileIdList = PowerMockito.mock(RList.class);
        PowerMockito.when(redissonClient.getList(anyString())).thenReturn(readFileIdList);
        ResponseEntity<ErrorResponse> responseEntity = exportRecordController.downloadExportFile("1", "12", servletResponse);
        Assert.assertEquals(HttpStatus.OK, responseEntity.getStatusCode());
    }

    @Test
    public void test_download_export_file_success_when_get_download_thread_limit() throws IOException {
        HttpServletResponse servletResponse = PowerMockito.mock(HttpServletResponse.class);
        ServletOutputStream stream = PowerMockito.mock(ServletOutputStream.class);
        PowerMockito.when(servletResponse.getOutputStream()).thenReturn(stream);
        PowerMockito.when(restTemplate.exchange(anyString(), any(), any(), eq(ClusterDetailInfo.class)))
            .thenReturn(getClusterDetailInfo());
        ResponseEntity<Object> responseObjectEntity = null;
        PowerMockito.when(restTemplate.postForEntity(anyString(), any(), eq(Object.class)))
            .thenReturn(responseObjectEntity);
        RLock rLock = PowerMockito.mock(RLock.class);
        PowerMockito.when(redissonClient.getLock(any())).thenReturn(rLock);
        RList<Object> readFileIdList = PowerMockito.mock(RList.class);
        PowerMockito.when(redissonClient.getList(anyString())).thenReturn(readFileIdList);
        ResponseEntity<ErrorResponse> responseEntity = exportRecordController.downloadExportFile("1", "12", servletResponse);
        Assert.assertEquals(HttpStatus.OK, responseEntity.getStatusCode());
    }

    private ResponseEntity<ClusterDetailInfo> getClusterDetailInfo() {
        SourceClustersParams clustersParams = new SourceClustersParams();
        clustersParams.setNodeCount(1);
        ClusterDetailInfo clusterDetailInfo = new ClusterDetailInfo();
        clusterDetailInfo.setSourceClusters(clustersParams);
        MultiValueMap<String, String> headers = getHeaders();
        ResponseEntity<ClusterDetailInfo> entity = new ResponseEntity(clusterDetailInfo, headers, HttpStatus.OK);
        return entity;
    }

    private MultiValueMap<String, String> getHeaders() {
        MediaType contentType = new MediaType(ConfigConstant.CONTENT_TYPE);
        MediaType disType = new MediaType(ConfigConstant.CONTENT_DISPOSITION);
        MultiValueMap<String, String> headers = new HttpHeaders();
        headers.add("Content-Type", contentType.toString());
        headers.add("Content-Disposition", disType.toString());
        return headers;
    }
}
