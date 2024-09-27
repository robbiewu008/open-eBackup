/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.emeistor.console.controller.request;

import lombok.Data;

import org.springframework.web.multipart.MultipartFile;

import javax.validation.constraints.NotNull;

/**
 * 导入证书请求体
 *
 * @author wx1011919
 * @since 2021-06-29
 */
@Data
public class ImportCertificateFiles {
    @NotNull
    MultipartFile caCertificate;

    MultipartFile serverCertificate;

    MultipartFile serverKey;

    MultipartFile agentCertificate;

    MultipartFile agentKey;

    MultipartFile dhParam;
}
