/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.dto;

import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.protection.model.PolicyBo;

import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

/**
 * Copy Replication Metadata
 *
 * @author l00272247
 * @since 2020-12-15
 */
@Data
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class CopyReplicationMetadata {
    private String jobType;
    private String username;
    private PolicyBo replicationPolicy;
    private JSONObject resource;
    private JSONObject sla;
}
