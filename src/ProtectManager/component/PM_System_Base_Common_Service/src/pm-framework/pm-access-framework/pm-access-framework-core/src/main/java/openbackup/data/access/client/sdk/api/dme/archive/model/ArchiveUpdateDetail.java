/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * DME更新归档副本返回体
 *
 * @since 2021-01-07
 */
@Data
public class ArchiveUpdateDetail {
    @JsonProperty("TaskId")
    private String taskId;
}
