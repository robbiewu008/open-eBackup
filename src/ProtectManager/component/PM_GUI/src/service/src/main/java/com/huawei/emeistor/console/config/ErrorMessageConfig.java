/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package com.huawei.emeistor.console.config;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.support.ReloadableResourceBundleMessageSource;

/**
 * 自动加载错误码配置
 *
 * @author y00559272
 * @since 2020-07-21
 */
@Configuration
public class ErrorMessageConfig {
    /**
     * 自动加载错误码配置
     *
     * @return 错误码配置
     */
    @Bean(name = "errorMessageSource")
    public ReloadableResourceBundleMessageSource messageSource() {
        ReloadableResourceBundleMessageSource messageBundle = new ReloadableResourceBundleMessageSource();
        messageBundle.addBasenames("classpath:message/error-common");
        messageBundle.addBasenames("classpath:message/error-user");
        messageBundle.setDefaultEncoding("UTF-8");
        return messageBundle;
    }
}
