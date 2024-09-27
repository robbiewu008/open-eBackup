package openbackup.system.base.sdk.job.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * job listener bo
 *
 * @author h30003246
 * @version [OceanProtect 8.1.RC1]
 * @since 2020-04-01
 */
@Data
public class JobUpdateBo {
    @JsonProperty("job_request_id")
    private String jobRequestId;

    @JsonProperty("job_id")
    private String jobId;

    @JsonProperty("job_speed")
    private String speed;

    @JsonProperty("job_progress")
    private Integer progress;

    @JsonProperty("job_logs")
    private List<JobLogUpdateBo> jobLogs;
}
