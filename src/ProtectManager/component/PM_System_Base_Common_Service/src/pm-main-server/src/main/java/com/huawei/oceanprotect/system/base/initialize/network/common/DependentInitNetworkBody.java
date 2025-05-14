/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.common;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.Data;
import openbackup.system.base.sdk.system.model.StorageAuth;

import javax.validation.Valid;
import javax.validation.constraints.NotNull;

/**
 * 软硬解耦初始化参数
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/4/5
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class DependentInitNetworkBody {
    /**
     * 存储爱数安装语言
     */
    @Valid
    private ConfigLanguage configLanguage;

    /**
     * 存储用户名、密码
     */
    @Valid
    private StorageAuth storageAuth;

    /**
     * 备份网络
     */
    @Valid
    @NotNull
    private DependentBackupNetworkConfig backupNetworkConfig;

    /**
     * 归档网络
     */
    @Valid
    private DependentArchiveNetworkConfig archiveNetworkConfig;

    /**
     * 复制网络
     */
    @Valid
    private DependentCopyNetworkConfig copyNetworkConfig;
}