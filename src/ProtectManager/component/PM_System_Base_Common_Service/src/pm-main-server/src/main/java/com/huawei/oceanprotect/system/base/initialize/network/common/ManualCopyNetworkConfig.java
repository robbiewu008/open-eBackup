/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.common;

import com.huawei.oceanprotect.system.base.dto.dorado.ManualInitPortDto;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 手动初始化复制配置
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/4/5
 */
@Getter
@Setter
public class ManualCopyNetworkConfig {
    private List<ManualInitPortDto> logicPorts;
}
