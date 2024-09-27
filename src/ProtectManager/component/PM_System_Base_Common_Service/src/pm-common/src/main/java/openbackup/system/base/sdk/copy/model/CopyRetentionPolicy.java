package openbackup.system.base.sdk.copy.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

/**
 * Copy Retention Policy
 *
 * @author l00272247
 * @since 2020-09-27
 */
@Data
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class CopyRetentionPolicy {
    private int retentionType;

    private int retentionDuration;

    private String durationUnit;
}
