/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.service;

import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.enums.InitType;

import openbackup.system.base.util.Applicable;

/**
 * 初始化配置方式服务
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/4/5
 */
public interface InitConfigMethodService extends Applicable<InitType> {
    /**
     * 添加逻辑端口
     *
     * @param initNetworkBody 请求参数
     */
    void addLogicPort(InitNetworkBody initNetworkBody);
}
