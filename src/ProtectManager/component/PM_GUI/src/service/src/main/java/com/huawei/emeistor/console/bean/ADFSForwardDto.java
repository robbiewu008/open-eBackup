/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.emeistor.console.bean;

import lombok.Data;

/**
 * ADFS跳转URL
 *
 * @author w30042425
 * @since 2023-05-18
 */
@Data
public class ADFSForwardDto {
    /**
     * 回调地址
     */
    private String callbackUrl;

    /**
     * ADFS地址
     */
    private String providerUrl;

    /**
     * 客户端id
     */
    private String clientId;

    /**
     * 跳转URL
     */
    private String forwardUrl;

    /**
     * 登出跳转url
     */
    private String logoutForwardUrl;
}
