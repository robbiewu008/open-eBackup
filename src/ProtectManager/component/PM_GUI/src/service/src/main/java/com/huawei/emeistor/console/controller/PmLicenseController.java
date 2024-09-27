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

import com.huawei.emeistor.console.contant.CommonErrorCode;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.util.NormalizerUtil;
import com.huawei.emeistor.console.util.RequestUtil;

import com.alibaba.fastjson.JSONObject;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.io.IOUtils;
import org.apache.http.HttpStatus;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.core.io.Resource;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.util.LinkedMultiValueMap;
import org.springframework.util.MultiValueMap;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;

import javax.servlet.ServletOutputStream;
import javax.servlet.http.HttpServletResponse;

/**
 * 转发license导入接口
 *
 * @author w00574036
 * @since 2023-02-09
 */
@RestController
@RequestMapping(ConfigConstant.CONSOLE)
@Slf4j
public class PmLicenseController {
    private static final String LICENSE_IMPORT_URL = "/v1/license/action/import";

    private static final String LICENSE_EXPORT_URL = "/v1/license/action/export";

    @Autowired
    private RestTemplate restTemplate;

    @Autowired
    private HttpServletResponse response;

    @Autowired
    private RequestUtil requestUtil;

    @Value("${api.gateway.endpoint}")
    private String apiGatewayUrl;

    /**
     * import license
     *
     * @param basicLicenseFile license file
     * @throws IOException service exception
     */
    @ExterAttack
    @PostMapping("/v1/license/action/import")
    public void importLicense(@RequestParam("basic_license_file") MultipartFile basicLicenseFile) {
        log.info("import license to pm.");
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        headers.setContentType(MediaType.MULTIPART_FORM_DATA);
        String requestUrl = NormalizerUtil.normalizeForString(apiGatewayUrl + LICENSE_IMPORT_URL);

        // 转发license文件
        MultiValueMap<String, Object> map = new LinkedMultiValueMap<>();
        Resource uploadResource = basicLicenseFile.getResource();
        map.add("basic_license_file", uploadResource);
        HttpEntity<MultiValueMap<String, Object>> httpEntity = new HttpEntity<>(map, headers);
        ResponseEntity<Object> responseEntity = restTemplate.postForEntity(requestUrl, httpEntity, Object.class);

        // 封装并返回数据处理
        response.setStatus(HttpStatus.SC_OK);
        String pmResponse = JSONObject.toJSONString(responseEntity.getBody());
        try (ServletOutputStream outputStream = response.getOutputStream()) {
            outputStream.write(pmResponse.getBytes(StandardCharsets.UTF_8));
            outputStream.flush();
        } catch (IOException ioException) {
            log.error("import license to pm error.");
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR);
        }
        log.info("import license to pm end.");
    }

    /**
     * export license
     */
    @ExterAttack
    @GetMapping("/v1/license/action/export")
    public void exportLicense() {
        HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
        String exportLicenseUrl = NormalizerUtil.normalizeForString(apiGatewayUrl + LICENSE_EXPORT_URL);
        restTemplate.execute(exportLicenseUrl, HttpMethod.GET,
            clientHttpRequest -> clientHttpRequest.getHeaders().setAll(headers.toSingleValueMap()),
            clientHttpResponse -> {
                try (InputStream inputStream = clientHttpResponse.getBody();
                    OutputStream outputStream = response.getOutputStream()) {
                    IOUtils.copy(inputStream, outputStream);
                } catch (IOException e) {
                    log.error("gui export License failed.");
                    throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR);
                }
                log.info("export license end.");
                return null;
            }
        );
    }
}