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
package openbackup.data.protection.access.provider.sdk.resource;

import com.fasterxml.jackson.annotation.JsonProperty;

import java.sql.Timestamp;
import java.util.Map;

/**
 * Protected Object
 *
 * @author l00272247
 * @since 2021-10-20
 */
public class ProtectedObject extends ResourceBase {
    private String slaId;
    private String slaName;
    private String envId;
    private String envType;
    private String resourceId;
    private String resourceGroupId;

    @JsonProperty("slaCompliance")
    private Boolean isSlaCompliance;

    /**
     * 约定:extParameter不能保存敏感信息
     */
    private Map<String, Object> extParameters;
    private int status;
    private Timestamp latestTime;
    private Timestamp earliestTime;
    private String chainId;

    public String getSlaId() {
        return slaId;
    }

    public void setSlaId(String slaId) {
        this.slaId = slaId;
    }

    public String getSlaName() {
        return slaName;
    }

    public void setSlaName(String slaName) {
        this.slaName = slaName;
    }

    public String getEnvId() {
        return envId;
    }

    public void setEnvId(String envId) {
        this.envId = envId;
    }

    public String getEnvType() {
        return envType;
    }

    public void setEnvType(String envType) {
        this.envType = envType;
    }

    public String getResourceId() {
        return resourceId;
    }

    public void setResourceId(String resourceId) {
        this.resourceId = resourceId;
    }
    public String getResourceGroupId() {
        return resourceGroupId;
    }

    public void setResourceGroupId(String resourceGroupId) {
        this.resourceGroupId = resourceGroupId;
    }

    public Boolean getSlaCompliance() {
        return isSlaCompliance;
    }

    public void setSlaCompliance(Boolean isSlaCompliance) {
        this.isSlaCompliance = isSlaCompliance;
    }

    public Map<String, Object> getExtParameters() {
        return extParameters;
    }

    public void setExtParameters(Map<String, Object> extParameters) {
        this.extParameters = extParameters;
    }

    public int getStatus() {
        return status;
    }

    public void setStatus(int status) {
        this.status = status;
    }

    public Timestamp getLatestTime() {
        return latestTime;
    }

    public void setLatestTime(Timestamp latestTime) {
        this.latestTime = latestTime;
    }

    public Timestamp getEarliestTime() {
        return earliestTime;
    }

    public void setEarliestTime(Timestamp earliestTime) {
        this.earliestTime = earliestTime;
    }

    public String getChainId() {
        return chainId;
    }

    public void setChainId(String chainId) {
        this.chainId = chainId;
    }
}
