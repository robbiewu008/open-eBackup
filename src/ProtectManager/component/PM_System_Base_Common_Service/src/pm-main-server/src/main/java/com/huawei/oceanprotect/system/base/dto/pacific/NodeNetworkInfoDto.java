/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.dto.pacific;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.Setter;
import openbackup.system.base.sdk.devicemanager.entity.IpPoolDto;

import java.util.List;

/**
 * pacific 某节点上的业务网络配置信息
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-05
 */
@Getter
@Setter
@AllArgsConstructor
public class NodeNetworkInfoDto {
    private String manageIp;
    private List<IpPoolDto> ipPoolDtoList;
}
