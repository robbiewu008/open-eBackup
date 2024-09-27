/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * cloud backup copy info
 *
 * @author g00500588
 * @since 2021/12/7
 */
@Data
public class DmeCloudBackupCopyInfo {
    // 单位秒
    @JsonProperty("CreateTime")
    private long createTime;

    // 1 增量， 0 全量
    @JsonProperty("Incremental")
    private int backupType;
}
