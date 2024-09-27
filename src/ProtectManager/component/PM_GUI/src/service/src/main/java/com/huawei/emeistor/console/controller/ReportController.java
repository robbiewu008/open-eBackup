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
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.RestTemplate;

import java.text.MessageFormat;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * 功能描述
 *
 * @author z00445440
 * @since 2023-01-09
 */
@RestController
@RequestMapping(ConfigConstant.CONSOLE)
class ReportController {
    private static final String REPORT_DOWNLOAD = "/v1/report/{0}/action/download";

    private static final String REPORT_BATCH_DOWNLOAD = "/v1/report/action/batchDownload";

    @Value("${api.gateway.endpoint}")
    private String reportApi;

    @Autowired
    private RequestUtil requestUtil;

    @Autowired
    private RestTemplate restTemplate;

    @Autowired
    private HttpServletRequest request;

    @Autowired
    private HttpServletResponse response;

    /**
     * 下载报表
     *
     * @param requestEntity RequestEntity
     * @param reportId reportId
     * @return byte[]
     */
    @ExterAttack
    @GetMapping("/v1/report/{reportId}/action/download")
    public byte[] downloadReport(@PathVariable String reportId, RequestEntity requestEntity) {
        String downloadUrl = MessageFormat.format(reportApi + REPORT_DOWNLOAD, reportId);
        return getBytes(downloadUrl, requestEntity);
    }

    /**
     * 批量下载报表
     *
     * @param requestEntity RequestEntity
     * @return byte[]
     */
    @ExterAttack
    @GetMapping("/v1/report/action/batchDownload")
    public byte[] batchDownloadReport(RequestEntity requestEntity) {
        return getBytes(reportApi + REPORT_BATCH_DOWNLOAD + "?" + request.getQueryString(), requestEntity);
    }

    @ExterAttack
    private byte[] getBytes(String url, RequestEntity requestEntity) {
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        HttpEntity<MultiValueMap<String, FileSystemResource>> httpEntity = new HttpEntity(requestEntity.getBody(),
            headers);
        ResponseEntity<byte[]> responseEntity = restTemplate.exchange(NormalizerUtil.normalizeForString(url),
            HttpMethod.GET, httpEntity, byte[].class);
        response.setStatus(responseEntity.getStatusCodeValue());
        response.setHeader(ConfigConstant.CONTENT_DISPOSITION,
            responseEntity.getHeaders().getContentDisposition().toString());
        return responseEntity.getBody();
    }
}