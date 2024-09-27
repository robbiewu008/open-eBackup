package openbackup.system.base.common.constants;

import lombok.AccessLevel;
import lombok.RequiredArgsConstructor;

/**
 * CacheConstant
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-06-19
 */
@RequiredArgsConstructor(access = AccessLevel.PRIVATE)
public class CacheConstant {
    /**
     * 启动后，1分钟执行刷新
     */
    public static final long INITIAL_DELAY = 60 * 1000L;

    /**
     * 一分钟刷新一次
     */
    public static final long PERIOD = 60 * 1000L;

    /**
     * redis map key
     */
    public static final String CACHE_REDIS_KEY = "redisCache";

    /**
     * redis 更新zookeeper锁key
     */
    public static final String CACHE_REDIS_UPDATE = "/redisCacheUpdate";

    /**
     * protect engine缓存key值
     */
    public static final String PROTECT_ENGINE_POD_INFO_CACHE = "protectEnginePodInfoCache";
}
