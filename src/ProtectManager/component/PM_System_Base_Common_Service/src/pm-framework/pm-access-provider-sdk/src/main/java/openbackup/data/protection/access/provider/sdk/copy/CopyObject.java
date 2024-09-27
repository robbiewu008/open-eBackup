/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.copy;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Copy Object
 *
 * @author l00557046
 * @version [BCManager 8.0.0]
 * @since 2020-06-17
 */
@Data
public class CopyObject {
    private String copyId;

    private String timeStamp;

    @JsonProperty("resource_id")
    private String resourceId;

    @JsonProperty("resource_properties")
    private String resourceProperties;
}

