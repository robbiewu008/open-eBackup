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
package com.huawei.emeistor.console.util;

import com.huawei.emeistor.console.bean.CiphertextVo;
import com.huawei.emeistor.console.bean.PlaintextVo;
import com.huawei.emeistor.console.contant.CommonErrorCode;
import com.huawei.emeistor.console.exception.LegoCheckedException;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.http.HttpEntity;
import org.springframework.stereotype.Component;
import org.springframework.web.client.HttpClientErrorException;
import org.springframework.web.client.HttpServerErrorException;
import org.springframework.web.client.RestClientException;
import org.springframework.web.client.RestTemplate;

/**
 * kmc加解密接口
 *
 */
@Component
@Slf4j
public class EncryptorRestClient {
    private static final int MAX_RETRY_COUNT = 3;

    /**
     * 调用System_base包装的查询secret接口
     */
    private static final String V1_SECRET = "/v1/secret/redis";

    private static final String V1_KMC = "/v1/kms";

    private static final String ENCRYPT = "/encrypt";

    private static final String DECRYPT = "/decrypt";

    @Value("${api.gateway.endpoint}")
    private String url;

    @Autowired
    private RestTemplate restTemplate;

    /**
     * 从System_base获取secret中Redis登陆密码
     *
     * @return redis登陆密码
     */
    public String getRedisAuthFromSecret() {
        String redisAuth = "";
        try {
            redisAuth = restTemplate.getForObject(NormalizerUtil.normalizeForString(url + V1_SECRET), String.class);
        } catch (RestClientException error) {
            log.error("get redis auth info from base rest error: ", error);
        }
        return redisAuth;
    }

    /**
     * 加密
     *
     * @param ciphertextVo ciphertextVo
     * @return 加密字符串
     */
    public CiphertextVo encrypt(String ciphertextVo) {
        if (StringUtils.isBlank(ciphertextVo)) {
            return new CiphertextVo();
        }
        PlaintextVo plaintextVo = new PlaintextVo();
        plaintextVo.setPlaintext(ciphertextVo);
        HttpEntity<PlaintextVo> httpEntity = new HttpEntity<>(plaintextVo);
        String requestUrl = NormalizerUtil.normalizeForString(url + V1_KMC + ENCRYPT);
        int retryCount = 0;
        while (retryCount < MAX_RETRY_COUNT) {
            try {
                return restTemplate.postForEntity(requestUrl, httpEntity, CiphertextVo.class, plaintextVo).getBody();
            } catch (HttpServerErrorException | HttpClientErrorException e) {
                log.error("Encrypt failed, retry count: {}.", retryCount, ExceptionUtil.getErrorMessage(e));
                retryCount++;
            }
        }
        log.error("Encrypt failed, retry finish.");
        throw new LegoCheckedException(CommonErrorCode.REQUEST_TIMEOUT, "Encrypt failed.");
    }

    /**
     * 解密
     *
     * @param ciphertext ciphertext
     * @return 解密字符串
     */
    public PlaintextVo decrypt(String ciphertext) {
        if (StringUtils.isBlank(ciphertext)) {
            return new PlaintextVo();
        }
        CiphertextVo ciphertextVo = new CiphertextVo();
        ciphertextVo.setCiphertext(ciphertext);
        HttpEntity<CiphertextVo> httpEntity = new HttpEntity<>(ciphertextVo);
        String requestUrl = NormalizerUtil.normalizeForString(url + V1_KMC + DECRYPT);
        int retryCount = 0;
        while (retryCount < MAX_RETRY_COUNT) {
            try {
                return restTemplate.postForEntity(requestUrl, httpEntity, PlaintextVo.class, ciphertextVo).getBody();
            } catch (HttpServerErrorException | HttpClientErrorException e) {
                log.error("Decrypt failed, retry count: {}.", retryCount, ExceptionUtil.getErrorMessage(e));
                retryCount++;
            }
        }
        log.error("Decrypt failed, retry finish.");
        throw new LegoCheckedException(CommonErrorCode.REQUEST_TIMEOUT, "Decrypt failed.");
    }
}
