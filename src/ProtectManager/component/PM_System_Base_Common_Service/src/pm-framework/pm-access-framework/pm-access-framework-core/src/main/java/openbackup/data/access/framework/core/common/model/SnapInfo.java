package openbackup.data.access.framework.core.common.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * Global Search index
 *
 * @author l00347293
 * @since 2021-01-06
 **/
@Getter
@Setter
public class SnapInfo {
    @JsonProperty("snap_id")
    private String snapId;

    @JsonProperty("snap_type")
    private String snapType;

    @JsonProperty("timestamp")
    private String timestamp;

    @JsonProperty("resource_id")
    private String resourceId;

    @JsonProperty("resource_name")
    private String resourceName;

    @JsonProperty("resource_type")
    private String resourceType;

    @JsonProperty("chain_id")
    private String chainId;

    @JsonProperty("gn")
    private int gn;

    @JsonProperty("snap_metadata")
    private String snapMetadata;

    @JsonProperty("user_id")
    private String userId;

    @JsonProperty("esn")
    private String esn;
}
