/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.emeistor.console.controller.request;

import lombok.Data;
import lombok.ToString;

import org.springframework.web.multipart.MultipartFile;

/**
 * ADFS保存配置请求体
 *
 * @author y30021475
 * @since 2023-05-15
 */
@Data
@ToString(exclude = {"clientPwd"})
public class ADFSConfigRequest {
    /**
     * 是否开启ADFS
     */
    private boolean isAdfsEnable;

    private String configName;

    /**
     * ADFS地址
     */
    private String providerUrl;

    /**
     * 客户端id
     */
    private String clientId;

    /**
     * 客户端秘钥
     */
    private String clientPwd;

    /**
     * CA证书
     */
    private MultipartFile caFile;
}
