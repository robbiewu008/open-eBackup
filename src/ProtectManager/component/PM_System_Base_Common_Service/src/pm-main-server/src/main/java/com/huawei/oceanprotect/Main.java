/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.oceanprotect;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.security.callee.CalleeMethodScan;

import org.mybatis.spring.annotation.MapperScan;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.cloud.openfeign.EnableFeignClients;
import org.springframework.retry.annotation.EnableRetry;
import org.springframework.scheduling.annotation.EnableAsync;
import org.springframework.scheduling.annotation.EnableScheduling;

/**
 * Main
 *
 * @author t00482481
 * @since 2020-07-02
 */
@Slf4j
@EnableRetry
@EnableAsync
@EnableScheduling
@SpringBootApplication(scanBasePackages = {"openbackup", "com.huawei.oceanprotect"})
@MapperScan(basePackages = {"openbackup.**.dao", "openbackup.**.mapper",
    "com.huawei.oceanprotect.**.dao", "com.huawei.oceanprotect.**.mapper"})
@EnableFeignClients(basePackages = {"com.huawei.oceanprotect", "openbackup"})
@CalleeMethodScan({"com.huawei.oceanprotect", "openbackup"})
public class Main {
    /**
     * entrance method
     *
     * @param args args
     */
    public static void main(final String[] args) {
        try {
            SpringApplication.run(Main.class, args);
            log.info("The system base is started successfully.");
        } catch (Exception e) {
            log.error("Fail to Start to system base", e);
            throw e;
        }
    }
}
