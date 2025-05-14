/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.common;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 配置状态
 *
 * @author xwx1016404
 * @since 2021-10-01
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class ConfigStatusKey {
    /**
     * 状态
     */
    private String statusKey;

    /**
     * 进度错误编码
     */
    private String codeKey;

    /**
     * 进度描述
     */
    private String descKey;

    /**
     * 进度比率
     */
    private String rateKey;

    /**
     * 进度错误码对应参数列表
     */
    private String errorCodeParamKey;

    /**
     * 存储在Redis Map中的状态标识
     */
    private String initRedisFlagKey;
}