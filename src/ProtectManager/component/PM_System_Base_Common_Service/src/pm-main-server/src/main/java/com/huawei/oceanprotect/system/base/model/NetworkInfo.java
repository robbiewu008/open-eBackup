/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.model;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 功能描述
 *
 * @author z00842230
 * @since 2024-03-07
 */

@AllArgsConstructor
@NoArgsConstructor
@Getter
@Setter
public class NetworkInfo {
    /**
     * 业务端口的IP
     */
    private String ip;

    /**
     * 业务端口的掩码
     */
    private String mask;

    /**
     * 业务端口的网关
     */
    private String gateway;

    /**
     * 业务端口的最大传输单元
     */
    private String mtu;
}