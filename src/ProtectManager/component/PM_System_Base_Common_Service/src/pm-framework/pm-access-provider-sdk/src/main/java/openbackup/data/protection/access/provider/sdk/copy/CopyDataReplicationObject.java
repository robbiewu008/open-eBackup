/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.copy;

import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Copy Data Replication Object
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2020-06-17
 */
@Data
public class CopyDataReplicationObject {
    @JsonProperty("copy_id")
    private ProtectedObject protectedObject;

    @JsonProperty("replication_target")
    private CopyDataReplicationTarget replicationTarget;
}
