/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.common;

import lombok.Getter;
import lombok.Setter;
import openbackup.system.base.sdk.devicemanager.request.NodeNetworkInfoRequest;

import java.util.List;

/**
 * 分布式初始化归档网络配置
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/4/5
 */
@Getter
@Setter
public class PacificArchiveNetworkConfig {
    private List<NodeNetworkInfoRequest> pacificInitNetWorkInfoList;
}
