package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import lombok.EqualsAndHashCode;
import lombok.ToString;

import java.util.List;

/**
 * host对象
 *
 * @author t00482481
 * @since 2020-09-20
 */
@Data
@EqualsAndHashCode(callSuper = true)
@ToString(callSuper = true)
public class Environment extends ResourceEntity {
    /**
     * host offline
     */
    public static final String ENVIRONMENT_OFFLINE = "0";

    private String port;

    @JsonProperty("user_name")
    private String userName;

    private String password;

    @JsonProperty("link_status")
    private String linkStatus;

    @JsonProperty("os_type")
    private String osType;

    @JsonProperty("is_cluster")
    private boolean isCluster;

    @JsonProperty("cluster_info")
    private List<String> clusterInfo;

    private String location;

    private String endpoint;

    @JsonProperty("agent_version")
    private String agentVersion;
}
