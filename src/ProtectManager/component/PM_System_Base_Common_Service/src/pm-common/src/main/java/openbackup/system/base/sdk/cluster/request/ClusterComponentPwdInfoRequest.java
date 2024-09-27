package openbackup.system.base.sdk.cluster.request;

import openbackup.system.base.sdk.cluster.model.ClusterComponentPwdInfo;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * 功能描述
 *
 * @author c30047317
 * @since 2023-07-27
 */
@Data
public class ClusterComponentPwdInfoRequest {
    /**
     * 内部组件密码信息
     */
    private List<ClusterComponentPwdInfo> clusterComponentPwdInfoList;

    /**
     * 是否正在组建多集群
     */
    @JsonProperty("isAssemble")
    private boolean isAssemble;

    /**
     * 节点角色
     */
    @JsonProperty("roleType")
    private int roleType = 0;
}
