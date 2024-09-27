/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
