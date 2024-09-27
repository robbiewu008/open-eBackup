package openbackup.tidb.resources.access.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * tidb Condition
 *
 * @author w00426202
 * @since 2023-07-15
 */
@Setter
@Getter
public class TidbCondition {
    /**
     * 操作类型
     */
    @JsonProperty("action_type")
    private String actionType;

    /**
     * 集群名称
     */
    private String clusterName;

    /**
     * 是否集群
     */
    private boolean isCluster;

    /**
     * 主机列表
     */
    private List<String> agentIds;
}
