/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.config.achive;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * DME Response Error
 *
 * @author l00272247
 * @since 2020-04-03
 */
@Getter
@Setter
public class DmeResponseError {
    @JsonProperty("Code")
    private long code;
    @JsonProperty("Description")
    private String description;
    @JsonProperty("Params")
    private String[] detailParams;
}

