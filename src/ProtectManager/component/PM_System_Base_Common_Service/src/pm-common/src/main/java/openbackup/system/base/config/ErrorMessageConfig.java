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
package openbackup.system.base.config;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.support.ReloadableResourceBundleMessageSource;

/**
 * ErrorMessageConfig
 *
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
