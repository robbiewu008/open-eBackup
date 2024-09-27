/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.copy;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 复制原副本保留时间
 *
 * @since 2023-03-06
 */
@Data
public class ReplicationOriginCopyDuration {
    @JsonProperty("retention_type")
    private int retentionType;

    @JsonProperty("retention_duration")
    private int retentionDuration;

    @JsonProperty("duration_unit")
    private String durationUnit;
}
