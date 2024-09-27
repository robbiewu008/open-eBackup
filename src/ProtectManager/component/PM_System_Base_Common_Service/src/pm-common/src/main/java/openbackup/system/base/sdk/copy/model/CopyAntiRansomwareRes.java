package openbackup.system.base.sdk.copy.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 副本防勒索检测信息
 *
 * @author nwx1077006
 * @since 2022-01-12
 */
@Data
public class CopyAntiRansomwareRes {
    @JsonProperty("copy_id")
    private String copyId;

    private String timestamp;

    private String model;

    private Integer status;

    @JsonProperty("detection_duration")
    private Integer detectionDuration;

    @JsonProperty("detection_time")
    private String detectionTime;

    private String report;
}
