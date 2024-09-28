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

import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.filter.AuthFilter;
import com.huawei.emeistor.console.filter.CsrfWebFilter;
import com.huawei.emeistor.console.filter.RequestNormalizeFilter;
import com.huawei.emeistor.console.filter.SecurityResponseHeaderFilter;
import com.huawei.emeistor.console.filter.TimeoutFilter;

import org.springframework.boot.web.servlet.FilterRegistrationBean;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.PropertySource;

/**
 * 配置Filter
 *
 */
@Configuration
@PropertySource(value = "classpath:/properties/security.properties")
public class WebFilterConfig {
    /**
     * 注册URL标准化过滤器
     *
     * @return FilterRegistrationBean
     */
    @Bean
    public FilterRegistrationBean<RequestNormalizeFilter> requestNormalizeFilter() {
        FilterRegistrationBean<RequestNormalizeFilter> registration = new FilterRegistrationBean<>();
        registration.setFilter(requestNormalizeFilterBean());
        registration.setName("requestNormalizeFilterBean");
        registration.addUrlPatterns("/*");
        registration.setOrder(1);
        return registration;
    }

    /**
     * 登录白名单过滤器
     *
     * @return FilterRegistrationBean
     */
    @Bean
    public FilterRegistrationBean<AuthFilter> authFilter() {
        FilterRegistrationBean<AuthFilter> registration = new FilterRegistrationBean<>();
        registration.setFilter(authFilterBean());
        registration.setName("authFilterBean");
        registration.addUrlPatterns("/*");
        registration.setOrder(2);
        return registration;
    }

    /**
     * 注册AddResponseHeaderFilter过滤器
     *
     * @return FilterRegistrationBean
     */
    @Bean
    public FilterRegistrationBean<SecurityResponseHeaderFilter> securityResponseHeaderFilter() {
        FilterRegistrationBean<SecurityResponseHeaderFilter> registration = new FilterRegistrationBean<>();
        registration.setFilter(securityResponseHeaderFilterBean());
        registration.setName("securityResponseHeaderFilter");
        registration.addUrlPatterns("/*");
        registration.setOrder(3);
        return registration;
    }

    /**
     * 注册CsrfWebFilter过滤器
     *
     * @return FilterRegistrationBean
     */
    @Bean
    public FilterRegistrationBean<CsrfWebFilter> csrfFilter() {
        FilterRegistrationBean<CsrfWebFilter> registration = new FilterRegistrationBean<>();
        registration.setFilter(csrfFilterBean());
        registration.setName("myCsrfFilter");
        registration.addUrlPatterns(ConfigConstant.CONSOLE + "/*");
        registration.setOrder(4);
        return registration;
    }

    /**
     * 注册TimeoutFilter过滤器
     *
     * @return FilterRegistrationBean
     */
    @Bean
    public FilterRegistrationBean<TimeoutFilter> timeoutFilter() {
        FilterRegistrationBean<TimeoutFilter> registration = new FilterRegistrationBean<>();
        registration.setFilter(timeoutFilterBean());
        registration.setName("timeoutFilter");
        registration.addUrlPatterns(ConfigConstant.CONSOLE + "/*");
        registration.setOrder(5);
        return registration;
    }

    /**
     * SpringBoot管理，便于Filter中使用注解注入
     *
     * @return RequestNormalizeFilter
     */
    @Bean
    public RequestNormalizeFilter requestNormalizeFilterBean() {
        return new RequestNormalizeFilter();
    }

    /**
     * SpringBoot管理，便于Filter中使用注解注入
     *
     * @return AuthFilter
     */
    @Bean
    public AuthFilter authFilterBean() {
        return new AuthFilter();
    }

    /**
     * SpringBoot管理，便于Filter中使用注解注入
     *
     * @return AddResponseHeaderFilter
     */
    @Bean
    public SecurityResponseHeaderFilter securityResponseHeaderFilterBean() {
        return new SecurityResponseHeaderFilter();
    }

    /**
     * SpringBoot管理，便于Filter中使用注解注入
     *
     * @return TimeoutFilter
     */
    @Bean
    public TimeoutFilter timeoutFilterBean() {
        return new TimeoutFilter();
    }

    /**
     * SpringBoot管理，便于Filter中使用注解注入
     *
     * @return CsrfWebFilter
     */
    @Bean
    public CsrfWebFilter csrfFilterBean() {
        return new CsrfWebFilter();
    }
}
