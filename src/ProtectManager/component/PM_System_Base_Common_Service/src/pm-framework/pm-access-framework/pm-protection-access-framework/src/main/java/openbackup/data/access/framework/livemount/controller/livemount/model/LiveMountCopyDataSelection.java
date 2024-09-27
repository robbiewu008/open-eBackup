/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.livemount.controller.livemount.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import javax.validation.constraints.NotNull;

/**
 * Live Mount Copy Data Selection
 *
 * @author l00272247
 * @since 2020-09-17
 */
@Data
public class LiveMountCopyDataSelection {
    @JsonProperty("association_id")
    @NotNull
    private String associationId;

    @JsonProperty("policy_id")
    private String policyId;
}
