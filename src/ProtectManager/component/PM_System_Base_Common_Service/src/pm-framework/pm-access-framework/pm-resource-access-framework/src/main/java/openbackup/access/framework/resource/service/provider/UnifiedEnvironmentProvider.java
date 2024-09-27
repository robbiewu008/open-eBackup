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
package openbackup.access.framework.resource.service.provider;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfig;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigConstants;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentHealthCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceScanProvider;

import com.fasterxml.jackson.databind.JsonNode;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Optional;

/**
 * 功能描述: 通用资源管理Provider
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-13
 */
@Slf4j
@Component
public class UnifiedEnvironmentProvider implements EnvironmentProvider {
    private final PluginConfigManager pluginConfigManager;
    private final ProviderManager providerManager;
    private final UnifiedResourceScanProvider unifiedScanProvider;
    private final UnifiedEnvironmentCheckProvider unifiedCheckProvider;
    private final UnifiedHealthCheckProvider unifiedHealthCheckProvider;

    /**
     * 构造器注入
     *
     * @param pluginConfigManager pluginConfigManager
     * @param providerManager providerManager
     * @param unifiedScanProvider unifiedScanProvider
     * @param unifiedCheckProvider unifiedCheckProvider
     * @param unifiedHealthCheckProvider unifiedHealthCheckProvider
     */
    public UnifiedEnvironmentProvider(PluginConfigManager pluginConfigManager, ProviderManager providerManager,
            UnifiedResourceScanProvider unifiedScanProvider, UnifiedEnvironmentCheckProvider unifiedCheckProvider,
            UnifiedHealthCheckProvider unifiedHealthCheckProvider) {
        this.pluginConfigManager = pluginConfigManager;
        this.providerManager = providerManager;
        this.unifiedScanProvider = unifiedScanProvider;
        this.unifiedCheckProvider = unifiedCheckProvider;
        this.unifiedHealthCheckProvider = unifiedHealthCheckProvider;
    }

    @Override
    public boolean applicable(String subtype) {
        Optional<PluginConfig> optConfig = pluginConfigManager.getPluginConfig(subtype);
        return optConfig.map(PluginConfig::getConfigMap)
            .map(configMap -> configMap.get(PluginConfigConstants.FUNCTION))
            .map(functionNode -> functionNode.findValue(PluginConfigConstants.ENVIRONMENTS))
            .map(envNode -> envNode.findValue(PluginConfigConstants.USE_UNIFIED_PROVIDER))
            .map(JsonNode::asBoolean)
            .orElse(false);
    }

    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        ResourceScanProvider scanProvider = providerManager.findProviderOrDefault(ResourceScanProvider.class,
                environment, unifiedScanProvider);
        return scanProvider.scan(environment);
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        EnvironmentCheckProvider provider = providerManager.findProviderOrDefault(EnvironmentCheckProvider.class,
                environment, unifiedCheckProvider);
        provider.check(environment);
    }

    @Override
    public void validate(ProtectedEnvironment environment) {
        EnvironmentHealthCheckProvider provider = providerManager
                .findProviderOrDefault(EnvironmentHealthCheckProvider.class, environment, unifiedHealthCheckProvider);
        provider.healthCheck(environment);
    }
}