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
package openbackup.sqlserver.resources.access.provider;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * sqlserver集群 新加provider实现resourceprovider
 *
 */
@Component
@Slf4j
public class SqlServerClusterResourceProvider implements ResourceProvider {
    private ResourceService resourceService;

    @Autowired
    public void setResourceService(ResourceService resourceService) {
        this.resourceService = resourceService;
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
    }

    /**
     * 资源重名检查配置
     *
     * @return SQL Server实例注册重名检查配置
     */
    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = ResourceProvider.super.getResourceFeature();
        // SQL Server集群扫描不需要刷新主机信息
        resourceFeature.setShouldUpdateDependencyHostInfoWhenScan(false);
        return resourceFeature;
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
    public boolean applicable(ProtectedResource protectedResource) {
        return Objects.nonNull(protectedResource) && Objects.equals(protectedResource.getSubType(),
            ResourceSubTypeEnum.SQL_SERVER_CLUSTER.getType());
    }
}