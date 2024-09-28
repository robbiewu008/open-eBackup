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
package openbackup.system.base.config;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.ErrorResponse;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.NumberUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.http.HttpMethod;
import org.springframework.http.HttpStatus;
import org.springframework.http.client.ClientHttpResponse;
import org.springframework.web.client.DefaultResponseErrorHandler;
import org.springframework.web.client.RestTemplate;

import java.io.IOException;
import java.net.URI;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.Optional;

/**
 * Rest Template Configuration
 *
 */
@Configuration
@Slf4j
public class RestTemplateConfiguration {
    /**
     * RestTemplate的超时时间
     */
    private static final int TIMEOUT = 1000 * 60;

    private static final int READ_TIMEOUT = 1000 * 60;

    /**
     * hcs rest template
     *
     * @return rest template
     */
    @Bean("hcsUserRestTemplate")
    public RestTemplate loginRestTemplate() {
        StorageHttpRequestFactory factory = new StorageHttpRequestFactory();
        factory.setConnectTimeout(TIMEOUT);
        factory.setReadTimeout(READ_TIMEOUT);
        RestTemplate restTemplate = new RestTemplate(factory);
        restTemplate.setErrorHandler(new CommonRestExceptionHandler());
        return restTemplate;
    }

    private static class InternalRestTemplateResponseErrorHandler extends DefaultResponseErrorHandler {
        @Override
        public void handleError(ClientHttpResponse response) {
            byte[] body = getResponseBody(response);
            Charset charset = Optional.ofNullable(getCharset(response)).orElse(StandardCharsets.UTF_8);
            String message = new String(body, charset);
            log.error("cause: {}", message);
            ErrorResponse errorResponse = JSONObject.fromObject(message).toBean(ErrorResponse.class);
            long errorCode = NumberUtil.convertToLong(errorResponse.getErrorCode(), CommonErrorCode.SYSTEM_ERROR);
            throw new LegoCheckedException(errorCode, errorResponse.getDetailParams(), errorResponse.getErrorMessage());
        }

        /**
         * handle Error
         *
         * @param url url
         * @param method method
         * @param response response
         * @throws IOException IOException
         */
        @Override
        public void handleError(URI url, HttpMethod method, ClientHttpResponse response) throws IOException {
            log.error("url: {}, method: {}", url, method);
            super.handleError(response);
        }
    }

    private class CommonRestExceptionHandler extends DefaultResponseErrorHandler {
        @Override
        public boolean hasError(ClientHttpResponse response) throws IOException {
            return super.hasError(response);
        }

        @Override
        public final void handleError(final ClientHttpResponse response) throws IOException {
            if (response.getStatusCode() != HttpStatus.OK) {
                if (response.getStatusCode() == HttpStatus.UNAUTHORIZED) {
                    throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED, "rest auth failed!");
                }
                if (response.getStatusCode() == HttpStatus.NOT_FOUND) {
                    throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "rest target obj not exit!");
                }
                throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "rest failed!");
            }
            super.handleError(response);
        }
    }
}
