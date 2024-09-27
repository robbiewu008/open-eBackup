/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.access.framework.resource.util;

import lombok.extern.slf4j.Slf4j;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * 资源扫描的线程池工具类
 *
 * @author t30028453
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-19
 */
@Slf4j
public class ResourceThreadPoolTool {
    // 核心线程池数量
    private static final int CORE_POOL_SIZE = 30;

    // 最大线程数量
    private static final int MAXIMUM_POOL_SIZE = 100;

    // 活跃时间
    private static final long KEEP_ALIVE_TIME = 120L;

    // 时间单位
    private static final TimeUnit TIME_UNIT = TimeUnit.SECONDS;

    // 工作队列大小
    private static final int WORK_QUEUE_SIZE = 300;

    // 初始化阻塞队列
    private static final BlockingQueue<Runnable> BLOCKING_QUEUE = new LinkedBlockingQueue<>(WORK_QUEUE_SIZE);

    // 线程的记号数
    private static final AtomicInteger THREAD_NUMBER = new AtomicInteger(1);

    // 线程名称
    private static final String NAME_PREFIX = "resource";

    // 线程池
    private static final ThreadPoolExecutor POOL = new ThreadPoolExecutor(CORE_POOL_SIZE,
            MAXIMUM_POOL_SIZE,
            KEEP_ALIVE_TIME,
            TIME_UNIT,
            BLOCKING_QUEUE,
            new PrefixThreadFactory());

    /**
     * PrefixThreadFactory线程工厂
     */
    private static class PrefixThreadFactory implements ThreadFactory {
        /**
         * 开启线程，使线程名称有实际含义
         *
         * @param runnable runnable
         * @return Thread
         */
        @Override
        public Thread newThread(Runnable runnable) {
            String threadName = NAME_PREFIX + "-thread-" + THREAD_NUMBER.getAndIncrement();
            return new Thread(runnable, threadName);
        }
    }

    /**
     * 获取线程池
     *
     * @return 线程池
     */
    public static ThreadPoolExecutor getPool() {
        return POOL;
    }

    /**
     * 判断线程池的队列是否满了
     *
     * @return 线程池的队列是否满了
     */
    public static boolean isQueueFull() {
        return POOL.getQueue().size() == WORK_QUEUE_SIZE;
    }

    /**
     * 判断线程池是否繁忙
     *
     * @return 线程池是否繁忙
     */
    public static boolean isBusy() {
        return POOL.getPoolSize() > CORE_POOL_SIZE && isQueueFull();
    }

    /**
     * 获取 线程池中的队列长度
     *
     * @return 线程池中的队列长度
     */
    public static int getQueueSzie() {
        return POOL.getQueue().size();
    }

    /**
     * 获取 线程池中的活跃线程数
     *
     * @return 线程池中的活跃线程数
     */
    public static int getThreadNum() {
        return POOL.getActiveCount();
    }
}
