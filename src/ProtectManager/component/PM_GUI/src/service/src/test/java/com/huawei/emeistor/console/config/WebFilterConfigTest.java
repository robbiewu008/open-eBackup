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

import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.filter.AuthFilter;
import com.huawei.emeistor.console.filter.CsrfWebFilter;
import com.huawei.emeistor.console.filter.RequestNormalizeFilter;
import com.huawei.emeistor.console.filter.SecurityResponseHeaderFilter;
import com.huawei.emeistor.console.filter.TimeoutFilter;
import com.huawei.emeistor.console.service.SessionService;
import com.huawei.emeistor.console.util.EncryptorRestClient;
import com.huawei.emeistor.console.util.RequestUtil;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.boot.web.servlet.FilterRegistrationBean;
import org.springframework.context.annotation.Import;
import org.springframework.test.context.junit4.SpringRunner;

import javax.annotation.Resource;

/**
 * {@link WebFilterConfig} 测试类
 *
 * @author w00616953
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-28
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {WebFilterConfig.class, RequestUtil.class})
public class WebFilterConfigTest {
    @MockBean
    private SessionService sessionService;

    @MockBean
    private EncryptorRestClient client;

    @MockBean
    private TimeoutFilter timeoutFilter;

    @Autowired
    private RequestUtil requestUtil;

    @Autowired
    private WebFilterConfig webFilterConfig;

    /**
     * 用例名称：生成FilterRegistrationBean成功，且返回值正确
     * 前置条件：无
     * check点：1. 生成Bean成功；2. 返回值正确
     */
    @Test
    public void should_returnCorrectValue_when_createRequestNormalizeFilterBean() {
        FilterRegistrationBean<RequestNormalizeFilter> bean
            = webFilterConfig.requestNormalizeFilter();
        assertThat(bean.getFilter()).isNotNull().isExactlyInstanceOf(RequestNormalizeFilter.class);
        assertThat(bean.getUrlPatterns()).contains("/*");
        assertThat(bean.getOrder()).isOne();
    }

    /**
     * 用例名称：生成FilterRegistrationBean成功，且返回值正确
     * 前置条件：无
     * check点：1. 生成Bean成功；2. 返回值正确
     */
    @Test
    public void should_returnCorrectValue_when_createAuthFilterBean() {
        FilterRegistrationBean<AuthFilter> bean
            = webFilterConfig.authFilter();
        assertThat(bean.getFilter()).isNotNull().isExactlyInstanceOf(AuthFilter.class);
        assertThat(bean.getUrlPatterns()).contains("/*");
        assertThat(bean.getOrder()).isEqualTo(2);
    }

    /**
     * 用例名称：生成FilterRegistrationBean成功，且返回值正确
     * 前置条件：无
     * check点：1. 生成Bean成功；2. 返回值正确
     */
    @Test
    public void should_returnCorrectValue_when_createSecurityResponseHeaderFilter() {
        FilterRegistrationBean<SecurityResponseHeaderFilter> bean
            = webFilterConfig.securityResponseHeaderFilter();
        assertThat(bean.getFilter()).isNotNull().isExactlyInstanceOf(SecurityResponseHeaderFilter.class);
        assertThat(bean.getUrlPatterns()).contains("/*");
        assertThat(bean.getOrder()).isEqualTo(3);
    }

    /**
     * 用例名称：生成FilterRegistrationBean成功，且返回值正确
     * 前置条件：无
     * check点：1. 生成Bean成功；2. 返回值正确
     */
    @Test
    public void should_returnCorrectValue_when_createCsrfFilter() {
        FilterRegistrationBean<CsrfWebFilter> bean
            = webFilterConfig.csrfFilter();
        assertThat(bean.getFilter()).isNotNull().isExactlyInstanceOf(CsrfWebFilter.class);
        assertThat(bean.getUrlPatterns()).contains(ConfigConstant.CONSOLE + "/*");
        assertThat(bean.getOrder()).isEqualTo(4);
    }

    /**
     * 用例名称：生成FilterRegistrationBean成功，且返回值正确
     * 前置条件：无
     * check点：1. 生成Bean成功；2. 返回值正确
     */
    @Test
    public void should_returnCorrectValue_when_createTimeoutFilter() {
        FilterRegistrationBean<TimeoutFilter> bean
            = webFilterConfig.timeoutFilter();
        assertThat(bean.getUrlPatterns()).contains(ConfigConstant.CONSOLE + "/*");
        assertThat(bean.getOrder()).isEqualTo(5);
    }
}
