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

import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.service.SessionService;
import com.huawei.emeistor.console.util.NormalizerUtil;
import com.huawei.emeistor.console.util.RequestUtil;

import lombok.SneakyThrows;
import lombok.extern.slf4j.Slf4j;

import org.apache.hc.core5.http.HttpStatus;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.core.io.Resource;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.MediaType;
import org.springframework.http.RequestEntity;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;
import org.springframework.util.StreamUtils;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.RequestCallback;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.multipart.MultipartFile;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * 描述
 *
 */
@RestController
@RequestMapping(ConfigConstant.CONSOLE + "/v1/host-agent")
@Slf4j
public class CrmFileProcessController {
    private static final String DELIMITER = ",";

    private static final String CRM_UPLOAD_FILE_URL = "/v1/host-agent";

    private static final String CRM_DOWNLOAD_FILE_URL = "/v1/host-agent/download";

    @Autowired
    private RestTemplate restTemplate;

    @Autowired
    private HttpServletResponse response;

    @Autowired
    private HttpServletRequest request;

    @Autowired
    private SessionService sessionService;

    @Autowired
    private RequestUtil requestUtil;

    @Value("${api.gateway.endpoint}")
    private String crmUrl;

    /**
     * 上传hostAgent文件
     *
     * @param agentClient 用户上传的安装包
     */
    @ExterAttack
    @PostMapping
    public void importAgent(@RequestParam("agentClient") MultipartFile agentClient) {
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        headers.setContentType(MediaType.MULTIPART_FORM_DATA);
        String requestUrl = NormalizerUtil.normalizeForString(this.crmUrl + CRM_UPLOAD_FILE_URL);
        Resource uploadResource = agentClient.getResource();
        MultiValueMap<String, Object> map = new LinkedMultiValueMap<>();
        map.add("agentClient", uploadResource);
        HttpEntity<MultiValueMap<String, Object>> httpEntity = new HttpEntity<>(map, headers);
        restTemplate.postForEntity(requestUrl, httpEntity, Object.class);
        response.setStatus(HttpStatus.SC_OK);
    }

    /**
     * 下载hostAgent安装包
     *
     * @param requestEntity 请求的实体类
     */
    @ExterAttack
    @SneakyThrows
    @PostMapping("/download")
    public void getHostAgentFile1(RequestEntity<Map<String, String>> requestEntity) {
        HttpEntity<Object> httpEntity = new HttpEntity<>(requestEntity.getBody(),
            requestUtil.getForwardHeaderAndValidCsrf());
        RequestCallback requestCallback = restTemplate.httpEntityCallback(httpEntity);
        String requestUrl = NormalizerUtil.normalizeForString(this.crmUrl + CRM_DOWNLOAD_FILE_URL);
        restTemplate.execute(requestUrl, HttpMethod.POST, requestCallback, clientHttpResponse -> {
            HttpHeaders responseHeaders = clientHttpResponse.getHeaders();
            responseHeaders.forEach((key, value) -> response.setHeader(key, String.join(DELIMITER, value)));
            try (OutputStream os = response.getOutputStream(); InputStream in = clientHttpResponse.getBody()) {
                StreamUtils.copy(in, os);
            }
            return null;
        });
    }
}
