/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.emeistor.console;

import lombok.extern.slf4j.Slf4j;

import org.mybatis.spring.annotation.MapperScan;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.cloud.openfeign.EnableFeignClients;
import org.springframework.retry.annotation.EnableRetry;
import org.springframework.scheduling.annotation.EnableAsync;
import org.springframework.transaction.annotation.EnableTransactionManagement;

/**
 * 启动类
 *
 * @author t00482481
 * @since 2020-9-06
 */
@EnableAsync
@EnableRetry
@SpringBootApplication
@EnableFeignClients(basePackages = {"com.huawei.emeistor.console"})
@EnableTransactionManagement
@MapperScan(basePackages = {"com.huawei.emeistor.console.**.dao", "com.huawei.emeistor.console.**.mapper"})
@Slf4j
public class ConsoleApplication {
    public static void main(String[] args) {
        SpringApplication.run(ConsoleApplication.class, args);
        log.info("The gui is started successfully.");
    }
}