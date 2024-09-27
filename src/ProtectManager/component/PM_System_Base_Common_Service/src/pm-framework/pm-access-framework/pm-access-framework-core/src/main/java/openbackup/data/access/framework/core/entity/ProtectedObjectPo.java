/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.core.entity;

import com.baomidou.mybatisplus.annotation.TableField;
import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import java.sql.Timestamp;

/**
 * Protected Object Po
 *
 * @author l00272247
 * @since 2021-10-20
 */
@TableName("PROTECTED_OBJECT")
public class ProtectedObjectPo {
    @TableId
    private String uuid;
    private String slaId;
    private String slaName;
    private String name;
    private String envId;
    private String envType;
    private String resourceId;
    private String resourceGroupId;
    private String type;
    private String subType;

    @TableField("sla_compliance")
    private Boolean isSlaCompliance;

    private String path;
    private String extParameters;
    private int status;
    private Timestamp latestTime;
    private Timestamp earliestTime;
    private String chainId;
    private String consistentStatus;
    private String consistentResults;

    public String getUuid() {
        return uuid;
    }

    public void setUuid(String uuid) {
        this.uuid = uuid;
    }

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

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
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

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }

    public String getSubType() {
        return subType;
    }

    public void setSubType(String subType) {
        this.subType = subType;
    }

    public Boolean getIsSlaCompliance() {
        return isSlaCompliance;
    }

    public void setSlaCompliance(Boolean isSlaCompliance) {
        this.isSlaCompliance = isSlaCompliance;
    }

    public String getPath() {
        return path;
    }

    public void setPath(String path) {
        this.path = path;
    }

    public String getExtParameters() {
        return extParameters;
    }

    public void setExtParameters(String extParameters) {
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

    public String getConsistentStatus() {
        return consistentStatus;
    }

    public void setConsistentStatus(String consistentStatus) {
        this.consistentStatus = consistentStatus;
    }

    public String getConsistentResults() {
        return consistentResults;
    }

    public void setConsistentResults(String consistentResults) {
        this.consistentResults = consistentResults;
    }
}
