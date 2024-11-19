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
package openbackup.gaussdbt.protection.access.provider;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.provider.DatabaseResourceProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * GaussDBT资源更新Provider
 *
 */
@Slf4j
@Component
public class GaussDBTResourceProvider extends DatabaseResourceProvider {
    private ResourceService resourceService;

    /**
     * GaussDBTResourceProvider
     *
     * @param providerManager provider manager
     * @param pluginConfigManager provider config manager
     * @param resourceService 资源服务
     */
    public GaussDBTResourceProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        ResourceService resourceService) {
        super(providerManager, pluginConfigManager);
        this.resourceService = resourceService;
    }

    /**
     * 更新GaussDBT资源，EnvironmentProvider中做连通性检查，不使用数据库框架再做校验
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeUpdate(ProtectedResource resource) {
        log.info("GaussDBT healthCheck and update GaussDBT environment nodes info.environment:{}", resource.getUuid());
    }

    @Override
    public boolean supplyDependency(ProtectedResource resource) {
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> agents = resourceService.queryDependencyResources(true, DatabaseConstants.AGENTS,
            Collections.singletonList(resource.getUuid()));
        dependencies.put(DatabaseConstants.AGENTS, agents);
        resource.setDependencies(dependencies);
        return true;
    }

    @Override
    public boolean applicable(ProtectedResource resource) {
        return ResourceSubTypeEnum.GAUSSDBT.getType().equals(resource.getSubType());
    }
}
