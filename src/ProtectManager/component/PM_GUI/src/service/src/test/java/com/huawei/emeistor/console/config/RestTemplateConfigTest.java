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
