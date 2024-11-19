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
package openbackup.gaussdbdws.protection.access.provider;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.provider.DatabaseResourceProvider;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * GaussDBDWS DWS资源相关接口的具体实现类
 * 实现了：健康状态检查，环境信息检查相关等接口
 *
 */
@Slf4j
@Component
public class GaussDBDWSClusterResourceProvider extends DatabaseResourceProvider {
    private ResourceService resourceService;

    /**
     * Constructor
     *
     * @param providerManager providerManager
     * @param pluginConfigManager pluginConfigManager
     */
    public GaussDBDWSClusterResourceProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager) {
        super(providerManager, pluginConfigManager);
    }

    @Autowired
    public void setResourceService(ResourceService resourceService) {
        this.resourceService = resourceService;
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.GAUSSDB_DWS.getType().equals(protectedResource.getSubType());
    }

    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = super.getResourceFeature();
        resourceFeature.setSupportedLanFree(false);
        return resourceFeature;
    }

    @Override
    public boolean supplyDependency(ProtectedResource resource) {
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> clusterAgents = resourceService.queryDependencyResources(true,
            DwsConstant.DWS_CLUSTER_AGENT, Collections.singletonList(resource.getUuid()));
        List<ProtectedResource> hostAgents = resourceService.queryDependencyResources(true, DwsConstant.HOST_AGENT,
            Collections.singletonList(resource.getUuid()));
        dependencies.put(DwsConstant.DWS_CLUSTER_AGENT, clusterAgents);
        dependencies.put(DwsConstant.HOST_AGENT, hostAgents);
        resource.setDependencies(dependencies);
        return true;
    }
}
