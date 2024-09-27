/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 功能描述
 *
 * @author dWX1009286
 * @since 2021-07-26
 */
@Data
public class ClusterResourceCount {
    @JsonProperty("resource_sub_type")
    private String subType;

    @JsonProperty("resource_type")
    private String resourceType;

    @JsonProperty("protected_count")
    private int protectedCount;

    @JsonProperty("unprotected_count")
    private int unprotectedCount;
}
