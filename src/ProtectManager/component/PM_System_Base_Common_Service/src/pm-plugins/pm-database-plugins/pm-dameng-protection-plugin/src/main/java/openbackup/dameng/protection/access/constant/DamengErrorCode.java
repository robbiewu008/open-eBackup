package openbackup.dameng.protection.access.constant;

/**
 * dameng相关错误码定义
 *
 * @author lWX1100347
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-15
 */
public class DamengErrorCode {
    /**
     * dameng注册的实例和集群实例数量不一致
     */
    public static final long INSTANCES_NUMBER_DIFF = -1L;

    /**
     * dameng注册的实例不属于同一集群
     */
    public static final long INSTANCES_CLUSTER_DIFF = -2L;

    /**
     * dameng集群实例的主备关系错误
     */
    public static final long INSTANCES_PRIMARY_SECONDARY_ERROR = -3L;
}
