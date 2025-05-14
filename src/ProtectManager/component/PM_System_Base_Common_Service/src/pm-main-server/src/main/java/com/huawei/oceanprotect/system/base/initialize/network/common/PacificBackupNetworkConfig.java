/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.common;

import lombok.Getter;
import lombok.Setter;
import openbackup.system.base.sdk.devicemanager.request.NodeNetworkInfoRequest;

import java.util.List;

import javax.validation.Valid;
import javax.validation.constraints.Size;

/**
 * 分布式初始化备份网络配置
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/4/5
 */
@Getter
@Setter
public class PacificBackupNetworkConfig {
    @Valid
    @Size(min = 1, message = "The backup network pacific init network info is configured with at least one property")
    private List<NodeNetworkInfoRequest> pacificInitNetWorkInfoList;
}
