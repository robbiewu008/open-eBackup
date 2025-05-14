/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.dto.dorado;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortRole;

import lombok.Getter;
import lombok.Setter;
import openbackup.system.base.common.enums.AddressFamily;

/**
 * 修改逻辑端口参数
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/1/25
 */
@Getter
@Setter
public class ModifyLogicPortDto {
    private String id;

    private String name;

    private String ip;

    private String mask;

    private String gateWay;

    private AddressFamily addressFamily;

    private Boolean isFailOver;

    private PortRole role;

    private String failoverGroupId;
}
