/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.dto.pacific;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * pacific 多个节点的业务网络配置信息
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-05
 */
@Getter
@Setter
@AllArgsConstructor
public class NetworkInfoDto {
    private List<NodeNetworkInfoDto> nodeNetworkInfoList;
}
