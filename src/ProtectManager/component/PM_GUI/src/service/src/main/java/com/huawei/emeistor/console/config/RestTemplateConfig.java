/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package com.huawei.emeistor.console.config;

import org.apache.http.conn.ssl.NoopHostnameVerifier;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.ssl.SSLContextBuilder;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Primary;
import org.springframework.http.client.ClientHttpRequestFactory;
import org.springframework.http.client.HttpComponentsClientHttpRequestFactory;
import org.springframework.http.client.SimpleClientHttpRequestFactory;
import org.springframework.http.converter.FormHttpMessageConverter;
import org.springframework.web.client.RestTemplate;

import java.security.KeyManagementException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.util.concurrent.TimeUnit;

import javax.net.ssl.SSLContext;

/**
 * 描述
 *
 * @author lwx544155
 * @version [OceanStor DJ V100R003C00, 2020年03月04日]
 * @see [相关类/方法]
 * @since [产品/模块版本]
 */
@Configuration
public class RestTemplateConfig {
    private static final int READ_TIMEOUT = 1000 * 60 * 15;

    private static final int CONNECT_TIMEOUT = 1000 * 60 * 15;

    private static final int TIMEOUT = 1000 * 60;

    private static final int MAX_CONN_TOTAL = 200;

    private static final int MAX_CONN_PER_ROUTE = 200;

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