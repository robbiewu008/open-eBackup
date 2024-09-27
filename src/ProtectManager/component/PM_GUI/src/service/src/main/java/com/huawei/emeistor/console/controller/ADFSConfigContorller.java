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
import com.huawei.emeistor.console.controller.request.ADFSConfigRequest;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.util.NormalizerUtil;
import com.huawei.emeistor.console.util.RequestUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.lang3.StringUtils;
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
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.RestTemplate;

import java.lang.reflect.InvocationTargetException;
import java.util.Map;
import java.util.Objects;

import javax.servlet.http.HttpServletResponse;
import javax.validation.Valid;

/**
 * ADFS保存配置
 *
 * @author y30021475
 * @since 2023-05-17
 */
@Slf4j
@RestController
@RequestMapping(ConfigConstant.CONSOLE)
public class ADFSConfigContorller {
    @Value("${api.gateway.endpoint}")
    private String gatewayApi;

    @Autowired
    private RequestUtil requestUtil;

    @Autowired
    private RestTemplate restTemplate;

    @Autowired
    private HttpServletResponse response;

    /**
     * 保存ADFS配置
     *
     * @param adfsConfigRequest adfsConfigRequest
     */
    @ExterAttack
    @PutMapping("/v1/adfs/config")
    public void setADFSConfig(ADFSConfigRequest adfsConfigRequest) {
        log.info("Start to setADFSConfig:{}", adfsConfigRequest.getConfigName());
        sendHttpRequest(adfsConfigRequest, HttpMethod.PUT, "/v1/adfs/config");
    }

    /**
     * 校验配置是否成功
     *
     * @param adfsConfigRequest adfsConfigRequest
     */
    @ExterAttack
    @PostMapping("/v1/adfs/config/check")
    public void checkADFSConfig(@Valid ADFSConfigRequest adfsConfigRequest) {
        log.info("Start to checkADFSConfig:{}", adfsConfigRequest.getConfigName());
        sendHttpRequest(adfsConfigRequest, HttpMethod.POST, "/v1/adfs/config/check");
    }

    private void sendHttpRequest(ADFSConfigRequest adfsConfigRequest, HttpMethod httpMethod, String url) {
        try {
            HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
            headers.setContentType(MediaType.MULTIPART_FORM_DATA);
            MultiValueMap<String, Object> map = new LinkedMultiValueMap<>();
            if (Objects.nonNull(adfsConfigRequest.getCaFile())) {
                Resource fileResource = adfsConfigRequest.getCaFile().getResource();
                map.add("caFile", fileResource);
            }
            Map<String, String> describe = BeanUtils.describe(adfsConfigRequest);
            describe.forEach((key, value) -> {
                if (!StringUtils.equals(key, "caFile")) {
                    map.add(key, value);
                }
            });
            HttpEntity<MultiValueMap<String, Object>> httpEntity = new HttpEntity<>(map, headers);
            ResponseEntity<Object> responseEntity = restTemplate.exchange(
                NormalizerUtil.normalizeForString(gatewayApi + url), httpMethod, httpEntity,
                Object.class);
            response.setStatus(responseEntity.getStatusCodeValue());
        } catch (InvocationTargetException | IllegalAccessException | NoSuchMethodException e) {
            log.error("convert adfs config :{} error:", adfsConfigRequest.getConfigName(), e);
        }
    }
}
