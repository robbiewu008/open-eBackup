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
package openbackup.system.base.common.utils;

import org.springframework.retry.backoff.FixedBackOffPolicy;
import org.springframework.retry.policy.SimpleRetryPolicy;
import org.springframework.retry.support.RetryTemplate;

import java.util.Map;

/**
 * 通用重试模板
 * UnifiedCopyDeleteCompleteHandler
 *
 * @author twx1009756
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-03-17
 */
public class RetryTemplateUtil {
    /**
     * 通用方法重试模板
     *
     * @param retryTimes 重试的次数
     * @param retryInternalTime 重试间隔时间
     * @param retryableExceptions 重试的异常列表
     * @return 重试模板
     */
    public static RetryTemplate fixedBackOffRetryTemplate(int retryTimes, long retryInternalTime,
        Map<Class<? extends Throwable>, Boolean> retryableExceptions) {
        // 设置重试策略，重试次数为retryTimes次, 重试的异常列表为retryableExceptions
        SimpleRetryPolicy simpleRetryPolicy = new SimpleRetryPolicy(retryTimes, retryableExceptions);
        RetryTemplate retryTemplate = new RetryTemplate();
        retryTemplate.setRetryPolicy(simpleRetryPolicy);

        // 设置为重试时间间隔为retryInternalTime秒
        FixedBackOffPolicy fixedBackOffPolicy = new FixedBackOffPolicy();
        fixedBackOffPolicy.setBackOffPeriod(retryInternalTime);
        retryTemplate.setBackOffPolicy(fixedBackOffPolicy);
        return retryTemplate;
    }
}
