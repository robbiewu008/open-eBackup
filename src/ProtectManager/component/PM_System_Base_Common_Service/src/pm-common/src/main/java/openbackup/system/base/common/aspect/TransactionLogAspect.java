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
package openbackup.system.base.common.aspect;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.AspectOrderConstant;
import openbackup.system.base.sdk.system.SystemConfigService;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.springframework.core.annotation.Order;
import org.springframework.stereotype.Component;
import org.springframework.transaction.annotation.Transactional;

import java.util.concurrent.TimeUnit;

/**
 * 事务日志切面
 *
 */
@Slf4j
@Aspect
@Component
@Order(AspectOrderConstant.LOGGING_ASPECT_ORDER)
public class TransactionLogAspect {
    private static final long DEFAULT_LOGGING_QUOTA_MILLIS = 1000L;
    private static long loggingQuota;

    public TransactionLogAspect(SystemConfigService systemConfigService) {
        String loggingQuotaStr = systemConfigService.getConfigValue("transaction_log_quota");
        loggingQuota = loggingQuotaStr == null ? DEFAULT_LOGGING_QUOTA_MILLIS : Long.parseLong(loggingQuotaStr);
    }

    /**
     * 事务日志切面
     *
     * @param joinPoint     切点
     * @param transactional 注解类
     * @return 返回值
     * @throws Throwable throwable
     */
    @Around(value = "@annotation(transactional)", argNames = "joinPoint,transactional")
    public Object processTransactionalLogging(ProceedingJoinPoint joinPoint, Transactional transactional)
            throws Throwable {
        long startTime = System.currentTimeMillis();
        long startTimeMono = System.nanoTime();
        Object proceed = joinPoint.proceed();
        long endTimeMono = System.nanoTime();
        long endTime = System.currentTimeMillis();
        long durationMono = TimeUnit.NANOSECONDS.toMillis(endTimeMono - startTimeMono);
        if (durationMono > loggingQuota) {
            log.info("[SLOW TRANSACTION] Transaction {} started at {}, ended at {}, duration {} ms.",
                    joinPoint, startTime, endTime, durationMono);
        }
        return proceed;
    }
}
