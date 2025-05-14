/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.beans;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * DeviceManager回应模板
 *
 * @param <T> 数据类型
 * @author swx1010572
 * @since 2021-06-22
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class ExpansionResponse<T> {
    /**
     * 回应数据
     */
    @JsonProperty("data")
    private T data;

    /**
     * 回应错误码
     */
    @JsonProperty("error")
    private ExpansionResponseError error;

    /**
     * 这个错误码不常用，好像是api接口的会使用，注意！这里是为了兼容
     */
    @JsonProperty("result")
    private ExpansionResponseError result;
}