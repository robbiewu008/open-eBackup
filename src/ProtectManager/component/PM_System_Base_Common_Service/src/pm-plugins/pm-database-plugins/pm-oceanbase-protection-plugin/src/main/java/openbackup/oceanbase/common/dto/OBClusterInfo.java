package openbackup.oceanbase.common.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * 集群谢谢
 *
 * @author c00826511
 * @since 2023-07-04
 */
@Data
public class OBClusterInfo {
    /**
     * 集群OBServer节点谢谢
     */
    private List<OBAgentInfo> obServerAgents;

    /**
     * 集群OBClient节点谢谢
     */
    private List<OBAgentInfo> obClientAgents;

    /**
     * 集群中的租户列表
     */
    private List<OBTenantInfo> tenantInfos;

    /**
     * 集群的版本
     */
    private String version;

    /**
     * 集群的自己的ID（不是PM上管理注册时分配的集群uuid）
     */
    @JsonProperty("cluster_id")
    private String clusterId;

    /**
     * 集群的自己的名称（不是PM上管理注册时写的集群名称）
     */
    @JsonProperty("cluster_name")
    private String clusterName;

    /**
     * 集群的状态，正常时值为"VALID"
     */
    @JsonProperty("cluster_status")
    private String clusterStatus;

    /**
     * 资源池
     */
    private List<ResourcePool> pools;
}
