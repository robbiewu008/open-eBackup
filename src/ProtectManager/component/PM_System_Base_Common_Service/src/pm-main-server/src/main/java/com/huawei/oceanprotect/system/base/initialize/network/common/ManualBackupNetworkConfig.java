/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.common;

import com.huawei.oceanprotect.system.base.dto.dorado.ManualInitPortDto;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

import javax.validation.Valid;
import javax.validation.constraints.Size;

/**
 * 手动初始化备份配置
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/4/5
 */
@Getter
@Setter
public class ManualBackupNetworkConfig {
    @Valid
    @Size(min = 1, message = "The backup network logic port is configured with at least one property")
    private List<ManualInitPortDto> logicPorts;
}
