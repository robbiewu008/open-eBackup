/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package com.huawei.emeistor.console.contant;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 异常对象类
 *
 * @author zKF66175
 * @version [V100R002C00, 2013-2-5]
 * @since 2019-10-25
 */
@NoArgsConstructor
@AllArgsConstructor
@Getter
@Setter
public class ErrorResponse {
    private String errorCode;

    private String errorMessage;

    @JsonProperty("parameters")
    private String[] detailParams;
}
