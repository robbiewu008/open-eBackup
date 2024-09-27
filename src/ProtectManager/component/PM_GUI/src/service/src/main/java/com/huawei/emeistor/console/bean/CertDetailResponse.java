/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.emeistor.console.bean;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 证书详情对象
 *
 * @author fwx1022842
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022/3/18
 */
@Data
public class CertDetailResponse {
    /**
     * 是否安全
     */
    @JsonProperty("safety")
    private Boolean isSafety;
}
