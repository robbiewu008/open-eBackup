package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 更新 import job
 *
 * @author z00633516
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-04-08
 */
@Data
public class UpdateImportJobRequest {
    @JsonProperty("CleanScanTaskResult")
    Boolean isCleanScanTaskResult;

    @JsonProperty("JobId")
    String jobId;
}
