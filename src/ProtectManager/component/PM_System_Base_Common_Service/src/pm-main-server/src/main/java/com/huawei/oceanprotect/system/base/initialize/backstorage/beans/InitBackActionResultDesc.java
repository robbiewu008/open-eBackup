/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.backstorage.beans;

import com.huawei.oceanprotect.system.base.initialize.backstorage.enums.InitBackActionResultCode;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.ToString;

/**
 * 初始化动作结果描述
 *
 * @author w00493811
 * @since 2020-12-28
 */
@Data
@ToString
@AllArgsConstructor
public class InitBackActionResultDesc {
    /**
     * 动过结果编码
     */
    private InitBackActionResultCode code;

    /**
     * 动作结果描述
     */
    private String desc;
}