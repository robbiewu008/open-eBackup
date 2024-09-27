package openbackup.oceanbase.common.constants;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-21
 */
public interface OBErrorCodeConstants {
    /**
     * 认证信息错误
     */
    long AUTH_ERROR = 1577209942L;

    /**
     * 集群状态异常
     */
    long CLUSTER_STATUS_INVALID_ERROR = 1577213524L;

    /**
     * 集群节点数不一致
     */
    long CLUSTER_NODE_COUNT_NOT_SAME_ERROR = 1577209972L;

    /**
     * OBServer不属于同一个集群
     */
    long OBSERVER_IS_NOT_ONE_CLUSTER_ERROR = 1577213525L;

    /**
     * OBServer连接异常
     */
    long OBSERVER_CONNECT_ERROR = 1577213522L;

    /**
     * 租户不存在（注册租户集）
     */
    long TENANT_NOT_EXIST = 1577213527L;

    /**
     * OBServer的业务IP和agent的IP不匹配
     */
    long OBSERVER_IP_NOT_MATCH = 1677947141L;
}
