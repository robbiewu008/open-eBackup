package openbackup.data.access.framework.core.config;

import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.scheduling.concurrent.ThreadPoolTaskExecutor;

import java.util.concurrent.Executor;

/**
 * job monitor 自定义线程池
 *
 * @author y30000858
 * @since 2021-10-16
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