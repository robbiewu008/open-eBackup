package openbackup.system.base.sdk.copy.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

/**
 * Copy Resource Summary
 *
 * @author l00272247
 * @since 2020-11-09
 */
@Data
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class CopyResourceSummary {
    private String resourceId;

    private String resourceName;

    private String resourceType;

    private String resourceSubType;

    private String resourceLocation;

    private String resourceStatus;

    private String resourceProperties;

    private String resourceEnvironmentName;

    private String resourceEnvironmentIp;

    private String slaName;

    private int copyCount;

    private String protectedResourceId;
}
