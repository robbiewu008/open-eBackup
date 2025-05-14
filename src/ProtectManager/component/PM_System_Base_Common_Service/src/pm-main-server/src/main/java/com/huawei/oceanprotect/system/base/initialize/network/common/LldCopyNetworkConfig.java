/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.common;

import com.huawei.oceanprotect.system.base.dto.dorado.LogicPortDto;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * lld初始化复制网络配置
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/4/5
 */
@Getter
@Setter
public class LldCopyNetworkConfig {
    private List<LogicPortDto> logicPorts;
}
