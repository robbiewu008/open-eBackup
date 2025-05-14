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

import com.huawei.oceanprotect.system.base.service.strategy.deploy.InitDeployTypeStrategyService;

import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.service.DeployTypeService;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Map;

/**
 * InitDeployTypeStrategyContext
 *
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-08-20
 */
@Component
public class InitDeployTypeStrategyContext {
    private static final String DEPLOY_TYPE_STRATEGY_SERVICE_IMPL_SUFFIX = "InitDeployTypeStrategyServiceImpl";

    private static final String OCEAN_PROTECT_X_STRATEGY_SERVICE_IMPL_PREFIX = "oceanProtectX";

    private static final String E1000_STRATEGY_SERVICE_IMPL_PREFIX = "e1000";

    private static final String E6000_STRATEGY_SERVICE_IMPL_PREFIX = "e6000";

    @Autowired
    private Map<String, InitDeployTypeStrategyService> strategyServiceMap;

    @Autowired
    private DefaultInitDeployTypeStrategyServiceImpl defaultInitDeployTypeStrategyService;

    @Autowired
    private DeployTypeService deployTypeService;

    /**
     * 获取策略
     *
     * @return 对应的策略
     */
    public InitDeployTypeStrategyService getStrategyService() {
        DeployTypeEnum deployType = deployTypeService.getDeployType();

        if (DeployTypeService.X_SERIES.contains(deployType)) {
            return strategyServiceMap.get(OCEAN_PROTECT_X_STRATEGY_SERVICE_IMPL_PREFIX
                + DEPLOY_TYPE_STRATEGY_SERVICE_IMPL_SUFFIX);
        }
        if (deployTypeService.isE1000()) {
            return strategyServiceMap.get(E1000_STRATEGY_SERVICE_IMPL_PREFIX
                + DEPLOY_TYPE_STRATEGY_SERVICE_IMPL_SUFFIX);
        }
        if (DeployTypeEnum.E6000.equals(deployType)) {
            return strategyServiceMap.get(E6000_STRATEGY_SERVICE_IMPL_PREFIX
                + DEPLOY_TYPE_STRATEGY_SERVICE_IMPL_SUFFIX);
        }
        return defaultInitDeployTypeStrategyService;
    }
}
