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
package openbackup.system.base.common.thread;

import lombok.extern.slf4j.Slf4j;

import java.util.Objects;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * PushUpdateThreadPoolTool
 *
 */
@Slf4j
public class PushUpdateThreadPoolTool {
    private static volatile ThreadPoolExecutor threadPoolExecutor;

    // 核心线程池数量
    private static final int CORE_POOL_SIZE = 10;

    // 最大线程数量
    private static final int MAXIMUM_POOL_SIZE = 100;

    // 活跃时间
    private static final long KEEP_ALIVE_TIME = 120L;

    // 时间单位
    private static final TimeUnit TIME_UNIT = TimeUnit.SECONDS;

    // 工作队列大小
    private static final int WORK_QUEUE_SIZE = 20000;

    // 初始化阻塞队列
    private static final BlockingQueue<Runnable> BLOCKING_QUEUE = new LinkedBlockingQueue<>(WORK_QUEUE_SIZE);

    // 线程的记号数
    private static final AtomicInteger THREAD_NUMBER = new AtomicInteger(1);

    // 线程名称
    private static final String NAME_PREFIX = "pushUpdateCertTask";

    private PushUpdateThreadPoolTool() {
    }

    /**
     * 获取线程池
     *
     * @return 线程池
     */
    public static ThreadPoolExecutor getPool() {
        if (threadPoolExecutor == null) {
            synchronized (PushUpdateThreadPoolTool.class) {
                if (threadPoolExecutor == null) {
                    threadPoolExecutor = new ThreadPoolExecutor(CORE_POOL_SIZE, MAXIMUM_POOL_SIZE, KEEP_ALIVE_TIME,
                        TIME_UNIT, BLOCKING_QUEUE, new PushUpdateThreadPoolTool.PrefixThreadFactory());
                }
            }
        }
        return threadPoolExecutor;
    }

    /**
     * 关闭线程池
     */
    public static void shutdownNow() {
        if (!Objects.isNull(threadPoolExecutor)) {
            threadPoolExecutor.shutdownNow();
            threadPoolExecutor = null;
        }
    }

    /**
     * PrefixThreadFactory线程工厂
     */
    private static class PrefixThreadFactory implements ThreadFactory {
        /**
         * 开启线程
         *
         * @param runnable runnable
         * @return Thread
         */
        @Override
        public Thread newThread(Runnable runnable) {
            String threadName = NAME_PREFIX + "-thread-" + THREAD_NUMBER.getAndIncrement();
            Thread newThread = new Thread(runnable, threadName);
            newThread.setUncaughtExceptionHandler(
                (Thread thread, Throwable throwable) -> log.error("newThread uncaughtException"));
            return newThread;
        }
    }
}
