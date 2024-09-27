package openbackup.data.protection.access.provider.sdk.resource;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * Agent connection result
 *
 * @author w30044259
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-27
 */
@Data
@NoArgsConstructor
public class EnvironmentConnectionResult {
    @JsonProperty("end_point")
    private String endPoint;

    @JsonProperty("link_status")
    private Integer linkStatus;

    @JsonProperty("cluster_name")
    private String clusterName;
}