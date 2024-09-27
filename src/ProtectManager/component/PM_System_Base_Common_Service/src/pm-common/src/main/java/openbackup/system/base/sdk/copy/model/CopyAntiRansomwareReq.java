package openbackup.system.base.sdk.copy.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.Date;

/**
 * 防勒索检测入参对象
 *
 * @author nwx1077006
 * @since 2021/10/25
 */
@Data
public class CopyAntiRansomwareReq {
    @JsonProperty("copy_id")
    private String copyId;

    private Integer status;

    private String model;

    @JsonProperty("detection_start_time")
    private Date detectionStartTime;

    @JsonProperty("detection_end_time")
    private Date detectionEndTime;

    private String report;

    @JsonProperty("tenant_id")
    private String tenantId;

    @JsonProperty("tenant_name")
    private String tenantName;
}