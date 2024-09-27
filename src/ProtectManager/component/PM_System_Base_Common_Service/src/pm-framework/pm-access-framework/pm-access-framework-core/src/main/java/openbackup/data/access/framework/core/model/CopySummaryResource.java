package openbackup.data.access.framework.core.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * 副本聚合
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-03-01
 */
@Setter
@Getter
public class CopySummaryResource {
    private Integer copyCount;

    private String properties;

    private String protectedObjectUuid;

    private String protectedResourceId;

    private String protectedSlaId;

    private String protectedSlaName;

    @JsonProperty("protectedStatus")
    private Boolean isProtected;

    private String resourceEnvironmentIp;

    private String resourceEnvironmentName;

    private String resourceId;

    private String resourceLocation;

    private String resourceName;

    private String resourceProperties;

    private String resourceStatus;

    private String resourceSubType;

    private String resourceType;

    private String slaName;
}
