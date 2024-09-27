package openbackup.system.base.sdk.host.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * host对象
 *
 * @author t00482481
 * @since 2020-09-20
 */
@Data
public class Host {
    /**
     * host offline
     */
    public static final String HOST_OFFLINE = "0";

    @JsonProperty("host_id")
    private String hostId;

    private String name;

    @JsonProperty("env_type")
    private String envType;

    @JsonProperty("endpoint")
    private String ip;

    private String port;

    @JsonProperty("link_status")
    private String linkStatus;

    @JsonProperty("os_type")
    private String osType;

    @JsonProperty("proxy_type")
    private String proxyType;

    @JsonProperty("is_cluster")
    private boolean isCluster;

    @JsonProperty("cluster_info")
    private List<String> clusterInfo;
}
