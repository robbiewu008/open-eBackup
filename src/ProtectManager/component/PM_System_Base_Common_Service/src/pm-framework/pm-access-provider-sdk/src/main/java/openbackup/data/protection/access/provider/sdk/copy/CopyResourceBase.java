package openbackup.data.protection.access.provider.sdk.copy;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

/**
 * 副本资源基类
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-10-22
 */
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class CopyResourceBase {
    @JsonProperty("resource_id")
    private String resourceId;

    @JsonProperty("resource_name")
    private String resourceName;

    @JsonProperty("resource_type")
    private String resourceType;

    @JsonProperty("resource_sub_type")
    private String resourceSubType;

    @JsonProperty("resource_location")
    private String resourceLocation;

    @JsonProperty("resource_status")
    private String resourceStatus;

    @JsonProperty("resource_properties")
    private String resourceProperties;

    @JsonProperty("resource_environment_name")
    private String resourceEnvironmentName;

    @JsonProperty("resource_environment_ip")
    private String resourceEnvironmentIp;

    public String getResourceId() {
        return resourceId;
    }

    public void setResourceId(String resourceId) {
        this.resourceId = resourceId;
    }

    public String getResourceName() {
        return resourceName;
    }

    public void setResourceName(String resourceName) {
        this.resourceName = resourceName;
    }

    public String getResourceType() {
        return resourceType;
    }

    public void setResourceType(String resourceType) {
        this.resourceType = resourceType;
    }

    public String getResourceSubType() {
        return resourceSubType;
    }

    public void setResourceSubType(String resourceSubType) {
        this.resourceSubType = resourceSubType;
    }

    public String getResourceLocation() {
        return resourceLocation;
    }

    public void setResourceLocation(String resourceLocation) {
        this.resourceLocation = resourceLocation;
    }

    public String getResourceStatus() {
        return resourceStatus;
    }

    public void setResourceStatus(String resourceStatus) {
        this.resourceStatus = resourceStatus;
    }

    public String getResourceProperties() {
        return resourceProperties;
    }

    public void setResourceProperties(String resourceProperties) {
        this.resourceProperties = resourceProperties;
    }

    public String getResourceEnvironmentName() {
        return resourceEnvironmentName;
    }

    public void setResourceEnvironmentName(String resourceEnvironmentName) {
        this.resourceEnvironmentName = resourceEnvironmentName;
    }

    public String getResourceEnvironmentIp() {
        return resourceEnvironmentIp;
    }

    public void setResourceEnvironmentIp(String resourceEnvironmentIp) {
        this.resourceEnvironmentIp = resourceEnvironmentIp;
    }
}
