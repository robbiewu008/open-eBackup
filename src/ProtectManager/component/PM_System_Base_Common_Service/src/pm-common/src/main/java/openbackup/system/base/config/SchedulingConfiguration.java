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