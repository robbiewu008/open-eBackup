/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * VMWare Cluster config info
 *
 * @author z00595268
 * @since 2021-07-14
 */
@Data
public class ClusterConfig {
    @JsonProperty("drs_enabled")
    private boolean drsEnabled;
}
