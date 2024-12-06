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

import com.zaxxer.hikari.HikariDataSource;

import feign.FeignException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.service.ConfigMapServiceImpl;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import javax.sql.DataSource;

/**
 * 数学工具类
 *
 */
@Configuration
public class DataSourceConfig {
    private static final Logger LOGGER = LoggerFactory.getLogger(DataSourceConfig.class);
    private static final int MINIMUM_IDLE_MIN_VALUE = 5;
    private static final int MINIMUM_IDLE_MAX_VALUE = 20;
    private static final int MAXIMUM_POOL_SIZE_MIN_VALUE = 5;
    private static final int MAXIMUM_POOL_SIZE_MAX_VALUE = 20;

    private String password;

    @Value("${spring.datasource.driverClassName}")
    private String driver;

    @Value("${spring.datasource.url}")
    private String url;

    @Value("${spring.datasource.userdb}")
    private String db;

    @Value("${spring.datasource.username}")
    private String username;

    @Value("${DATASOURCE_CONFIG_MINIMUM_IDLE:20}")
    private int minimumIdleValue;

    @Value("${DATASOURCE_CONFIG_MAXIMUM_POOL_SIZE:20}")
    private int maximumPoolSizeValue;

    @Autowired
    private ConfigMapServiceImpl configMapService;

    private void getConnectionInformation() {
        // 方便本地调试优先从环境变量中获取
        if (System.getenv("database.generalPassword") != null) {
            password = System.getenv("database.generalPassword");
            return;
        }
        try {
            LOGGER.info("get gauss information from infra.");
            password = configMapService.getValueFromSecretByKey("database.generalPassword", Boolean.FALSE);
            LOGGER.info("set password success.");
        } catch (FeignException.FeignClientException exception) {
            LOGGER.error("wrong with get password from inf.");
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
            if (VerifyUtil.isEmpty(password)) {
                LOGGER.error("NA. with password.");
                Thread.sleep(1000); // 死循环中降低CPU占用
            }
        } while (VerifyUtil.isEmpty(password));

        HikariDataSource dataSource = new HikariDataSource();
        dataSource.setDriverClassName(driver);
        final String datasourceUrl = System.getenv("spring.datasource.realUrl");
        String realUrl;
        if (datasourceUrl != null) {
            realUrl = datasourceUrl;
            LOGGER.info("value get from environment variable : {}", realUrl);
        } else {
            realUrl = this.url;
            LOGGER.info("value get from local variable : {}", realUrl);
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
        dataSource.addDataSourceProperty("sslfactory", "openbackup.system.base.config.GaussSSLSocketFactory");

        LOGGER.info("Environment realUrl: {}", dataSource.getJdbcUrl());
        return new RetryableDataSource(dataSource);
    }

    private void initPoolConfig(HikariDataSource dataSource) {
        int minimumIdle = revise(minimumIdleValue, MINIMUM_IDLE_MIN_VALUE, MINIMUM_IDLE_MAX_VALUE);
        dataSource.setMinimumIdle(minimumIdle);
        int maximumPoolSize = revise(maximumPoolSizeValue, Math.max(MAXIMUM_POOL_SIZE_MIN_VALUE, minimumIdle),
                MAXIMUM_POOL_SIZE_MAX_VALUE);
        dataSource.setMaximumPoolSize(maximumPoolSize);
        LOGGER.info("minimumIdle: {}, maximumPoolSize: {}", minimumIdle, maximumPoolSize);
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
