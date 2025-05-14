/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.controller.log.context;

import lombok.Getter;
import lombok.Setter;

/**
 * 保存初始化配置上下文结果
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/1/27
 */
@Getter
@Setter
public class InitConfigContextResult {
    private String role;

    private String homePortType;

    private String homePortId;
}
