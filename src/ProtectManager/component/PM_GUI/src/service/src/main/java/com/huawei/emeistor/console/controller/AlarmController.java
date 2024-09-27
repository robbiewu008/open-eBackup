/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.emeistor.console.controller;

import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.util.NormalizerUtil;
import com.huawei.emeistor.console.util.RequestUtil;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.core.io.FileSystemResource;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.RequestEntity;
import org.springframework.http.ResponseEntity;
import org.springframework.util.MultiValueMap;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.RestTemplate;

import java.io.File;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * 功能描述
 *
 * @author y30000858
 * @since 2020-07-10
 */
@RestController
@RequestMapping(ConfigConstant.CONSOLE)
class AlarmController {
    private static final String EVENTS_EXPORT = "/v1/events/action/export";

    private static final String ALARM_DUMP = "/v1/alarms/dump/files";

    @Value("${api.gateway.endpoint}")
    private String alarmApi;

    @Autowired
    private RestTemplate restTemplate;

    @Autowired
    private HttpServletResponse response;

    @Autowired
    private HttpServletRequest request;

    @Autowired
    private RequestUtil requestUtil;

    /**
     * 功能描述
     *
     * @param requestEntity RequestEntity
     * @return byte[]
     */
    @PostMapping("/v1/events/action/export")
    public byte[] exportEvents(RequestEntity requestEntity) {
        return getBytes(alarmApi + EVENTS_EXPORT + "?" + request.getQueryString(), requestEntity);
    }

    /**
     * 转储导出
     *
     * @param fileId 文件id
     * @param requestEntity RequestEntity
     * @return byte[]
     */
    @PostMapping("/v1/alarms/dump/files/{fileId}")
    public byte[] exportDumpFile(@PathVariable String fileId, RequestEntity requestEntity) {
        return getBytes(alarmApi + ALARM_DUMP + File.separator + fileId, requestEntity);
    }

    @ExterAttack
    private byte[] getBytes(String url, RequestEntity requestEntity) {
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        HttpEntity<MultiValueMap<String, FileSystemResource>> httpEntity =
            new HttpEntity(requestEntity.getBody(), headers);
        ResponseEntity<byte[]> responseEntity =
            restTemplate.exchange(NormalizerUtil.normalizeForString(url), HttpMethod.POST, httpEntity, byte[].class);
        response.setStatus(responseEntity.getStatusCodeValue());
        response.setHeader(ConfigConstant.CONTENT_TYPE, responseEntity.getHeaders().getContentType().getType());
        response.setHeader(ConfigConstant.CONTENT_DISPOSITION,
            responseEntity.getHeaders().getContentDisposition().toString());
        return responseEntity.getBody();
    }
}
