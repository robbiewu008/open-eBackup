package openbackup.access.framework.resource.service.lock;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.pack.lock.Lock;
import openbackup.system.base.pack.lock.LockService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;
import java.util.function.Function;

/**
 * 资源分布式锁Service
 *
 * @author c30016231
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-05-12
 */
@Slf4j
@Component
public class ResourceDistributedLockService {
    private final LockService lockService;

    public ResourceDistributedLockService(LockService lockService) {
        this.lockService = lockService;
    }

    /**
     * 给资源的操作加锁后运行
     *
     * @param key lock key
     * @param resource resource
     * @param function function
     * @return function result
     */
    public <T extends ProtectedResource> Object tryLockAndGet(String key, T resource, Function<T, Object> function) {
        Lock lock = lockService.createDistributeLock(key);
        if (!lock.tryLock(ResourceConstants.RESOURCE_LOCK_WAIT_TIME_OUT, TimeUnit.SECONDS)) {
            throw new LegoCheckedException(CommonErrorCode.SAME_RESOURCE_OPERATION_IS_RUNNING,
                "resource of the same type is updating.");
        }
        try {
            log.info("lock success, key :{}, resource:{}", key, resource.getUuid());
            return function.apply(resource);
        } finally {
            lock.unlock();
        }
    }

    /**
     * 给资源的操作加锁后运行
     *
     * @param key lock key
     * @param resource resource
     * @param consumer consumer
     */
    public <T extends ProtectedResource> void tryLockAndRun(String key, T resource, Consumer<T> consumer) {
        tryLockAndGet(key, resource, (res) -> {
            consumer.accept(res);
            return null;
        });
    }

    /**
     * 获取资源分布式锁的key
     *
     * @param keyPrefix keyPrefix
     * @param resource resource
     * @return lock key
     */
    public String getResourceLockKey(String keyPrefix, ProtectedResource resource) {
        return keyPrefix + "_" + resource.getType() + "_" + resource.getSubType();
    }
}
