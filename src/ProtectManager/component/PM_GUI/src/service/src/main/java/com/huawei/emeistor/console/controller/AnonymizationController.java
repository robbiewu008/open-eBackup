/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.emeistor.console.controller;

import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.service.SessionService;

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
 * @author z00560567
 * @since 2021-01-12
 */
@RestController
@RequestMapping(ConfigConstant.CONSOLE)
class AnonymizationController extends AdvBaseController {
    private static final String ANONYMIZATION_EXPORT = "/v1/anonymization/report/download";

    @Value("${api.gateway.endpoint}")
    private String anonymizationApi;

    @Autowired
    private SessionService sessionService;

    @Autowired
    private RestTemplate restTemplate;

    @Autowired
    private HttpServletResponse response;

    @Autowired
    private HttpServletRequest request;

    /**
     * 功能描述
     *
     * @param requestEntity RequestEntity
     * @return byte[]
     */
    @PostMapping("/v1/anonymization/report/download")
    public byte[] exportAnonymizationReport(RequestEntity requestEntity) {
        return getBytes(anonymizationApi + ANONYMIZATION_EXPORT + "?" + request.getQueryString(), requestEntity);
    }

    @ExterAttack
    private byte[] getBytes(String url, RequestEntity requestEntity) {
        ResponseEntity<byte[]> responseEntity = restTemplate.exchange(super.normalizeForString(url),
                HttpMethod.POST, super.getHttpEntity(requestEntity.getBody(), request), byte[].class);
        response.setStatus(responseEntity.getStatusCodeValue());
        response.setHeader(ConfigConstant.CONTENT_TYPE, responseEntity.getHeaders().getContentType().getType());
        response.setHeader(ConfigConstant.CONTENT_DISPOSITION,
            responseEntity.getHeaders().getContentDisposition().toString());
        return responseEntity.getBody();
    }
}
