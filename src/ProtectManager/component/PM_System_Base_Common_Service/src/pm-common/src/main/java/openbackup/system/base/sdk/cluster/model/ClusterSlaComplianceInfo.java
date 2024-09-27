package openbackup.system.base.sdk.cluster.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 目标集群Sla遵从数量信息查询数据返回模型
 *
 * @author dWX1009286
 * @since 2021-07-27
 */
@Data
public class ClusterSlaComplianceInfo {
    // 遵循数量
    @JsonProperty("in_compliance")
    private int inCompliance;

    // 不遵循数量
    @JsonProperty("out_of_compliance")
    private int outOfCompliance;
}
