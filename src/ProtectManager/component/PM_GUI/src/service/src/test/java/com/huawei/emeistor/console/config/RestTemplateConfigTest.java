/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.emeistor.console.config;

import static org.assertj.core.api.Assertions.assertThat;

import org.junit.Test;
import org.springframework.http.client.ClientHttpRequestFactory;
import org.springframework.http.client.HttpComponentsClientHttpRequestFactory;
import org.springframework.http.client.SimpleClientHttpRequestFactory;
import org.springframework.web.client.RestTemplate;

import java.security.KeyManagementException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;

/**
 * {@link RestTemplateConfig} 测试类
 *
 * @author w00616953
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-28
 */
public class RestTemplateConfigTest {
    /**
     * 用例名称：生成RestTemplateBean成功，且返回值正确
     * 前置条件：无
     * check点：1. 生成Bean成功；2. 返回值正确
     */
    @Test
    public void should_returnCorrectValue_when_createRestTemplate()
        throws KeyStoreException, NoSuchAlgorithmException, KeyManagementException {
        RestTemplateConfig config = new RestTemplateConfig();
        RestTemplate restTemplate = config.restTemplate();
        assertThat(restTemplate).isNotNull();
        assertThat(restTemplate.getRequestFactory()).isNotNull();
        assertThat(restTemplate.getMessageConverters()).isNotEmpty();
        ClientHttpRequestFactory requestFactory = restTemplate.getRequestFactory();
        assertThat(requestFactory).isExactlyInstanceOf(HttpComponentsClientHttpRequestFactory.class);
    }

    /**
     * 用例名称：生成ClientHttpRequestFactoryBean成功，且返回值正确
     * 前置条件：无
     * check点：1. 生成Bean成功；2. 返回值正确
     */
    @Test
    public void should_returnCorrectValue_when_createClientHttpRequestFactory() {
        RestTemplateConfig config = new RestTemplateConfig();
        ClientHttpRequestFactory factory = config.simpleClientHttpRequestFactory();
        assertThat(factory).isNotNull().isExactlyInstanceOf(SimpleClientHttpRequestFactory.class);
    }
}
