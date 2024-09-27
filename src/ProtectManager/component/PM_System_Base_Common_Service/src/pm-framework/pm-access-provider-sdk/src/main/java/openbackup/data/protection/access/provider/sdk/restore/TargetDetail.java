package openbackup.data.protection.access.provider.sdk.restore;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * Target Detail
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2020-06-17
 */
@Data
public class TargetDetail {
    @JsonProperty("src_id")
    private List<String> srcId;

    @JsonProperty("target_id")
    private String targetId;

    @JsonProperty("target_type")
    private String targetType;
}
