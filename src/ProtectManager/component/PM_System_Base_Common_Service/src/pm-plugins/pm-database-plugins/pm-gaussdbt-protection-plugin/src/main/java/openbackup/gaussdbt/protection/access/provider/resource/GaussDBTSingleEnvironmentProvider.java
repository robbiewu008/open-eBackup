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
package openbackup.gaussdbt.protection.access.provider.resource;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.gaussdbt.protection.access.provider.service.GaussDBTSingleService;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

/**
 * GaussDBT单机版环境provider
 *
 */
@Slf4j
@Component
public class GaussDBTSingleEnvironmentProvider extends DatabaseEnvironmentProvider {
    private GaussDBTSingleService gaussDBTSingleService;

    /**
     * Constructor
     *
     * @param providerManager providerManager
     * @param pluginConfigManager pluginConfigManager
     * @param gaussDBTSingleService gaussDBTSingleService
     */
    public GaussDBTSingleEnvironmentProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        GaussDBTSingleService gaussDBTSingleService) {
        super(providerManager, pluginConfigManager);
        this.gaussDBTSingleService = gaussDBTSingleService;
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        if (VerifyUtil.isEmpty(environment.getUuid())) {
            beforeCreate(environment);
        } else {
            beforeUpdate(environment);
        }
    }

    private void beforeCreate(ProtectedEnvironment environment) {
        log.info("Start create gaussdbt single parameters check. resource name: {}, uuid: {}", environment.getName(),
            environment.getExtendInfoByKey(DatabaseConstants.HOST_ID));
        // 检查单机资源是否注册
        gaussDBTSingleService.checkSingleIsRegistered(environment);
        // 检查连通性
        String checkResult = gaussDBTSingleService.checkConnection(environment);
        // 填充资源属性值
        gaussDBTSingleService.fillGaussDBTSingleProperties(environment, checkResult);
        log.info("End create gaussdbt single parameters check. resource name: {}, uuid: {}", environment.getName(),
            environment.getExtendInfoByKey(DatabaseConstants.HOST_ID));
    }

    private void beforeUpdate(ProtectedEnvironment environment) {
        log.info("Start update gaussdbt single parameters check. resource name: {}, uuid: {}", environment.getName(),
            environment.getExtendInfoByKey(DatabaseConstants.HOST_ID));
        // 检查单机资源的用户名是否被修改
        gaussDBTSingleService.checkSingleInstallUserIsChanged(environment);
        // 检查连通性
        String checkResult = gaussDBTSingleService.checkConnection(environment);
        // 填充资源属性值
        gaussDBTSingleService.fillGaussDBTSingleProperties(environment, checkResult);
        log.info("End update gaussdbt single parameters check. resource name: {}, uuid: {}", environment.getName(),
            environment.getExtendInfoByKey(DatabaseConstants.HOST_ID));
    }

    @Override
    public void validate(ProtectedEnvironment environment) {
        gaussDBTSingleService.checkGaussDTSingleStatus(environment);
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.GAUSSDBT_SINGLE.equalsSubType(subType);
    }
}
