/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.vo;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 设备ESN和机器码
 *
 * @author n30046257
 * @since 2024-03-23
 */
@Getter
@Setter
@AllArgsConstructor
@NoArgsConstructor
public class DeviceInfo {
    /**
     * 设备esn
     */
    private String esn;

    /**
     * osa鉴权用户名
     */
    private String username;
}
