/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024 All rights reserved.
 */

package com.huawei.oceanprotect.system.base.service.impl.strategy.deploy;

import com.huawei.oceanprotect.system.base.initialize.network.ability.InitializeNetworkBodyXlsAbility;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.service.strategy.deploy.InitDeployTypeStrategyService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import org.springframework.web.multipart.MultipartFile;

/**
 * OceanProtectXInitDeployTypeStrategyServiceImpl
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-08-20
 */
@Component
@Slf4j
public class OceanProtectXInitDeployTypeStrategyServiceImpl implements InitDeployTypeStrategyService {
    @Autowired
    private InitializeNetworkBodyXlsAbility initializeNetworkBodyXlsAbility;

    /**
     * 根据lld获取初始化网络配置信息
     *
     * @param lld lld
     * @return 网络配置信息
     */
    @Override
    public InitNetworkBody getInitNetworkBodyByLLD(MultipartFile lld) {
        return initializeNetworkBodyXlsAbility.checkAndReturnInitXls(lld);
    }
}
