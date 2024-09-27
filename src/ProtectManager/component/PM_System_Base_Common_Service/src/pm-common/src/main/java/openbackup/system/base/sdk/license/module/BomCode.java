/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.license.module;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * dorado activelicense 接口返回内容
 *
 * @author x30066966
 * @version [OceanProtect E6000]
 * @since 2024/8/29
 */

@Getter
@Setter
public class BomCode {
    @JsonProperty("bomCode")
    private String bomCode;

    @JsonProperty("bomResourceNum")
    private String bomResourceNum;
}
