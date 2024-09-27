/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.devicemanager.entity;

import lombok.Getter;
import lombok.Setter;

/**
 * pacific业务网络的ip和端口
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-02-01
 */
@Getter
@Setter
public class PacificIpInfo {
    // ip地址/掩码
    private String ipAddress;

    // 端口名称
    private String ifaceName;
}
