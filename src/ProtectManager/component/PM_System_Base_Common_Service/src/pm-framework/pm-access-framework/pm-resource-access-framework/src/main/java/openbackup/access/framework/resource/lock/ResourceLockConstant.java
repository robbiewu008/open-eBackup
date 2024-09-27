package openbackup.access.framework.resource.lock;

/**
 * Redis冗余锁常量类
 *
 * @author y30044273
 * @since 2023-10-08
 */
public class ResourceLockConstant {
    /**
     * Redis中保存lock_id的set集合的key
     */
    public static final String LOCK_ID_REDIS_SET_KEY = "LOCK_ID_REDIS_SET_KEY";

    /**
     * Redis中保存lock_id的前缀
     */
    public static final String REDIS_LOCK_ID_PREFIX = "redis_resource_lock_";

    /**
     * 远端复制资源ID前缀
     */
    public static final String LOCK_REPLICATION_TASK_FLAG = "FS_DELETE_";

    private ResourceLockConstant() {
    }
}
