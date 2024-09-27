/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.access.framework.resource.vo;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * dmc host trust code
 *
 * @author z30027603
 * @since 2023-01-12
 */
@Getter
@Setter
public class DmcHostTrustError {
    @JsonProperty("Code")
    private Integer code;

    @JsonProperty("Description")
    private String description;

    public DmcHostTrustError(Integer code, String description) {
        this.code = code;
        this.description = description;
    }
}
