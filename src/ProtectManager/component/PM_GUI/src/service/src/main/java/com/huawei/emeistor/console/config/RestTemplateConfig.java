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
package com.huawei.emeistor.console.config;

import com.huawei.emeistor.console.util.KeyToolUtil;

import org.apache.http.conn.ssl.NoopHostnameVerifier;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.ssl.SSLContextBuilder;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Primary;
import org.springframework.http.client.ClientHttpRequestFactory;
import org.springframework.http.client.HttpComponentsClientHttpRequestFactory;
import org.springframework.http.client.SimpleClientHttpRequestFactory;
import org.springframework.http.converter.FormHttpMessageConverter;
import org.springframework.web.client.RestTemplate;

import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.UnrecoverableKeyException;
import java.util.concurrent.TimeUnit;

import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;

/**
 * 描述
 *
 * @see [相关类/方法]
 */
@Configuration
public class RestTemplateConfig {
    private static final int READ_TIMEOUT = 1000 * 60 * 15;

    private static final int CONNECT_TIMEOUT = 1000 * 60 * 15;

    private static final int TIMEOUT = 1000 * 60;

    private static final int MAX_CONN_TOTAL = 200;

    private static final int MAX_CONN_PER_ROUTE = 200;

    /**
     * KeyStore密钥文件
     */
    @Value("${server.ssl.key-store-password-file}")
    private String keyStorePwdFile;

    @Autowired
    private KeyToolUtil keyToolUtil;

    /**
     * 初始化 RestTemplate
     *
     * @return RestTemplate RestTemplate
     * @throws KeyStoreException new SSLContextBuilder().loadTrustMaterial方法抛出异常
     * @throws NoSuchAlgorithmException new SSLContextBuilder().loadTrustMaterial方法抛出异常
     * @throws KeyManagementException new SSLContextBuilder().loadTrustMaterial方法抛出异常
     */
    @Bean
    @Primary
    public RestTemplate restTemplate() throws KeyStoreException, NoSuchAlgorithmException, KeyManagementException {
        return generateRestTemplate(READ_TIMEOUT, CONNECT_TIMEOUT);
    }

    /**
     * 用户使用的 RestTemplate
     *
     * @return RestTemplate RestTemplate
     * @throws KeyStoreException new SSLContextBuilder().loadTrustMaterial方法抛出异常
     * @throws NoSuchAlgorithmException new SSLContextBuilder().loadTrustMaterial方法抛出异常
     * @throws KeyManagementException new SSLContextBuilder().loadTrustMaterial方法抛出异常
     */
    @Bean("userRestTemplate")
    public RestTemplate loginRestTemplate() throws KeyStoreException, NoSuchAlgorithmException, KeyManagementException {
        return generateRestTemplate(TIMEOUT, TIMEOUT);
    }

    /**
     * 直接调base使用的 RestTemplate
     *
     * @return RestTemplate RestTemplate
     * @throws KeyStoreException new SSLContextBuilder().loadTrustMaterial方法抛出异常
     * @throws NoSuchAlgorithmException new SSLContextBuilder().loadTrustMaterial方法抛出异常
     * @throws KeyManagementException new SSLContextBuilder().loadTrustMaterial方法抛出异常
     * @throws UnrecoverableKeyException new SSLContextBuilder().loadTrustMaterial方法抛出异常
     */
    @Bean("baseRestTemplate")
    public RestTemplate baseRestTemplate()
        throws KeyStoreException, NoSuchAlgorithmException, KeyManagementException, UnrecoverableKeyException {
        return generateBaseRestTemplate(TIMEOUT, TIMEOUT);
    }

    private RestTemplate generateBaseRestTemplate(int generalReadTimeout, int generalConnectTimeout)
        throws NoSuchAlgorithmException, KeyManagementException, KeyStoreException, UnrecoverableKeyException {
        SSLContext sslContext = getFeignTrustingSslContext();
        CloseableHttpClient httpClient = HttpClients.custom()
            .setMaxConnTotal(MAX_CONN_TOTAL)
            .setMaxConnPerRoute(MAX_CONN_PER_ROUTE)
            .setSSLContext(sslContext)
            .setSSLHostnameVerifier(new NoopHostnameVerifier())
            .evictExpiredConnections()
            .evictIdleConnections(15, TimeUnit.MINUTES)
            .build();
        HttpComponentsClientHttpRequestFactory factory = new HttpComponentsClientHttpRequestFactory();
        factory.setHttpClient(httpClient);
        factory.setBufferRequestBody(false);
        factory.setReadTimeout(generalReadTimeout);
        factory.setConnectTimeout(generalConnectTimeout);
        RestTemplate template = new RestTemplate(factory);
        template.getMessageConverters().add(new FormHttpMessageConverter());
        return template;
    }

    private SSLContext getFeignTrustingSslContext()
        throws NoSuchAlgorithmException, KeyManagementException, KeyStoreException, UnrecoverableKeyException {
        KeyStore keyStore = keyToolUtil.getInternalKeystore();
        KeyManagerFactory keyManagerFactory = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
        keyManagerFactory.init(keyStore, keyToolUtil.getKeyStorePassword(keyStorePwdFile).toCharArray());
        KeyManager[] keyManagers = keyManagerFactory.getKeyManagers();
        TrustManagerFactory trustManagerFactory = TrustManagerFactory.getInstance(
            TrustManagerFactory.getDefaultAlgorithm());
        trustManagerFactory.init(keyStore);
        TrustManager[] trustManagers = trustManagerFactory.getTrustManagers();
        SSLContext sslInstance = SSLContext.getInstance(KeyToolUtil.SSL_CONTEXT_VERSION);
        sslInstance.init(keyManagers, trustManagers, SecureRandom.getInstanceStrong());
        return sslInstance;
    }

    private RestTemplate generateRestTemplate(int generalReadTimeout, int generalConnectTimeout)
        throws NoSuchAlgorithmException, KeyManagementException, KeyStoreException {
        SSLContext sslContext = new SSLContextBuilder().loadTrustMaterial(null, (certificate, authType) -> true)
            .build();
        CloseableHttpClient httpClient = HttpClients.custom()
            .setMaxConnTotal(MAX_CONN_TOTAL)
            .setMaxConnPerRoute(MAX_CONN_PER_ROUTE)
            .setSSLContext(sslContext)
            .setSSLHostnameVerifier(new NoopHostnameVerifier())
            .evictExpiredConnections()
            .evictIdleConnections(15, TimeUnit.MINUTES)
            .build();
        HttpComponentsClientHttpRequestFactory factory = new HttpComponentsClientHttpRequestFactory();
        factory.setHttpClient(httpClient);
        factory.setBufferRequestBody(false);
        factory.setReadTimeout(generalReadTimeout);
        factory.setConnectTimeout(generalConnectTimeout);
        RestTemplate template = new RestTemplate(factory);
        template.getMessageConverters().add(new FormHttpMessageConverter());
        return template;
    }

    /**
     * 初始化连接池工厂类
     *
     * @return ClientHttpRequestFactory ClientHttpRequestFactory
     */
    @Bean("SimpleClientHttpRequestFactory")
    public ClientHttpRequestFactory simpleClientHttpRequestFactory() {
        SimpleClientHttpRequestFactory factory = new SimpleClientHttpRequestFactory();
        factory.setReadTimeout(READ_TIMEOUT);
        factory.setConnectTimeout(CONNECT_TIMEOUT);
        factory.setBufferRequestBody(false);
        return factory;
    }
}