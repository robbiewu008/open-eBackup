package openbackup.database.base.plugin.enums;

/**
 * 集群实例在线状态策略
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-12-14
 */
public enum ClusterInstanceOnlinePolicy {
    /**
     * 所有节点实例在线，集群实例在线
     */
    ALL_NODES_ONLINE("1"),

    /**
     * 任意节点实例在线，集群实例在线
     */
    ANY_NODE_ONLINE("2");

    private final String policy;

    /**
     * 构造方法
     *
     * @param policy 策略类型
     */
    ClusterInstanceOnlinePolicy(String policy) {
        this.policy = policy;
    }

    public String getPolicy() {
        return policy;
    }
}
