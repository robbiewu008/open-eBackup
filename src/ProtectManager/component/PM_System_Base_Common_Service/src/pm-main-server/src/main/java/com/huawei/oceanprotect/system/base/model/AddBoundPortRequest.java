/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.model;

import com.huawei.oceanprotect.system.base.dto.dorado.BondPortDto;

import lombok.Getter;
import lombok.Setter;
import openbackup.system.base.sdk.system.model.StorageAuth;

import java.util.List;

import javax.validation.Valid;
import javax.validation.constraints.NotNull;

/**
 * 添加绑定端口请求对象
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023/12/29
 */

@Getter
@Setter
public class AddBoundPortRequest {
    @Valid
    private StorageAuth storageAuth;

    @NotNull
    private List<BondPortDto> bondPorts;
}
