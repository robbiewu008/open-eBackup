package openbackup.data.protection.access.provider.sdk.job;

import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;

import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.List;

/**
 * job listener bo
 *
 * @author h30003246
 * @version [OceanProtect 8.1.RC1]
 * @since 2020-04-01
 */
public class ProviderJobMessage extends TaskBaseMessage {
    // 状态
    private ProviderJobStatusEnum status;

    @JsonProperty("job_speed")
    private String speed;

    @JsonProperty("job_progress")
    private Integer progress;

    @JsonProperty("job_logs")
    private List<ProviderJobLogMessage> jobLogs;

    public ProviderJobStatusEnum getStatus() {
        return status;
    }

    public void setStatus(ProviderJobStatusEnum status) {
        this.status = status;
    }

    public String getSpeed() {
        return speed;
    }

    public void setSpeed(String speed) {
        this.speed = speed;
    }

    public Integer getProgress() {
        return progress;
    }

    public void setProgress(Integer progress) {
        this.progress = progress;
    }

    public List<ProviderJobLogMessage> getJobLogs() {
        return jobLogs;
    }

    public void setJobLogs(List<ProviderJobLogMessage> jobLogs) {
        this.jobLogs = jobLogs;
    }
}