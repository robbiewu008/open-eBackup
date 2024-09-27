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
import com.huawei.emeistor.console.controller.request.SSOConfigRequest;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.util.DownloadUtil;
import com.huawei.emeistor.console.util.ExceptionUtil;
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
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.RestTemplate;

import java.lang.reflect.InvocationTargetException;
import java.util.Map;
import java.util.Objects;

import javax.servlet.http.HttpServletResponse;

/**
 * SSO保存配置
 *
 * @author w30042425
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-08
 */
@Slf4j
@RestController
@RequestMapping(ConfigConstant.CONSOLE)
public class SSOConfigController {
    @Value("${api.gateway.endpoint}")
    private String gatewayApi;

    @Autowired
    private RequestUtil requestUtil;

    @Autowired
    private RestTemplate restTemplate;

    @Autowired
    private HttpServletResponse response;

    @Autowired
    private DownloadUtil downloadUtil;

    /**
     * 保存SSO配置
     *
     * @param ssoConfigRequest 请求体
     */
    @ExterAttack
    @PostMapping("/v1/sso/config")
    public void createSSOConfig(SSOConfigRequest ssoConfigRequest) {
        try {
            HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
            headers.setContentType(MediaType.MULTIPART_FORM_DATA);
            Resource fileResource = ssoConfigRequest.getFile().getResource();
            MultiValueMap<String, Object> map = new LinkedMultiValueMap<>();
            map.add("file", fileResource);
            Map<String, String> describe = BeanUtils.describe(ssoConfigRequest);
            describe.forEach((key, value) -> {
                if (!StringUtils.equals(key, "file")) {
                    map.add(key, value);
                }
            });
            HttpEntity<MultiValueMap<String, Object>> httpEntity = new HttpEntity<>(map, headers);
            ResponseEntity<Object> responseEntity = restTemplate.postForEntity(
                NormalizerUtil.normalizeForString(gatewayApi + "/v1/sso/config"), httpEntity, Object.class);
            response.setStatus(responseEntity.getStatusCodeValue());
        } catch (InvocationTargetException | IllegalAccessException | NoSuchMethodException e) {
            log.error("save sso config :{} error: ", ssoConfigRequest, ExceptionUtil.getErrorMessage(e));
        }
    }

    /**
     * 保存SSO配置
     *
     * @param ssoConfigRequest 请求体
     */
    @ExterAttack
    @PutMapping("/v1/sso/config")
    public void updateSSOConfig(SSOConfigRequest ssoConfigRequest) {
        try {
            HttpHeaders headers = requestUtil.getForwardHeaderAndValidCsrf();
            headers.setContentType(MediaType.MULTIPART_FORM_DATA);
            MultiValueMap<String, Object> map = new LinkedMultiValueMap<>();
            if (Objects.nonNull(ssoConfigRequest.getFile())) {
                Resource fileResource = ssoConfigRequest.getFile().getResource();
                map.add("file", fileResource);
            }
            Map<String, String> describe = BeanUtils.describe(ssoConfigRequest);
            describe.forEach((key, value) -> {
                if (!StringUtils.equals(key, "file")) {
                    map.add(key, value);
                }
            });
            HttpEntity<MultiValueMap<String, Object>> httpEntity = new HttpEntity<>(map, headers);
            ResponseEntity<Object> responseEntity = restTemplate.exchange(
                NormalizerUtil.normalizeForString(gatewayApi + "/v1/sso/config"), HttpMethod.PUT, httpEntity,
                Object.class);
            response.setStatus(responseEntity.getStatusCodeValue());
        } catch (InvocationTargetException | IllegalAccessException | NoSuchMethodException e) {
            log.error("update sso config :{} error: ", ssoConfigRequest, ExceptionUtil.getErrorMessage(e));
        }
    }

    /**
     * 下载元数据
     */
    @GetMapping("/v1/sso/config/metadata")
    public void getMetadata() {
        String requestUrl = NormalizerUtil.normalizeForString(gatewayApi + "/v1/sso/config/metadata");
        downloadUtil.download(requestUrl);
    }
}
