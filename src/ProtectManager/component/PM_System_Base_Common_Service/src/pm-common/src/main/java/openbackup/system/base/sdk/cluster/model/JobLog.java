package openbackup.system.base.sdk.cluster.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * 任务进度
 *
 * @author w00607005
 * @since 2023-05-23
 */
@Data
public class JobLog {
    @JsonProperty("log_info")
    private String logInfo;

    @JsonProperty("log_info_param")
    private List<String> logInfoParam;

    @JsonProperty("log_timestamp")
    private Long logTimestamp;

    @JsonProperty("log_detail")
    private Integer logDetail;

    @JsonProperty("log_detail_param")
    private List<String> logDetailParam;

    @JsonProperty("log_detail_info")
    private List<String> logDetailInfo;

    @JsonProperty("log_level")
    private Integer logLevel;

    @JsonProperty("unique")
    private boolean isUnique;
}
