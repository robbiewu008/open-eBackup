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
package openbackup.data.access.framework.livemount.common.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import openbackup.system.base.sdk.livemount.model.LiveMountTargetLocation;

import org.hibernate.validator.constraints.Length;

import java.util.List;
import java.util.Map;

import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.NotNull;

/**
 * Live Mount Create Request
 *
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
