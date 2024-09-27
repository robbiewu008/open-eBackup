/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.job;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 功能描述
 *
 * @author y30000858
 * @since 2020-09-21
 */
@Data
public class ExtendsInfo {
    @JsonProperty("backup_id")
    private String backupId;
    @JsonProperty("instance_id")
    private String instanceId;
    @JsonProperty("backup_damaged")
    private boolean isCopyDamaged;
}
