/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 更新归档副本PM元数据请求体
 *
 * @author z30009433
 * @since 2020-12-30
 */
@Data
public class DmeArchiveUpdateSnapRequest {
    @JsonProperty("Id")
    String archiveId;

    @JsonProperty("PMMetadata")
    String metadata;

    @JsonProperty("Type")
    int type;

    @JsonProperty("Cloud")
    CloudArchiveInfo cloud;
}
