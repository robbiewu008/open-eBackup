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

import openbackup.system.base.common.utils.ExceptionUtil;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.env.EnvironmentPostProcessor;
import org.springframework.core.env.ConfigurableEnvironment;
import org.springframework.core.env.MutablePropertySources;
import org.springframework.core.env.PropertiesPropertySource;
import org.springframework.core.io.FileUrlResource;
import org.springframework.core.io.support.ResourcePropertySource;
import org.springframework.stereotype.Component;

import java.io.IOException;
import java.net.URL;
import java.util.Enumeration;
import java.util.Properties;

/**
 * 功能描述
 *
 */
@Component
public class EnvironmentInitial implements EnvironmentPostProcessor {
    private static final Logger LOGGER = LoggerFactory.getLogger(EnvironmentInitial.class);

    @Override
    public void postProcessEnvironment(ConfigurableEnvironment environment, SpringApplication application) {
        try {
            Enumeration<URL> enumeration = this.getClass().getClassLoader().getResources("api.properties");
            MutablePropertySources propertySources = environment.getPropertySources();
            while (enumeration.hasMoreElements()) {
                URL url = enumeration.nextElement();
                FileUrlResource resource = new FileUrlResource(url);
                ResourcePropertySource source = new ResourcePropertySource(resource);
                propertySources.addLast(source);
            }
        } catch (IOException e) {
            LOGGER.error("postProcessEnvironment error:", ExceptionUtil.getErrorMessage(e));
        }
        String envKeys = System.getenv("env.support.keys");
        if (envKeys == null) {
            return;
        }
        String[] keys = envKeys.split(",");
        Properties properties = new Properties();
        for (String key : keys) {
            if (System.getenv(key) == null) {
                continue;
            }
            properties.put(key, System.getenv(key));
        }
        PropertiesPropertySource propertySource = new PropertiesPropertySource("env-properties", properties);
        environment.getPropertySources().addLast(propertySource);
    }
}
