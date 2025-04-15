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
package openbackup.antdb.protection.access.provider.resource;

import com.google.common.collect.ImmutableMap;

import lombok.extern.slf4j.Slf4j;
import openbackup.antdb.protection.access.common.AntDBConstants;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * AntDB集群实例provider
 *
 */
@Component
@Slf4j
public class AntDBClusterInstanceProvider implements ResourceProvider {
    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final InstanceResourceService instanceResourceService;

    private final ResourceService resourceService;

    public AntDBClusterInstanceProvider(ProtectedEnvironmentService protectedEnvironmentService,
        InstanceResourceService instanceResourceService, ResourceService resourceService) {
        this.protectedEnvironmentService = protectedEnvironmentService;
        this.instanceResourceService = instanceResourceService;
        this.resourceService = resourceService;
    }

    @Override
    public boolean supplyDependency(ProtectedResource resource) {
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> children = resourceService.queryDependencyResources(true, DatabaseConstants.CHILDREN,
            Collections.singletonList(resource.getUuid()));
        children.stream().forEach(child -> {
            Map<String, List<ProtectedResource>> childDependencies = new HashMap<>();
            List<ProtectedResource> agents = resourceService.queryDependencyResources(true, DatabaseConstants.AGENTS,
                Collections.singletonList(child.getUuid()));
            childDependencies.put(DatabaseConstants.AGENTS, agents);
            child.setDependencies(childDependencies);
        });
        dependencies.put(DatabaseConstants.CHILDREN, children);
        resource.setDependencies(dependencies);
        return true;
    }

    @Override
    public void check(ProtectedResource resource) {
        log.info("Start create antdb cluster instance check. resource name: {}", resource.getName());
        checkCluster(resource);
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        log.info("End create antdb cluster instance check. resource name: {}", resource.getName());
    }

    private void checkCluster(ProtectedResource resource) {
        List<ProtectedResource> children = resource.getDependencies().get(DatabaseConstants.CHILDREN);
        // 构造environment，以使用公共方法
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setSubType(ResourceSubTypeEnum.ANT_DB.getType());
        List<ProtectedResource> agents = children.stream()
            .map(child -> protectedEnvironmentService.getEnvironmentById(
                child.getDependencies().get(DatabaseConstants.AGENTS).get((IsmNumberConstant.ZERO)).getUuid()))
            .collect(Collectors.toList());
        environment.setDependencies(ImmutableMap.of(DatabaseConstants.AGENTS, agents));
        resource.setEnvironment(environment);
        // 校验集群实例
        // 检查连通性获取版本信息
        AgentBaseDto checkResult = instanceResourceService.checkClusterInstance(resource);
        if (Long.parseLong(checkResult.getErrorCode()) != DatabaseConstants.SUCCESS_CODE) {
            JSONObject jsonObject = JSONObject.fromObject(checkResult.getErrorMessage());
            throw new LegoCheckedException(Long.parseLong(jsonObject.getString("bodyErr")),
                jsonObject.getString("message"));
        }
        setAntDBClusterInstanceVersion(resource, checkResult);
        try {
            // 查询集群详情
            AppEnvResponse clusterInstanceInfo = instanceResourceService.queryClusterInstanceNodeRoleByAgent(resource);
            Map<String, List<NodeInfo>> appEnvMap = clusterInstanceInfo.getNodes()
                .stream()
                .collect(Collectors.groupingBy(NodeInfo::getEndpoint));
            resource.getDependencies()
                .get(DatabaseConstants.CHILDREN)
                .forEach(childNode -> buildClusterNodeRole(childNode, appEnvMap));
        } catch (LegoCheckedException | NullPointerException e) {
            log.error("Create antdb cluster instance check error", ExceptionUtil.getErrorMessage(e));
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "target environment is offline.");
        }
        // 如果没有主节点直接报错
        Optional<ProtectedResource> childOptional = children.stream()
            .filter(
                item -> StringUtils.equals(AntDBConstants.PRIMARY, item.getExtendInfo().get(DatabaseConstants.ROLE)))
            .findFirst();
        if (!childOptional.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "target environment is offline.");
        }
        environment = protectedEnvironmentService.getEnvironmentById(children.get(IsmNumberConstant.ZERO)
            .getDependencies()
            .get(DatabaseConstants.AGENTS)
            .get((IsmNumberConstant.ZERO))
            .getUuid());
        resource.setEnvironment(environment);
        resource.setRootUuid(environment.getUuid());
        resource.setParentUuid(environment.getUuid());
    }

    private void buildClusterNodeRole(ProtectedResource childNode, Map<String, List<NodeInfo>> appEnvMap) {
        childNode.getExtendInfo()
            .put(DatabaseConstants.ROLE, appEnvMap.get(childNode.getExtendInfoByKey(DatabaseConstants.SERVICE_IP))
                .get(IsmNumberConstant.ZERO)
                .getExtendInfo()
                .get(DatabaseConstants.ROLE));
        childNode.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
            appEnvMap.get(childNode.getExtendInfoByKey(DatabaseConstants.SERVICE_IP))
                .get(IsmNumberConstant.ZERO)
                .getExtendInfo()
                .get(DatabaseConstants.STATUS));
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
        resource.setPath(resource.getName());
    }

    private void setAntDBClusterInstanceVersion(ProtectedResource resource, AgentBaseDto checkResult) {
        Map<String, String> messageMap = JSONObject.fromObject(checkResult.getErrorMessage()).toMap(String.class);
        resource.setVersion(messageMap.getOrDefault(AntDBConstants.PG_VERSION, ""));
        resource.setExtendInfoByKey(AntDBConstants.ANTDB_VERSION,
            messageMap.getOrDefault(AntDBConstants.ANTDB_VERSION, ""));
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        log.info("Start update antdb cluster instance check. resource name: {}", resource.getName());
        checkCluster(resource);
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        log.info("End update antdb cluster instance check. resource name: {}", resource.getName());
    }

    @Override
    public void healthCheck(ProtectedResource resource) {
        log.info("Start AntDB cluster instance healthCheck");
        ProtectedResource updatedResource = resourceService.getResourceById(resource.getUuid())
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "not exist"));
        for (ProtectedResource child : updatedResource.getDependencies().get(DatabaseConstants.CHILDREN)) {
            ProtectedEnvironment environment = protectedEnvironmentService.getEnvironmentById(
                child.getDependencies().get(DatabaseConstants.AGENTS).get(0).getUuid());
            child.setEnvironment(environment);
            child.setName(updatedResource.getName());
            instanceResourceService.healthCheckSingleInstance(child);
        }
        log.info("Finish AntDB cluster single instance healthCheck");
        for (ProtectedResource child : updatedResource.getDependencies().get(DatabaseConstants.CHILDREN)) {
            if (LinkStatusEnum.OFFLINE.getStatus()
                .toString()
                .equals(child.getExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY))) {
                updatedResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
                    LinkStatusEnum.OFFLINE.getStatus().toString());
                updateResourceStatus(updatedResource);
                log.info("Finish AntDB cluster instance healthCheck with offline");
                return;
            }
        }
        updatedResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
            LinkStatusEnum.ONLINE.getStatus().toString());
        updateResourceStatus(updatedResource);
        log.info("Finish AntDB cluster instance healthCheck with online");
    }

    private void updateResourceStatus(ProtectedResource resource) {
        ProtectedResource updateResource = new ProtectedResource();
        updateResource.setUuid(resource.getUuid());
        updateResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
            resource.getExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY));
        resourceService.updateSourceDirectly(Collections.singletonList(updateResource));
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.ANT_DB_CLUSTER_INSTANCE.equalsSubType(protectedResource.getSubType());
    }
}