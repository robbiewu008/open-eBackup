/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model;

import openbackup.system.base.sdk.accesspoint.model.DmeLocalDevice;
import openbackup.system.base.sdk.accesspoint.model.DmeRemoteDevice;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * Dme Remove Pair Request
 *
 * @author l00272247
 * @since 2020-12-16
 */
@Data
public class DmeRemovePairRequest {
    @JsonProperty("resourceid")
    private String resourceId;

    private List<String> targetEsns;

    private DmeLocalDevice localDevice;

    private DmeRemoteDevice remoteDevice;
}
