/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.emeistor.console.controller.request;

import lombok.Data;

import org.springframework.web.multipart.MultipartFile;

import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.NotNull;

/**
 * SSO保存配置请求体
 *
 * @author w30042425
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-08
 */
@Data
public class SSOConfigRequest {
    @NotNull
    private MultipartFile file;

    private String uuid;

    @NotEmpty
    private String configName;

    private String description;
}
