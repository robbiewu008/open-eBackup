/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.livemount.common.model;

import openbackup.system.base.sdk.livemount.model.LiveMountTargetLocation;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import org.hibernate.validator.constraints.Length;

import java.util.List;
import java.util.Map;

import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.NotNull;

/**
 * Live Mount Create Request
 *
 * @author l00272247
 * @since 2020-09-18
 */
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class LiveMountObject {
    @Length(max = 512)
    private String name;

    @NotNull
    @Length(max = 128)
    private String sourceResourceId;

    @NotNull
    @Length(max = 64)
    private String copyId;

    @Length(max = 64)
    private String policyId;

    @NotNull
    private LiveMountTargetLocation targetLocation;

    @NotEmpty
    private List<String> targetResourceUuidList;

    private Map<String, Object> parameters;

    private List<LiveMountFileSystemShareInfo> fileSystemShareInfoList;

    private String exerciseJobId;

    private String exerciseId;

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getSourceResourceId() {
        return sourceResourceId;
    }

    public void setSourceResourceId(String sourceResourceId) {
        this.sourceResourceId = sourceResourceId;
    }

    public String getCopyId() {
        return copyId;
    }

    public void setCopyId(String copyId) {
        this.copyId = copyId;
    }

    public String getPolicyId() {
        return policyId;
    }

    public void setPolicyId(String policyId) {
        this.policyId = policyId;
    }

    public LiveMountTargetLocation getTargetLocation() {
        return targetLocation;
    }

    public void setTargetLocation(LiveMountTargetLocation targetLocation) {
        this.targetLocation = targetLocation;
    }

    public List<String> getTargetResourceUuidList() {
        return targetResourceUuidList;
    }

    public void setTargetResourceUuidList(List<String> targetResourceUuidList) {
        this.targetResourceUuidList = targetResourceUuidList;
    }

    public Map<String, Object> getParameters() {
        return parameters;
    }

    public void setParameters(Map<String, Object> parameters) {
        this.parameters = parameters;
    }

    public List<LiveMountFileSystemShareInfo> getFileSystemShareInfoList() {
        return fileSystemShareInfoList;
    }

    public void setFileSystemShareInfoList(List<LiveMountFileSystemShareInfo> fileSystemShareInfoList) {
        this.fileSystemShareInfoList = fileSystemShareInfoList;
    }

    public String getExerciseJobId() {
        return exerciseJobId;
    }

    public void setExerciseJobId(String exerciseJobId) {
        this.exerciseJobId = exerciseJobId;
    }

    public String getExerciseId() {
        return exerciseId;
    }

    public void setExerciseId(String exerciseId) {
        this.exerciseId = exerciseId;
    }
}
