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

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;

import lombok.extern.slf4j.Slf4j;

import java.util.Map;

/**
 * The DatabaseResourceProvider
 *
 * @author g30003063
 * @since 2022-05-30
 */
@Slf4j
public class DatabaseResourceProvider implements ResourceProvider {
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
    public DatabaseResourceProvider(final ProviderManager providerManager,
        final PluginConfigManager pluginConfigManager) {
        this.providerManager = providerManager;
        this.pluginConfigManager = pluginConfigManager;
    }

    @Override
    public boolean applicable(final ProtectedResource protectedResource) {
        return false;
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
        ResourceCheckContext context = providerManager.findProvider(ResourceConnectionCheckProvider.class, resource)
            .checkConnection(resource);
        beforeCreate(resource, context.getContext());
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        ResourceCheckContext context = providerManager.findProvider(ResourceConnectionCheckProvider.class, resource)
            .checkConnection(resource);
        beforeUpdate(resource, context.getContext());
    }

    /**
     * 创建前检查
     *
     * @param resource 资源
     * @param context 检查上下文
     */
    protected void beforeCreate(ProtectedResource resource, Map<String, Object> context) {
        log.info("Resource check success before create.");
    }

    /**
     * 修改前检查
     *
     * @param resource 资源
     * @param context 检查上下文
     */
    protected void beforeUpdate(ProtectedResource resource, Map<String, Object> context) {
        log.info("Resource check success before update.");
    }
}