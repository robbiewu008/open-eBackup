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
package openbackup.kingbase.protection.access.provider.resource;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.kingbase.protection.access.service.KingBaseService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * kingbase集群实例provider
 *
 */
@Component
@Slf4j
public class KingBaseClusterInstanceProvider implements ResourceProvider {
    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final InstanceResourceService instanceResourceService;

    private final KingBaseService kingBaseService;

    private ResourceService resourceService;

    public KingBaseClusterInstanceProvider(ProtectedEnvironmentService protectedEnvironmentService,
        InstanceResourceService instanceResourceService, KingBaseService kingBaseService) {
        this.protectedEnvironmentService = protectedEnvironmentService;
        this.instanceResourceService = instanceResourceService;
        this.kingBaseService = kingBaseService;
    }

    @Autowired
    public void setResourceService(ResourceService resourceService) {
        this.resourceService = resourceService;
    }

    @Override
    public void check(ProtectedResource resource) {
        log.info("Start create kingbase cluster instance check. resource name: {}", resource.getName());
        resource.setEnvironment(buildEnvironment(resource.getParentUuid()));

        // 校验集群实例是否已经注册
        instanceResourceService.checkClusterInstanceIsRegistered(resource);

        // 校验集群实例
        AgentBaseDto checkResult = instanceResourceService.checkIsClusterInstance(resource);

        // 设置集群实例的属性值
        setKingBaseClusterInstanceProperties(resource, checkResult);
        log.info("End create kingbase cluster instance check. resource name: {}", resource.getName());
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
    }

    private ProtectedEnvironment buildEnvironment(String environmentId) {
        return protectedEnvironmentService.getEnvironmentById(environmentId);
    }

    private void setKingBaseClusterInstanceProperties(ProtectedResource resource, AgentBaseDto checkResult) {
        Map<String, String> messageMap = JSONObject.fromObject(checkResult.getErrorMessage()).toMap(String.class);
        resource.setVersion(messageMap.get(DatabaseConstants.VERSION));
        resource.setPath(resource.getEnvironment().getEndpoint());
        resource.setExtendInfoByKey(DatabaseConstants.DB_MODE_KEY, messageMap.get(DatabaseConstants.DB_MODE_KEY));
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        // 设置集群实例节点角色
        instanceResourceService.setClusterInstanceNodeRole(resource);
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        log.info("Start update kingbase cluster instance check. resource name: {}", resource.getName());
        resource.setEnvironment(buildEnvironment(resource.getParentUuid()));
        fillPassword(resource);

        // 校验集群实例
        AgentBaseDto checkResult = instanceResourceService.checkIsClusterInstance(resource);

        // 设置集群实例的属性值
        setKingBaseClusterInstanceProperties(resource, checkResult);
        log.info("End update kingbase cluster instance check. resource name: {}", resource.getName());
    }

    private void fillPassword(ProtectedResource resource) {
        Map<String, List<ProtectedResource>> dependencies = Optional.ofNullable(resource.getDependencies())
            .orElseGet(HashMap::new);
        List<ProtectedResource> resources = Optional.ofNullable(dependencies.get(DatabaseConstants.CHILDREN))
            .orElse(Collections.emptyList());
        resources.forEach(instance -> {
            if (instance.getAuth().getAuthType() == Authentication.APP_PASSWORD && VerifyUtil.isEmpty(
                instance.getAuth().getAuthPwd())) {
                instance.getAuth()
                    .setAuthPwd(kingBaseService.getResourceById(instance.getUuid()).getAuth().getAuthPwd());
            }
        });
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.KING_BASE_CLUSTER_INSTANCE.equalsSubType(protectedResource.getSubType());
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
