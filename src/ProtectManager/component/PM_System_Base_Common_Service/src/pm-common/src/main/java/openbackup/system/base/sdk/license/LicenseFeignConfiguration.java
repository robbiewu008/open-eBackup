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
package openbackup.system.base.sdk.license;

import openbackup.system.base.common.model.storage.StorageResponse;
import openbackup.system.base.common.model.storage.StorageSession;
import openbackup.system.base.common.rest.FeignBuilder;
import openbackup.system.base.common.utils.VerifyUtil;

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
import org.springframework.boot.autoconfigure.http.HttpMessageConverters;
import org.springframework.cloud.openfeign.support.ResponseEntityDecoder;
import org.springframework.cloud.openfeign.support.SpringDecoder;
import org.springframework.context.annotation.Bean;
import org.springframework.http.MediaType;
import org.springframework.http.converter.json.MappingJackson2HttpMessageConverter;

import java.io.IOException;
import java.lang.reflect.Type;
import java.security.KeyManagementException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import javax.net.ssl.SSLContext;

/**
 * 功能描述
 *
 */
@Slf4j
public class LicenseFeignConfiguration implements RequestInterceptor {
    /**
     * 自定义request拦截
     *
     * @return Feign.Builder
     */
    @Bean("DoradoFeignBuilder")
    public Feign.Builder feignBuilder() {
        Client client;
        try {
            SSLContext context = new SSLContextBuilder().loadTrustMaterial(null, (chain, authType) -> true).build();
            client = new Client.Default(context.getSocketFactory(), new NoopHostnameVerifier());
        } catch (NoSuchAlgorithmException | KeyStoreException | KeyManagementException e) {
            log.error("Create feign client with SSL config failed", e);
            client = new Client.Default(null, null);
        }
        return FeignBuilder.getDefaultRetryableBuilder().client(client);
    }

    /**
     * 将契约改为feign原生的默认契约。这样就可以使用feign自带的注解了。
     * 用feign.Contract.Default替换SpringMvcContract契约
     *
     * @return 默认的feign契约 contract
     */
    @Bean
    public Contract feignContract() {
        // Spring Cloud Netflix默认的SpringMvcController将替换为feign.Contract.Default。
        return new feign.Contract.Default();
    }

    /**
     * feignDecoder
     *
     * @return feignDecoder decoder
     */
    @Bean
    public Decoder feignDecoder() {
        return new ResponseEntityDecoder(new MyDecoder(feignHttpMessageConverter()));
    }

    private ObjectFactory<HttpMessageConverters> feignHttpMessageConverter() {
        final HttpMessageConverters httpMessageConverters =
            new HttpMessageConverters(new PhpMappingJackson2HttpMessageConverter());
        return () -> httpMessageConverters;
    }

    @Override
    public void apply(RequestTemplate template) {
        Collection<String> headers = template.headers().get("Content-Type");
        if (VerifyUtil.isEmpty(headers)) {
            return;
        }
        String header = headers.toArray(new String[0])[0];
        if (!header.contains("multipart/form-data; charset=UTF-8; boundary=")) {
            return;
        }
        int startIndex = header.indexOf("charset");
        int endIndex = header.indexOf("boundary");
        String tempHeader = header.substring(0, startIndex) + header.substring(endIndex);
        template.header("Content-Type", new ArrayList<>());
        template.header("Content-Type", tempHeader);
    }

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
                StorageResponse<?> storageResponse = (StorageResponse<?>) object;
                if (storageResponse.getData() instanceof StorageSession) {
                    StorageSession storageSession = (StorageSession) storageResponse.getData();
                    Collection<String> list = response.headers().get("Set-Cookie");
                    if (list != null && !list.isEmpty()) {
                        storageSession.setCookie(list.iterator().next());
                    }
                }
            }
            return object;
        }
    }

    private static class PhpMappingJackson2HttpMessageConverter extends MappingJackson2HttpMessageConverter {
        PhpMappingJackson2HttpMessageConverter() {
            List<MediaType> mediaTypes = new ArrayList<>();
            mediaTypes.add(MediaType.valueOf(MediaType.TEXT_HTML_VALUE + ";charset=UTF-8"));
            mediaTypes.add(MediaType.valueOf(MediaType.TEXT_PLAIN_VALUE + ";charset=UTF-8"));
            setSupportedMediaTypes(mediaTypes);
        }
    }
}
