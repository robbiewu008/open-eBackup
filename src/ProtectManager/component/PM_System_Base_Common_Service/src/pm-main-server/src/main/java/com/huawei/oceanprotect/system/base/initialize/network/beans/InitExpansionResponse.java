/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.beans;

import lombok.Data;
import lombok.NoArgsConstructor;
import lombok.ToString;

import java.util.LinkedList;
import java.util.List;

/**
 * 响应参数
 *
 * @author swx1010572
 * @since 2021-06-16
 */
@Data
@ToString
@NoArgsConstructor
public class InitExpansionResponse {
    /**
     * 状态
     */
    private int error;

    /**
     * 进度错误编码
     */
    private String code;


    private String description;

    /**
     * 返回参数
     */
    private List<String> params = new LinkedList<>();
}
