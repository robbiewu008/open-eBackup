/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.model;

import lombok.Getter;
import lombok.Setter;
import openbackup.system.base.sdk.system.model.StorageAuth;

import java.util.Map;

import javax.validation.Valid;

/**
 * 查询逻辑端口参数
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/1/26
 */
@Setter
@Getter
public class QueryLogicPortRequest {
    @Valid
    private StorageAuth storageAuth;

    private Map<String, String> condition;
}
