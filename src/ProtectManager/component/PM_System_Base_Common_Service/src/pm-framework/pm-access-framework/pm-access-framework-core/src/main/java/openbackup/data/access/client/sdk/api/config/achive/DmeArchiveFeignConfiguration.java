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
package openbackup.data.access.client.sdk.api.config.achive;

import openbackup.system.base.common.constants.FeignClientConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.common.rest.PhpMappingJackson2HttpMessageConverter;
import openbackup.system.base.config.FeignClientConfig;

import feign.ExceptionPropagationPolicy;
import feign.Feign;
import feign.RetryableException;
import feign.Retryer;
import feign.codec.Decoder;
import feign.codec.ErrorDecoder;
import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.ObjectFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.autoconfigure.http.HttpMessageConverters;
import org.springframework.cloud.openfeign.support.ResponseEntityDecoder;
import org.springframework.cloud.openfeign.support.SpringDecoder;
import org.springframework.context.annotation.Bean;
import org.springframework.http.HttpStatus;

import java.io.IOException;
import java.util.Objects;
import java.util.Optional;

/**
 * DEM feign configuration
 *
 */
@Slf4j
public class DmeArchiveFeignConfiguration {
    @Autowired
    private FeignClientConfig feignClientConfig;

    /**
     * 自定义request拦截
     *
     * @return Feign.Builder
     */
    @Bean("ebackupFeignBuilder")
    public Feign.Builder feignBuilder() {
        return FeignBuilder.getDefaultRetryableBuilder().client(feignClientConfig.getInternalClient());
    }

    /**
     * 添加重试机制
     *
     * @return Retryer
     */
    @Bean
    public Retryer feignRetryer() {
        return new Retryer.Default(FeignClientConstant.DME_PERIOD, FeignClientConstant.DME_MAX_PERIOD,
                FeignClientConstant.MAX_ATTEMPTS);
    }

    /**
     * ExceptionPropagationPolicy
     *
     * @return ExceptionPropagationPolicy
     */
    @Bean
    public ExceptionPropagationPolicy policy() {
        return ExceptionPropagationPolicy.UNWRAP;
    }

    private ErrorDecoder errorDecoder(Decoder decoder) {
        return (methodKey, resp) -> {
            try {
                boolean shouldDecodeResponseBody =
                        resp.status() >= HttpStatus.OK.value() && resp.status() < HttpStatus.MULTIPLE_CHOICES.value();
                if (!shouldDecodeResponseBody) {
                    return new RetryableException(resp.status(), "http error code", resp.request().httpMethod(), null,
                            resp.request());
                }
                Object object = decoder.decode(resp, DmeResponse.class);
                if (!(object instanceof DmeResponse)) {
                    return new LegoCheckedException("http error");
                }
                DmeResponse<?> response = (DmeResponse<?>) object;
                Optional<LegoCheckedException> exception = response.getExceptionIfError();
                return exception.orElseGet(() -> new LegoCheckedException("http error"));
            } catch (IOException e) {
                return e;
            }
        };
    }

    /**
     * errorDecoder
     *
     * @return ErrorDecoder
     */
    @Bean
    public ErrorDecoder errorDecoder() {
        return errorDecoder(decode());
    }

    /**
     * decoder
     *
     * @return Decoder
     */
    @Bean
    public Decoder decoder() {
        return decode();
    }

    private Decoder decode() {
        ResponseEntityDecoder responseEntityDecoder =
                new ResponseEntityDecoder(new SpringDecoder(feignHttpMessageConverter()));
        return (response, type) -> {
            Object object = responseEntityDecoder.decode(response, type);
            if (object instanceof DmeResponse) {
                DmeResponse<?> dmeResponse = (DmeResponse<?>) object;
                Optional<LegoCheckedException> cause = dmeResponse.getExceptionIfError();
                DmeResponseError responseError = dmeResponse.getError();
                if (cause.isPresent() && Objects.nonNull(responseError)) {
                    log.error("Exception from DME, code: {}, desc: {}, param: {}.",
                        responseError.getCode(), responseError.getDescription(), responseError.getDetailParams());
                }
            }
            return object;
        };
    }

    private ObjectFactory<HttpMessageConverters> feignHttpMessageConverter() {
        final HttpMessageConverters httpMessageConverters =
                new HttpMessageConverters(new PhpMappingJackson2HttpMessageConverter());
        return () -> httpMessageConverters;
    }
}
