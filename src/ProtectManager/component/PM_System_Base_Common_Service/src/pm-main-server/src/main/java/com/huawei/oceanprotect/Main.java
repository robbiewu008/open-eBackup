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
