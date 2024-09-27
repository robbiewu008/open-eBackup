/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.dme.archive.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 清理扫描归档任务 请求体
 *
 * @author z30009433
 * @since 2020-12-31
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class DmeCleanScanTaskRequest {
    @JsonProperty("TaskID")
    String taskId;
}
