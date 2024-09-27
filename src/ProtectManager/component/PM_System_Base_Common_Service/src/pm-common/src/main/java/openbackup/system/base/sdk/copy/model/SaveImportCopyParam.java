/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.sdk.copy.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * SaveImportCopyParam
 *
 * @author g00500588
 * @since 2021/9/15
 */
@Data
public class SaveImportCopyParam {
    private String uuid;

    private String name;

    @JsonProperty("generated_time")
    private long generatedTime;

    @JsonProperty("generated_by")
    private String generatedBy;

    @JsonProperty("resource_type")
    private String resourceType;

    @JsonProperty("resource_sub_type")
    private String resourceSubType;

    @JsonProperty("resource_id")
    private String resourceId;

    @JsonProperty("chain_id")
    private String chainId;
}
