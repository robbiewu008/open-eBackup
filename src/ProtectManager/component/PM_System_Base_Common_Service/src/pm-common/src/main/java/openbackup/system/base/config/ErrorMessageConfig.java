/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 */

package openbackup.system.base.config;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.support.ReloadableResourceBundleMessageSource;

/**
 * ErrorMessageConfig
 *
 * @author w00448845
 * @version [CDM Integrated machine]
 * @since 2019-11-09
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
        messageBundle.addBasenames("classpath:message/error-cert");
        messageBundle.addBasenames("classpath:message/error-cluster");
        messageBundle.addBasenames("classpath:message/error-repository");
        messageBundle.addBasenames("classpath:message/error-job");
        messageBundle.addBasenames("classpath:message/error-livemount");
        messageBundle.setDefaultEncoding("UTF-8");
        return messageBundle;
    }
}
