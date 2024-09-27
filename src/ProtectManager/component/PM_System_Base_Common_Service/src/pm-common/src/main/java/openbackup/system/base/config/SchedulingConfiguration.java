/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
 */

package openbackup.system.base.config;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import java.util.concurrent.ScheduledThreadPoolExecutor;

/**
 * 功能描述: Schedule线程池
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-20
 */
@Configuration
public class SchedulingConfiguration {
    private static final int SCHEDULE_THREAD_COUNT = 10;

    /**
     * 注入Schedule线程池
     *
     * @return ScheduledThreadPoolExecutor
     */
    @Bean
    public ScheduledThreadPoolExecutor scheduledExecutorService() {
        return new ScheduledThreadPoolExecutor(SCHEDULE_THREAD_COUNT);
    }
}