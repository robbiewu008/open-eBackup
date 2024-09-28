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

import lombok.extern.slf4j.Slf4j;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.core.env.Environment;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpStatus;
import org.springframework.http.RequestEntity;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.client.RestTemplate;

import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * 通用的接口转发
 *
 * @see [相关类/方法]
 */
@RestController
@RequestMapping(ConfigConstant.CONSOLE)
@Slf4j
public class WebForwardController {
    private static final Logger LOGGER = LoggerFactory.getLogger(WebForwardController.class);

    @Autowired
    private Environment environment;

    @Autowired
    private RestTemplate restTemplate;

    @Autowired
    private HttpServletResponse response;

    @Autowired
    private HttpServletRequest request;

    @Autowired
    private RequestUtil requestUtil;

    @Value("${api.gateway.endpoint}")
    private String apiGatewayEndpoint;

    private List<String> urlBlackList = new ArrayList<String>() {
        private static final long serialVersionUID = 3816265311692619551L;
        {
            add("/v1/kms");
            add("/v1/secret/redis");
            add("/v1/auth/token");
        }
    };

    /**
     * 统一的接口转发
     *
     * @param version api的版本
     * @param api String
     * @param requestEntity RequestEntity
     * @return ResponseEntity
     */
    @ExterAttack
    @RequestMapping("/{version}/{api}/**")
    public ResponseEntity<Object> getApi(@PathVariable("version") String version, @PathVariable("api") String api,
        RequestEntity<Object> requestEntity) {
        String url = apiGatewayEndpoint + request.getRequestURI().replaceFirst(ConfigConstant.CONSOLE, "");
        String finalUrl = url;
        if (urlBlackList.stream().anyMatch(finalUrl::contains)) {
            log.error("url: {} is not found", url);
            return new ResponseEntity(HttpStatus.NOT_FOUND);
        }
        String query = requestEntity.getUrl().getQuery();
        if (query != null) {
            url = url + "?" + requestEntity.getUrl().getRawQuery();
        }
        URI uri;
        try {
            uri = new URL(NormalizerUtil.normalizeForString(url)).toURI();
        } catch (MalformedURLException | URISyntaxException e) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
        }
        HttpEntity<Object> httpEntity = new HttpEntity<>(requestEntity.getBody(),
            requestUtil.getForwardHeaderAndValidCsrf());
        ResponseEntity<Object> responseEntity = restTemplate.exchange(uri,
            Objects.requireNonNull(requestEntity.getMethod()), httpEntity, Object.class);
        return new ResponseEntity<>(responseEntity.getBody(), responseEntity.getStatusCode());
    }
}
