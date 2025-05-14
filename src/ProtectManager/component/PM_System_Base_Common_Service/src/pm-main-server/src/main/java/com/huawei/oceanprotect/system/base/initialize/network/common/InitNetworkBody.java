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
 * 归档网络信息
 *
 * @author l00347293
 * @since 2020-02-02
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class InitNetworkBody {
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
    private BackupNetworkConfig backupNetworkConfig;

    /**
     * 归档网络
     */
    @Valid
    private ArchiveNetworkConfig archiveNetworkConfig;

    /**
     * 复制网络
     */
    @Valid
    private CopyNetworkConfig copyNetworkConfig;
}