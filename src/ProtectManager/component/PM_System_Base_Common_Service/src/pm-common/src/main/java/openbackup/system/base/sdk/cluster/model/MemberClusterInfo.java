package openbackup.system.base.sdk.cluster.model;

import openbackup.system.base.sdk.cluster.enums.ClusterEnum;

import lombok.Data;

/**
 * member cluster vo
 *
 * @author w00607005
 * @since 2023-06-19
 */
@Data
public class MemberClusterInfo {
    /* Cluster id */
    private Integer clusterId;

    /* Cluster name */
    private String clusterName;

    /* Cluster status;27: Online； 28：Offline */
    private Integer status;

    /* Trunking service IP */
    private String clusterIp;

    /* Cluster port */
    private Integer clusterPort;

    /* remote esn */
    private String remoteEsn;

    /* Cluster User Name */
    private String username;

    /* password */
    private String password;

    /* Create time */
    private long createTime;

    /* Update time */
    private long lastUpdateTime;

    private int role;

    private String netPlaneName;

    /**
     * 是否健康
     *
     * @return 是否健康
     */
    public boolean isHealth() {
        return status != null && status.equals(ClusterEnum.StatusEnum.ONLINE.getStatus());
    }
}
