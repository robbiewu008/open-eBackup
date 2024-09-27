/*
 *
 *  * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 */

package openbackup.access.framework.resource.vo;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * dmc host trust vo
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2023-01-12
 */
@Getter
@Setter
public class DmcHostTrustVo {
    @JsonProperty("Data")
    private DmcHostTrustData data;

    @JsonProperty("Error")
    private DmcHostTrustError error;

    public DmcHostTrustVo(DmcHostTrustData data, DmcHostTrustError error) {
        this.data = data;
        this.error = error;
    }
}
