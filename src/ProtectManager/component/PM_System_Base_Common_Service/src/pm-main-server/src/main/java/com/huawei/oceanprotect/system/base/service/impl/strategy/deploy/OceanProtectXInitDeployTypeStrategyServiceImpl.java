/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
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
