/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024 All rights reserved.
 */

package com.huawei.oceanprotect.system.base.service.strategy.deploy;

import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.springframework.web.multipart.MultipartFile;

/**
 * 部署形态策略接口
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-08-20
 */
public interface InitDeployTypeStrategyService {
    /**
     * 根据lld获取初始化网络配置信息
     *
     * @param lld lld
     * @return 网络配置信息
     */
    default InitNetworkBody getInitNetworkBodyByLLD(MultipartFile lld) {
        throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
            "Current device is not support init by lld.");
    }
}
