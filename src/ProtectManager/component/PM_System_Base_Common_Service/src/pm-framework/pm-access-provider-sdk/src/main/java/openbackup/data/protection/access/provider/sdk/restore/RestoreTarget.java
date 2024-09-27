package openbackup.data.protection.access.provider.sdk.restore;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * Restore Target
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2020-06-17
 */
@Data
public class RestoreTarget {
    @JsonProperty("env_id")
    private String envId;

    @JsonProperty("env_type")
    private String envType;

    @JsonProperty("restore_target")
    private String restoreTarget;

    private List<TargetDetail> details;
}
