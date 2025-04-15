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
package com.huawei.emeistor.console.config.datasource;

import com.huawei.emeistor.console.exception.LegoCheckedException;

import com.zaxxer.hikari.HikariDataSource;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import javax.sql.DataSource;

/**
 * 数据库配置类
 *
 */
@Configuration
@Slf4j
public class DataSourceConfig {
    private static final int MINIMUM_IDLE_VALUE = 20;

    private static final int MAXIMUM_POOL_SIZE_VALUE = 10;

    private static final int MINIMUM_IDLE_MIN_VALUE = 5;

    private static final int MINIMUM_IDLE_MAX_VALUE = 50;

    private static final int MAXIMUM_POOL_SIZE_MIN_VALUE = 5;

    private static final int MAXIMUM_POOL_SIZE_MAX_VALUE = 10;

    private static final long ILLEGAL_PARAM = 1677929218L;

    private String password;

    @Value("${spring.datasource.driver-class-name}")
    private String driver;

    @Value("${spring.datasource.url}")
    private String url;

    @Value("${spring.datasource.userdb}")
    private String db;

    @Value("${spring.datasource.username}")
    private String username;

    @Autowired
    private DataSourceConfigService dataSourceConfigService;

    private void getConnectionInformation() {
        // 方便本地调试优先从环境变量中获取
        if (System.getenv("database.generalPassword") != null) {
            password = System.getenv("database.generalPassword");
            return;
        }
        try {
            JSONArray dataSourceConfig = dataSourceConfigService.getDataSourceConfigSecreteMap().getData();
            log.info("get gauss information from infra.");
            for (final Object obj : dataSourceConfig) {
                if (!(obj instanceof JSONObject)) {
                    throw new LegoCheckedException(ILLEGAL_PARAM, "obj is not instance of JSONObject");
                }
                JSONObject object = (JSONObject) obj;
                if (!object.containsKey("database.generalPassword")) {
                    continue;
                }
                final Object pwd = object.get("database.generalPassword");
                if (!(pwd instanceof String)) {
                    throw new LegoCheckedException(ILLEGAL_PARAM, "pwd is not instance of String");
                }
                password = (String) pwd;
                log.info("set password success.");
                return;
            }
        } catch (FeignException.FeignClientException exception) {
            log.error("wrong with get password from inf.");
        }
    }

    /**
     * dataSource
     *
     * @return DataSource 数据源
     * @throws InterruptedException 中断异常
     */
    @Bean
    public DataSource dataSource() throws InterruptedException {
        do {
            getConnectionInformation();
            if (StringUtils.isEmpty(password)) {
                log.error("NA. with password.");
                Thread.sleep(1000); // 死循环中降低CPU占用
            }
        } while (StringUtils.isEmpty(password));

        HikariDataSource dataSource = new HikariDataSource();
        dataSource.setDriverClassName(driver);
        final String datasourceUrl = System.getenv("spring.datasource.realUrl");
        String realUrl;
        if (datasourceUrl != null) {
            realUrl = datasourceUrl;
            log.info("value get from environment variable : {}", realUrl);
        } else {
            realUrl = this.url;
            log.info("value get from local variable : {}", realUrl);
        }
        dataSource.setJdbcUrl(realUrl.substring(0, realUrl.length() - "postgres".length()) + db);
        final String datasourceUsername = System.getenv("spring.datasource.username");
        if (datasourceUsername != null) {
            dataSource.setUsername(datasourceUsername);
        } else {
            dataSource.setUsername(username);
        }
        dataSource.setPassword(password);
        initPoolConfig(dataSource);
        dataSource.addDataSourceProperty("ssl", true);
        dataSource.addDataSourceProperty("sslmode", "verify-ca");
        dataSource.addDataSourceProperty("sslfactory",
            "com.huawei.emeistor.console.config.datasource.GaussSSLSocketFactory");

        log.info("Environment realUrl: {}", dataSource.getJdbcUrl());
        return new RetryableDataSource(dataSource);
    }

    private void initPoolConfig(HikariDataSource dataSource) {
        int minimumIdle = revise(MINIMUM_IDLE_VALUE, MINIMUM_IDLE_MIN_VALUE, MINIMUM_IDLE_MAX_VALUE);
        dataSource.setMinimumIdle(minimumIdle);
        int maximumPoolSize = revise(MAXIMUM_POOL_SIZE_VALUE, Math.max(MAXIMUM_POOL_SIZE_MIN_VALUE, minimumIdle),
            MAXIMUM_POOL_SIZE_MAX_VALUE);
        dataSource.setMaximumPoolSize(maximumPoolSize);
        log.info("minimumIdle: {}, maximumPoolSize: {}", minimumIdle, maximumPoolSize);
    }

    /**
     * revise method
     *
     * @param value value
     * @param min min
     * @param max max
     * @return result
     */
    public static int revise(int value, int min, int max) {
        int temp = Math.max(value, min);
        return Math.min(temp, max);
    }
}