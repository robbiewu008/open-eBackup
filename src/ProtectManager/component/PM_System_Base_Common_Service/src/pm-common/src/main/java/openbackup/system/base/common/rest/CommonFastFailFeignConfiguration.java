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

import feign.Feign;
import feign.RequestInterceptor;
import feign.RequestTemplate;
import feign.codec.Decoder;
import feign.codec.ErrorDecoder;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.FeignClientConstant;
import openbackup.system.base.config.FeignClientConfig;
import openbackup.system.base.service.SensitiveDataEliminateService;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Bean;

/**
 * CommonFastFailFeignConfiguration
 *
 */
@Slf4j
public class CommonFastFailFeignConfiguration implements RequestInterceptor {
    @Autowired
    private SensitiveDataEliminateService sensitiveDataEliminateService;

    @Autowired
    private FeignClientConfig feignClientConfig;

    /**
     * 自定义request拦截
     *
     * @return Feign.Builder
     */
    @Bean("commonFastFailBuilder")
    public Feign.Builder feignBuilder() {
        return FeignBuilder.getFastFailFeignBuilder().client(feignClientConfig.getInternalClient());
    }

    /**
     * decoder
     *
     * @return decoder
     */
    @Bean
    public static Decoder decoder() {
        return CommonDecoder.decoder();
    }

    /**
     * errorDecoder
     *
     * @return errorDecoder
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
        CommonRetryPolicy policy = new CommonRetryPolicy();
        policy.setAttempts(FeignClientConstant.FAST_FAIL_MAX_ATTEMPTS);
        policy.setWaitTime(FeignClientConstant.FAST_FAIL_PERIOD);
        return CommonRetryer.create(policy);
    }

    @Override
    public void apply(RequestTemplate requestTemplate) {
        String url = sensitiveDataEliminateService.eliminateUrl(requestTemplate.url());
        log.debug("send request. method: {}, url: {}.", requestTemplate.method(), url);
    }
}
