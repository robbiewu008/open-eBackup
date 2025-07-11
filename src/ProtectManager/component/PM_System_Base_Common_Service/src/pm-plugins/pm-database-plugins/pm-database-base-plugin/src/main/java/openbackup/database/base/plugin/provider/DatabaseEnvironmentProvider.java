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
package openbackup.database.base.plugin.provider;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfig;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Collection;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * The DatabaseEnvironmentProvider
 *
 */
@Slf4j
@Component
public class DatabaseEnvironmentProvider implements EnvironmentProvider {
    /**
     * provider管控
     */
    protected final ProviderManager providerManager;

    /**
     * 插件配置管控
     */
    protected final PluginConfigManager pluginConfigManager;

    /**
     * DatabaseResourceProvider
     *
     * @param providerManager provider manager
     * @param pluginConfigManager provider config manager
     */
    public DatabaseEnvironmentProvider(final ProviderManager providerManager,
        final PluginConfigManager pluginConfigManager) {
        this.providerManager = providerManager;
        this.pluginConfigManager = pluginConfigManager;
    }

    @Override
    public boolean applicable(final String resourceSubType) {
        Set<String> subTypeSet = pluginConfigManager.getPluginConfigs()
            .stream()
            .filter(e -> Objects.equals(e.getType(), ResourceTypeEnum.DATABASE.getType()))
            .map(PluginConfig::getSubType)
            .collect(Collectors.toSet());
        Collection<DatabaseEnvironmentProvider> providers = providerManager.findProviders(
            DatabaseEnvironmentProvider.class);
        for (DatabaseEnvironmentProvider provider : providers) {
            if (Objects.equals(provider.getClass(), DatabaseEnvironmentProvider.class)) {
                continue;
            }
            // 有其他的实现，则跳过该实现
            if (provider.applicable(resourceSubType)) {
                return false;
            }
        }
        return subTypeSet.contains(resourceSubType);
    }

    @Override
    public void register(final ProtectedEnvironment environment) {
        log.info("Preparing to check the environment before creating, sub type: {}.", environment.getSubType());
    }

    @Override
    public void validate(final ProtectedEnvironment environment) {
        ResourceCheckContext checkContext = providerManager.findProvider(ResourceConnectionCheckProvider.class,
            environment).checkConnection(environment);
        healthCheck(environment, checkContext.getContext());
    }

    /**
     * 创建前检查
     *
     * @param environment 资源
     * @param context 检查上下文
     */
    protected void healthCheck(ProtectedEnvironment environment, Map<String, Object> context) {
        log.info("The heartbeat check of the database environment ends, uuid: {}, sub type: {}", environment.getUuid(),
            environment.getSubType());
    }
}
