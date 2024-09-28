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
package openbackup.system.base.common.rest;

import openbackup.system.base.config.FeignClientConfig;
import openbackup.system.base.service.SensitiveDataEliminateService;

import feign.Feign;
import feign.RequestInterceptor;
import feign.RequestTemplate;
import feign.codec.Decoder;
import feign.codec.ErrorDecoder;
import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.autoconfigure.http.HttpMessageConverters;
import org.springframework.cloud.openfeign.support.ResponseEntityDecoder;
import org.springframework.cloud.openfeign.support.SpringDecoder;
import org.springframework.context.annotation.Bean;

import java.util.Arrays;

/**
 * emeistor 自定义配置类
 *
 */
@Slf4j
public class EmeistorFeignConfiguration implements RequestInterceptor {
    @Autowired
    private SensitiveDataEliminateService sensitiveDataEliminateService;

    @Autowired
    private FeignClientConfig feignClientConfig;

    /**
     * 自定义request拦截
     *
     * @return Feign.Builder
     */
    @Bean("emeistorFeign")
    public Feign.Builder emeistorFeignBuilder() {
        return FeignBuilder.getDefaultRetryableBuilder().client(feignClientConfig.getInternalClient());
    }

    /**
     * ErrorDecoder
     *
     * @return ErrorDecoder
     */
    @Bean
    public ErrorDecoder errorDecoder() {
        return CommonDecoder::retryableErrorDecode;
    }

    /**
     * retryer
     *
     * @return retryer
     */
    @Bean
    public CommonRetryer<Long> retryer() {
        return CommonRetryer.create();
    }

    /**
     * decoder
     *
     * @return decoder
     */
    @Bean
    public Decoder decoder() {
        return new ResponseEntityDecoder(
                new SpringDecoder(() -> new HttpMessageConverters(new PhpMappingJackson2HttpMessageConverter())));
    }

    @Override
    public void apply(RequestTemplate requestTemplate) {
        requestTemplate.header("x-request-id", "2345678");
        // TOKEN
        requestTemplate.header("x-auth-token", "x-auth-token");
        // 认证类型，枚举值如下：0,eBackup Token 1,云场景IAM Token 2,BCManager Token
        requestTemplate.header("HTTP_X_USERTYPE", "2");
        log.debug(
                "send request. method: {}, url: {}.",
                requestTemplate.method(),
                sensitiveDataEliminateService.eliminateUrl(requestTemplate.url(), Arrays.asList()));
    }
}
