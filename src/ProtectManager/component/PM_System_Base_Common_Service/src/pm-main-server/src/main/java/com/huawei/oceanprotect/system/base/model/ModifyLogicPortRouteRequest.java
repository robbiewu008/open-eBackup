/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.model;

import openbackup.system.base.sdk.devicemanager.entity.PortRouteInfo;

import lombok.Getter;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotBlank;

/**
 * ModifyLogicPortRouteRequest
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-02-20
 */
@Getter
@Setter
public class ModifyLogicPortRouteRequest {
    @Length(max = 256)
    @NotBlank
    private String portName;

    private PortRouteInfo route;
}
