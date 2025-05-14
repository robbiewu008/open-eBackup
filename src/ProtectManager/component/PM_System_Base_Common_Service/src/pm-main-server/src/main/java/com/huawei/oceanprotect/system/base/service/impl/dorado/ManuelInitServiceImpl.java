/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.service.impl.dorado;

import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.enums.InitType;
import com.huawei.oceanprotect.system.base.service.InitConfigMethodService;

import org.springframework.stereotype.Service;

/**
 * 手动初始化特有方法
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/4/5
 */
@Service
public class ManuelInitServiceImpl implements InitConfigMethodService {
    @Override
    public boolean applicable(InitType initType) {
        return initType.isManualInit();
    }

    @Override
    public void addLogicPort(InitNetworkBody initNetworkBody) {

    }
}
