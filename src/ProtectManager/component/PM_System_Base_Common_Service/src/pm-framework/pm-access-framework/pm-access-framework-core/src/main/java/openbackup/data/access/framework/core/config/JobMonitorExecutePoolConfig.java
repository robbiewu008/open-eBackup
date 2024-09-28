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
package openbackup.data.access.framework.core.config;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.scheduling.concurrent.ThreadPoolTaskExecutor;

import java.util.concurrent.Executor;

/**
 * job monitor 自定义线程池
 *
 */
@Configuration
public class JobMonitorExecutePoolConfig {
    private static final int CORE_POOL_SIZE = 40;

    private static final int MAX_POOL_SIZE = 40;

    private static final int QUEUE_CAPACITY = 128;

    private static final String THREAD_NAME_PREFIX = "job-monitor-";

    /**
     * 获取线程池
     *
     * @return executor
     */
    @Bean("jobMonitorExecutePool")
    public Executor getAsyncExecutor() {
        ThreadPoolTaskExecutor executor = new ThreadPoolTaskExecutor();
        // 设置最大线程池
        executor.setMaxPoolSize(CORE_POOL_SIZE);
        // 设置主线程池
        executor.setCorePoolSize(MAX_POOL_SIZE);
        // 设置队列数量
        executor.setQueueCapacity(QUEUE_CAPACITY);
        // 设置线程前缀
        executor.setThreadNamePrefix(THREAD_NAME_PREFIX);
        // 初始化
        executor.initialize();
        return executor;
    }
}
