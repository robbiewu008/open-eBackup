/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.beans;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.LinkedList;
import java.util.List;

/**
 * DeviceManager回应错误信息
 *
 * @author w00493811
 * @since 2020-12-12
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class ExpansionResponseError {
    /**
     * 进度错误编码
     */
    private String code;

    /**
     * 返回参数
     */
    private List<String> errorParam = new LinkedList<>();
}