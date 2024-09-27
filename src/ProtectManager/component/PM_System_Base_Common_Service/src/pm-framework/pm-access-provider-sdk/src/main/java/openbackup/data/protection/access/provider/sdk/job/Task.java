package openbackup.data.protection.access.provider.sdk.job;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Task produced by backup、restore
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2020-06-17
 */
@Data
public class Task {
    private String uuid;
    private String status;
    private int progress;
    @JsonProperty("copy_id")
    private String copyId;
}
