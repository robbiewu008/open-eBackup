/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.dto.pacific;

import lombok.Getter;
import lombok.Setter;

import java.util.Map;

/**
 * 每个节点的名称和端口绑定信息
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-19
 */
@Getter
@Setter
public class NodeInfoDto {
    // 节点名称
    private String name;

    // 节点管理ip
    private String manageIp;

    // 节点绑定的端口的绑定类型信息
    private Map<String, String> portBondMap;
}
