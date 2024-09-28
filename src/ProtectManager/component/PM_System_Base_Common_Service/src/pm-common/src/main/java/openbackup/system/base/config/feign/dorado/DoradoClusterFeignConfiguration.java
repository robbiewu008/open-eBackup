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
package openbackup.system.base.config.feign.dorado;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.storage.StorageResponse;
import openbackup.system.base.common.model.storage.StorageSession;
import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.sdk.storage.StorageService;
import openbackup.system.base.sdk.storage.model.DoradoResponse;

import com.fasterxml.jackson.databind.DeserializationFeature;
import com.fasterxml.jackson.databind.ObjectMapper;

import feign.Client;
import feign.Contract;
import feign.Feign;
import feign.FeignException;
import feign.RequestInterceptor;
import feign.RequestTemplate;
import feign.Response;
import feign.codec.Decoder;
import lombok.extern.slf4j.Slf4j;

import org.apache.hc.client5.http.ssl.NoopHostnameVerifier;
import org.apache.hc.core5.ssl.SSLContextBuilder;
import org.springframework.beans.factory.ObjectFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.autoconfigure.http.HttpMessageConverters;
import org.springframework.cloud.openfeign.support.ResponseEntityDecoder;
import org.springframework.cloud.openfeign.support.SpringDecoder;
import org.springframework.context.annotation.Bean;
import org.springframework.http.MediaType;
import org.springframework.http.converter.json.MappingJackson2HttpMessageConverter;

import java.io.IOException;
import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.net.ssl.SSLContext;

/**
 * 功能描述
 *
 */
@Slf4j
public class DoradoClusterFeignConfiguration implements RequestInterceptor {
    private static final String REQUEST_REMOTE_STORAGE_SUCCESS = "0";

    private static final Map<String, Long> PERFORMANCE_ERROR_CODE = new HashMap<String, Long>();

    static {
        PERFORMANCE_ERROR_CODE.put("1073952264", 1677930004L);
        PERFORMANCE_ERROR_CODE.put("1073951745", 1677930004L);
        PERFORMANCE_ERROR_CODE.put("503", 1677930005L);
    }

    @Autowired
    private StorageService storageService;

    /**
     * 自定义request拦截
     *
     * @return Feign.Builder
     */
    @Bean("DoradoClusterFeignBuilder")
    public Feign.Builder feignBuilder() {
        Client client;
        try {
            SSLContext context = new SSLContextBuilder().loadTrustMaterial(null, (chain, authType) -> true).build();
            client = new Client.Default(context.getSocketFactory(), new NoopHostnameVerifier());
        } catch (Exception e) {
            log.error("Create feign client with SSL config failed", e);
            client = new Client.Default(null, null);
        }
        return FeignBuilder.getDefaultRetryableBuilder().client(client);
    }

    /**
     * 将契约改为feign原生的默认契约。这样就可以使用feign自带的注解了。
     * 用feign.Contract.Default替换SpringMvcContract契约
     *
     * @return 默认的feign契约
     */
    @Bean
    public Contract feignContract() {
        return new Contract.Default();
    }

    /**
     * decoder
     *
     * @return decoder
     */
    @Bean("DoradoClusterDecoder")
    public Decoder decoder() {
        return new ResponseEntityDecoder(new MyDecoder(feignHttpMessageConverter()));
    }

    private ObjectFactory<HttpMessageConverters> feignHttpMessageConverter() {
        final HttpMessageConverters httpMessageConverters =
            new HttpMessageConverters(new PhpMappingJackson2HttpMessageConverter());
        return () -> httpMessageConverters;
    }

    /**
     * interceptor
     *
     * @param template RequestTemplate
     */
    @Override
    public void apply(RequestTemplate template) {
        log.debug("request url: {}", template.feignTarget().url());
        StorageResponse<StorageSession> storageSession = storageService.getStorageSession();
        template.header("iBaseToken", storageSession.getData().getBaseToken());
        template.header("Cookie", storageSession.getData().getCookie());
    }

    /**
     * MyDecoder
     *
     */
    private static class MyDecoder extends SpringDecoder {
        /**
         * MyDecoder
         *
         * @param messageConverters messageConverters
         */
        MyDecoder(ObjectFactory<HttpMessageConverters> messageConverters) {
            super(messageConverters);
        }

        @Override
        public Object decode(Response response, Type type) throws IOException, FeignException {
            Object object = super.decode(response, type);
            if (object instanceof StorageResponse) {
                StorageResponse storageResponse = (StorageResponse) object;
                if (storageResponse.getError() != null) {
                    if (!REQUEST_REMOTE_STORAGE_SUCCESS.equals(storageResponse.getError().getCode())) {
                        long errorCode = Long.parseLong(storageResponse.getError().getCode());
                        String errorMsg = storageResponse.getError().getDescription();
                        log.error("get StorageSystem rest is fail, error code is {}, error description is {}",
                            errorCode, errorMsg);
                        throw new LegoCheckedException(errorCode, errorMsg);
                    }
                }
            }
            if (object instanceof DoradoResponse) {
                DoradoResponse doradoResponse = (DoradoResponse) object;
                if (doradoResponse.getResult() != null) {
                    if (!REQUEST_REMOTE_STORAGE_SUCCESS.equals(doradoResponse.getResult().getCode())) {
                        log.error("get StorageSystem rest is fail, error code is {}, error description is {}",
                            doradoResponse.getResult().getCode(), doradoResponse.getResult().getDescription());
                        throw new LegoCheckedException(convertDoradoErrorCode(doradoResponse.getResult().getCode()));
                    }
                }
            }
            return object;
        }

        private static Long convertDoradoErrorCode(String errorCode) {
            if (PERFORMANCE_ERROR_CODE.containsKey(errorCode)) {
                return PERFORMANCE_ERROR_CODE.get(errorCode);
            }
            return Long.parseLong(errorCode);
        }
    }

    /**
     * PhpMappingJackson2HttpMessageConverter
     *
     */
    private static class PhpMappingJackson2HttpMessageConverter extends MappingJackson2HttpMessageConverter {
        PhpMappingJackson2HttpMessageConverter() {
            List<MediaType> mediaTypes = new ArrayList<>();
            mediaTypes.add(MediaType.valueOf(MediaType.APPLICATION_OCTET_STREAM_VALUE));
            mediaTypes.add(MediaType.valueOf(MediaType.TEXT_HTML_VALUE + ";charset=UTF-8"));
            mediaTypes.add(MediaType.valueOf(MediaType.TEXT_PLAIN_VALUE + ";charset=UTF-8"));
            setSupportedMediaTypes(mediaTypes);
            ObjectMapper objectMapper = getObjectMapper();
            objectMapper.configure(DeserializationFeature.ACCEPT_SINGLE_VALUE_AS_ARRAY, true);
            setObjectMapper(objectMapper);
        }
    }
}
