package openbackup.data.protection.access.provider.sdk.livemount;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Live Mount Modify Param
 *
 * @author l00272247
 * @since 2020-09-21
 */
@Data
public class LiveMountModifyParam {
    @JsonProperty("BACKUPID")
    private String backupId;

    @JsonProperty("PERFORMANCE")
    private LiveMountPerformance performance;

    @JsonProperty("APPTYPE")
    private String appType;
}
