/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.dme.replicate;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 终止复制任务
 *
 * @author m00576658
 * @since 2021-01-13
 */
@Data
public class AdvanceReplicationJob {
    @JsonProperty("taskId")
    private String taskId;
}
