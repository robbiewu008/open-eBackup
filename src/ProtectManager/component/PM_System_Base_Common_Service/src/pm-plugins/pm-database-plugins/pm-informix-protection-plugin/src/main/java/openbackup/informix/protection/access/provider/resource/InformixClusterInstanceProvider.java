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
package openbackup.informix.protection.access.provider.resource;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.informix.protection.access.service.InformixService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * informix集群实例注册provider
 *
 */
@Component
@Slf4j
public class InformixClusterInstanceProvider implements ResourceProvider {
    private final InformixService informixService;

    private ResourceService resourceService;

    /**
     * InformixClusterInstanceProvider
     *
     * @param informixService informixService
     */
    public InformixClusterInstanceProvider(InformixService informixService) {
        this.informixService = informixService;
    }

    @Autowired
    public void setResourceService(ResourceService resourceService) {
        this.resourceService = resourceService;
    }

    /**
     * 资源重名检查配置
     *
     * @return ResourceFeature
     */
    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = new ResourceFeature();
        resourceFeature.setShouldCheckResourceNameDuplicate(false);
        return resourceFeature;
    }

    /**
     * provider过滤器，过滤条件接口
     *
     * @param protectedResource object 受保护资源
     * @return boolean true 获取该bean false过滤掉该bean
     */
    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.INFORMIX_CLUSTER_INSTANCE.getType().equals(protectedResource.getSubType());
    }

    /**
     * 检查informix实例资源连通性检查
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码
     *
     * @param resource 受保护资源
     */
    @Override
    public void check(ProtectedResource resource) {
        log.info("Start to create informix cluster instance. resource name: {}, resource subtype : {}",
            resource.getName(), resource.getSubType());
        informixService.doClusterInstanceAction(resource, true);
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
    }

    /**
     * 检查受保护资源informix实例修改连通性检查
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeUpdate(ProtectedResource resource) {
    }

    @Override
    public void updateCheck(ProtectedResource resource) {
        log.info("Start to update informix cluster instance. resource name: {}, resource subtype : {}",
            resource.getName(), resource.getSubType());
        informixService.doClusterInstanceAction(resource, false);
    }

    @Override
    public void healthCheck(ProtectedResource resource) {
        log.info("Start to check cluster health.");
        informixService.healthCheckOfClusterInstance(resource);
    }

    @Override
    public ResourceDeleteContext preHandleDelete(ProtectedResource resource) {
        // 删除文件系统
        informixService.removeRepositoryOfResource(resource.getUuid());
        return ResourceDeleteContext.defaultValue();
    }

    @Override
    public boolean supplyDependency(ProtectedResource resource) {
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> children = resourceService.queryDependencyResources(true, DatabaseConstants.CHILDREN,
            Collections.singletonList(resource.getUuid()));
        for (ProtectedResource child : children) {
            Map<String, List<ProtectedResource>> childDependencies = new HashMap<>();
            List<ProtectedResource> agents = resourceService.queryDependencyResources(true, DatabaseConstants.AGENTS,
                Collections.singletonList(child.getUuid()));
            childDependencies.put(DatabaseConstants.AGENTS, agents);
            child.setDependencies(childDependencies);
        }
        dependencies.put(DatabaseConstants.CHILDREN, children);
        resource.setDependencies(dependencies);
        return true;
    }
}
