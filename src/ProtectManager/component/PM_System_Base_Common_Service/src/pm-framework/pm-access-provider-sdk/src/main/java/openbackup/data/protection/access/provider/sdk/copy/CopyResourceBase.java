/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.data.protection.access.provider.sdk.copy;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

/**
 * 副本资源基类
 *
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
