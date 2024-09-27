/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.devicemanager.entity;

import lombok.Getter;
import lombok.Setter;

import java.util.List;
import java.util.Map;

/**
 * pacific节点的信息
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-19
 */
@Getter
@Setter
public class NodeInfoDto {
    // 节点基本信息
    private PacificNodeInfoVo pacificNodeInfoVo;

    // 节点绑定的端口的绑定类型信息
    private Map<String, String> portBondMap;

    // 节点的业务网信息
    private List<IpPoolDto> ipList;
}
