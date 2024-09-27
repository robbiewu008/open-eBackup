package openbackup.system.base.sdk.cluster.model;

import lombok.Data;

import java.util.List;

/**
 * cluster params
 *
 * @author p30001902
 * @since 2020-12-17
 */
@Data
public class SourceClustersParams {
    // cluster added as target cluster times
    private int addedCount;

    // cluster status
    private int clusterStatus;

    // dorado controller node count
    private int nodeCount;

    // cluster node ip list
    private List<String> mgrIpList;

    // storage controller ip obj
    private String storageDisplayIps;

    private String deployType;

    // 当前节点的roleType
    private String roleType;
}
