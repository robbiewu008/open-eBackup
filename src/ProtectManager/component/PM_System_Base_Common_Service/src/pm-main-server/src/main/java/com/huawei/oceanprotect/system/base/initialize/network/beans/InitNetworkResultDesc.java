/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.beans;

import com.huawei.oceanprotect.system.base.initialize.network.enums.InitNetworkResultCode;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;
import lombok.ToString;

/**
 * 初始化动作结果描述
 *
 * @author swx1010572
 * @since 2021-01-15
 */
@Data
@ToString
@NoArgsConstructor
@AllArgsConstructor
public class InitNetworkResultDesc {
    /**
     * 动过结果编码
     */
    private InitNetworkResultCode code;

    /**
     * 动作结果描述
     */
    private String desc;
}
