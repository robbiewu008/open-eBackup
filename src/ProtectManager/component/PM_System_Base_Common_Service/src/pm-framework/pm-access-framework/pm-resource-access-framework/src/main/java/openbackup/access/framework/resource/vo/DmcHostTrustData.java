/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.access.framework.resource.vo;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * dmc host trust data
 *
 * @author z30027603
 * @since 2023-01-12
 */
@Getter
@Setter
public class DmcHostTrustData {
    @JsonProperty("IsTrusted")
    private Boolean isTrusted;
}
