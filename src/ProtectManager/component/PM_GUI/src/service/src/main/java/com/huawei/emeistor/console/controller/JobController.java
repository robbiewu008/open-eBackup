/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.emeistor.console.controller;

import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.service.SessionService;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.HttpMethod;
import org.springframework.http.RequestEntity;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.RestTemplate;

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
class JobController extends AdvBaseController {
    private static final Logger LOGGER = LoggerFactory.getLogger(JobController.class);

    private static final String JOBS_EXPORT = "/v1/jobs/action/export";

    @Value("${api.gateway.endpoint}")
    private String jobApi;

    @Autowired
    private SessionService sessionService;

    @Autowired
    private RestTemplate restTemplate;

    @Autowired
    private HttpServletRequest request;

    @Autowired
    private HttpServletResponse response;

    /**
     * 导出告警
     *
     * @param requestEntity RequestEntity
     * @return byte[]
     */
    @ExterAttack
    @PostMapping("/v1/jobs/action/export")
    public byte[] exportAlarm(RequestEntity requestEntity) {
        return getBytes(jobApi + JOBS_EXPORT + "?" + request.getQueryString(), requestEntity);
    }

    private byte[] getBytes(String addrLink, RequestEntity requestEntity) {
        ResponseEntity<byte[]> responseEntity = restTemplate.exchange(super.normalizeForString(addrLink),
            HttpMethod.POST, super.getHttpEntity(requestEntity.getBody(), request), byte[].class);
        response.setStatus(responseEntity.getStatusCodeValue());
        response.setHeader(ConfigConstant.CONTENT_TYPE, responseEntity.getHeaders().getContentType().getType());
        response.setHeader(ConfigConstant.CONTENT_DISPOSITION,
            responseEntity.getHeaders().getContentDisposition().toString());
        return responseEntity.getBody();
    }
}
