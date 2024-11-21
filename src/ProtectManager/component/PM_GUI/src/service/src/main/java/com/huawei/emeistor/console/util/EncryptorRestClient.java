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
import com.huawei.emeistor.console.service.EncryptorRestApi;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.beans.factory.annotation.Value;
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

    @Value("${pm-system-base.url}")
    private String url;

    @Autowired
    @Qualifier("baseRestTemplate")
    private RestTemplate kmsRestTemplate;

    @Autowired
    private EncryptorRestApi encryptorRestApi;

    /**
     * 从System_base获取secret中Redis登陆密码
     *
     * @return redis登陆密码
     */
    public String getRedisAuthFromSecret() {
        String redisAuth = "";
        try {
            redisAuth = kmsRestTemplate.getForObject(NormalizerUtil.normalizeForString(url + V1_SECRET), String.class);
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
        int retryCount = 0;
        while (retryCount < MAX_RETRY_COUNT) {
            try {
                return encryptorRestApi.encrypt(plaintextVo);
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
        int retryCount = 0;
        while (retryCount < MAX_RETRY_COUNT) {
            try {
                return encryptorRestApi.decrypt(ciphertextVo);
            } catch (HttpServerErrorException | HttpClientErrorException e) {
                log.error("Decrypt failed, retry count: {}.", retryCount, ExceptionUtil.getErrorMessage(e));
                retryCount++;
            }
        }
        log.error("Decrypt failed, retry finish.");
        throw new LegoCheckedException(CommonErrorCode.REQUEST_TIMEOUT, "Decrypt failed.");
    }
}
