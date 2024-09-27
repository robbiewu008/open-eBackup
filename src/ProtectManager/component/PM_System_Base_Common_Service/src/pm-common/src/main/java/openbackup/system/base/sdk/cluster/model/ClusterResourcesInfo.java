package openbackup.system.base.sdk.cluster.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * 目标集群资源信息查询数据返回模型
 *
 * @author dWX1009286
 * @since 2021-07-20
 */
@Data
public class ClusterResourcesInfo {
    // 目标集群资源数量
    @JsonProperty("summary")
    private List<ClusterResourceCount> clusterResourceCounts;
}
