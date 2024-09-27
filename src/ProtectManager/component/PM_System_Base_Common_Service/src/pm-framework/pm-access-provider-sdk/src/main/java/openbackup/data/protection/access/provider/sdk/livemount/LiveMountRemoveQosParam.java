package openbackup.data.protection.access.provider.sdk.livemount;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Live Mount Remove Qos Param
 *
 * @author l00272247
 * @since 2020-09-21
 */
@Data
public class LiveMountRemoveQosParam {
    @JsonProperty("BACKUPID")
    private String backupId;

    @JsonProperty("APPTYPE")
    private String appType;
}
