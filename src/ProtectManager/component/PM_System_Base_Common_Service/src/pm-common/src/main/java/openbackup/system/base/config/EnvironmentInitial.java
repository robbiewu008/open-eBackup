/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
 * @author y00413474
 * @since 2020-06-29
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
