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
