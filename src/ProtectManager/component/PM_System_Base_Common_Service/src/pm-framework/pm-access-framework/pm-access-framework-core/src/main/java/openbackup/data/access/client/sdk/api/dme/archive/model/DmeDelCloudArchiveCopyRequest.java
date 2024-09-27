/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * DME Delete Cloud Archive request
 *
 * @author d00512967
 * @since 2020-12-12
 */
@Data
public class DmeDelCloudArchiveCopyRequest {
    @JsonProperty("RequestId")
    private String requestId;

    @JsonProperty("TaskID")
    private String taskId;

    @JsonProperty("ArchiveCopyId")
    private String archiveCopyId;

    @JsonProperty("Type")
    private Integer type;

    @JsonProperty("Cloud")
    private CloudArchiveInfo cloudArchiveInfo;

    @JsonProperty("ApplicationType")
    private Integer applicationType;

    // 0: 非强制 1:强制删除
    @JsonProperty("IsForce")
    private int isForce;
}
