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
package openbackup.dameng.protection.access.provider;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.dameng.protection.access.constant.DamengConstant;
import openbackup.dameng.protection.access.service.DamengService;
import openbackup.dameng.protection.access.util.DamengParamCheckUtil;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * dameng集群实例注册
 *
 */
@Slf4j
@Component
public class DamengClusterProvider extends DatabaseEnvironmentProvider {
    private final DamengService damengService;

    private final ResourceService resourceService;

    private final ProtectedResourceChecker protectedResourceChecker;

    /**
     * 构造方法
     *
     * @param providerManager provider管理器
     * @param pluginConfigManager 插件配置管理器
     * @param damengService dameng服务
     * @param resourceService 资源服务
     * @param protectedResourceChecker 检查checker
     */
    public DamengClusterProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        DamengService damengService, ResourceService resourceService,
        @Qualifier("unifiedResourceConnectionChecker") ProtectedResourceChecker protectedResourceChecker) {
        super(providerManager, pluginConfigManager);
        this.damengService = damengService;
        this.resourceService = resourceService;
        this.protectedResourceChecker = protectedResourceChecker;
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.DAMENG_CLUSTER.getType().equals(resourceSubType);
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("start to check dameng cluster: {}.", environment.getName());
        EnvironmentParamCheckUtil.checkEnvironmentNamePattern(environment.getName());
        checkClusterParams(environment.getDependencies().get(DatabaseConstants.CHILDREN));
        checkClusterExist(environment);
        updateEnvironment(environment);
        environment.setUuid(Optional.ofNullable(environment.getUuid()).orElse(UUIDGenerator.getUUID()));
    }

    private void checkClusterParams(List<ProtectedResource> protectedResources) {
        protectedResources.forEach(resource -> {
            DamengParamCheckUtil.checkPort(resource.getExtendInfoByKey(DatabaseConstants.PORT));
            DamengParamCheckUtil.checkAuthKey(resource.getAuth().getAuthKey());
        });
    }

    private void checkClusterExist(ProtectedEnvironment environment) {
        List<ProtectedResource> instances = environment.getDependencies().get(DatabaseConstants.CHILDREN);
        Set<String> uuidAndPortSet = damengService.getExistingUuidAndPort(environment);
        List<String> childrenList = new ArrayList<>();
        for (ProtectedResource instance : instances) {
            ProtectedEnvironment agentEnv = damengService.queryAgentEnvironment(instance);
            String uuidAndPort = damengService.connectUuidAndPort(agentEnv.getUuid(),
                instance.getExtendInfo().get(DatabaseConstants.PORT));
            if (uuidAndPortSet.contains(uuidAndPort)) {
                log.error("The Dameng cluster exists registered instance:{}.", uuidAndPort);
                throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODE_IS_REGISTERED,
                    "The Dameng cluster exists registered instance.");
            }
            if (childrenList.contains(uuidAndPort)) {
                log.error("The Dameng cluster params exists same instance:{}.", uuidAndPort);
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                    "The Dameng cluster params exists same instance.");
            }
            childrenList.add(uuidAndPort);
        }
    }

    private void updateEnvironment(ProtectedEnvironment environment) {
        providerManager.findProvider(ResourceConnectionCheckProvider.class, environment).checkConnection(environment);
        List<AppEnvResponse> appEnvResponseList = damengService.check(environment);
        List<ProtectedResource> deleteChildren = environment.getDependencies().get(DatabaseConstants.DELETE_CHILDREN);
        if (CollectionUtils.isNotEmpty(deleteChildren)) {
            checkDeleteChildren(deleteChildren, environment);
        }
        List<ProtectedResource> instances = environment.getDependencies().get(DatabaseConstants.CHILDREN);
        List<NodeInfo> nodeInfoList = new ArrayList<>();
        for (ProtectedResource instance : instances) {
            ProtectedEnvironment agentEnv = damengService.queryAgentEnvironment(instance);
            appEnvResponseList.forEach(response -> {
                NodeInfo subNodeInfo = getNodeInfo(response, agentEnv, instance);
                if (!VerifyUtil.isEmpty(subNodeInfo.getUuid())) {
                    updateLinkStatus(environment, subNodeInfo, agentEnv);
                    nodeInfoList.add(subNodeInfo);
                }
            });
        }
        Map<String, String> extendInfo = Optional.ofNullable(environment.getExtendInfo()).orElse(new HashMap<>());
        extendInfo.put(DamengConstant.NODES, JSONObject.writeValueAsString(nodeInfoList));
        AppEnvResponse response = appEnvResponseList.get(IsmNumberConstant.ZERO);
        extendInfo.put(DamengConstant.VERSION, response.getExtendInfo().get(DamengConstant.VERSION));
        extendInfo.put(DamengConstant.BIG_VERSION, response.getExtendInfo().get(DamengConstant.BIG_VERSION));
        environment.setExtendInfo(extendInfo);
    }

    private void checkDeleteChildren(List<ProtectedResource> deleteChildren, ProtectedEnvironment environment) {
        List<String> childrenIds = deleteChildren.stream().map(ProtectedResource::getUuid).collect(Collectors.toList());
        List<String> instanceIds = damengService.getEnvironmentById(environment.getUuid())
            .getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toList());
        if (!instanceIds.containsAll(childrenIds)) {
            log.error("Delete children id:{} is not exist instance", childrenIds);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Delete children id not exist instance");
        }
    }

    private void updateLinkStatus(ProtectedEnvironment environment, NodeInfo subNodeInfo,
        ProtectedEnvironment agentEnv) {
        Map<String, String> extendInfo = subNodeInfo.getExtendInfo();
        if (!Objects.equals(extendInfo.get(DamengConstant.ROLE), DamengConstant.PRIMARY)) {
            return;
        }
        environment.setPath(agentEnv.getEndpoint());
        environment.setEndpoint(agentEnv.getEndpoint());
        String instanceStatus = extendInfo.get(DamengConstant.INSTANCESTATUS);
        if (LinkStatusEnum.OFFLINE.getStatus().toString().equals(instanceStatus)) {
            environment.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
            log.info("The Dameng instance:{} is offLine.", subNodeInfo.getUuid());
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "Protected environment is offLine!");
        }
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    private NodeInfo getNodeInfo(AppEnvResponse response, ProtectedEnvironment agentEnv, ProtectedResource instance) {
        NodeInfo filterNodeInfo = response.getNodes()
            .stream()
            .filter(nodeInfo -> filterNodeInfo(nodeInfo, agentEnv.getUuid(), instance))
            .findFirst()
            .orElse(null);
        if (VerifyUtil.isEmpty(filterNodeInfo)) {
            return new NodeInfo();
        }
        filterNodeInfo.setName(agentEnv.getName());
        Map<String, String> extendInfo = Optional.ofNullable(filterNodeInfo.getExtendInfo()).orElse(new HashMap<>());
        extendInfo.put(DamengConstant.AUTH_TYPE, String.valueOf(instance.getAuth().getAuthType()));
        extendInfo.put(DatabaseConstants.END_POINT, filterNodeInfo.getEndpoint());
        filterNodeInfo.setEndpoint(agentEnv.getEndpoint());
        filterNodeInfo.setExtendInfo(extendInfo);
        return filterNodeInfo;
    }

    private Boolean filterNodeInfo(NodeInfo nodeInfo, String agentUuid, ProtectedResource instance) {
        return Objects.equals(nodeInfo.getUuid(), agentUuid) && Objects.equals(
            nodeInfo.getExtendInfo().get(DatabaseConstants.PORT), instance.getExtendInfo().get(DatabaseConstants.PORT))
            && nodeInfo.getExtendInfo().get(DamengConstant.INSTANCESTATUS) != null;
    }

    @Override
    public Optional<String> healthCheckWithResultStatus(ProtectedEnvironment environment) {
        log.info("The Dameng cluster healthCheck start, uuid: {}.", environment.getUuid());
        List<NodeInfo> nodeInfoList = damengService.getNodeInfoFromNodes(environment);
        // 更新实例状态
        updateInstanceStatus(environment, nodeInfoList);
        // 更新主备关系
        updateRole(environment, nodeInfoList);
        // 更新environment
        String linkStatus = updateEnv(environment, nodeInfoList);
        return Optional.of(linkStatus);
    }

    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        log.info("start DamengClusterProvider scan");
        List<NodeInfo> nodeInfoList = damengService.getNodeInfoFromNodes(environment);
        // 更新主备关系
        updateRole(environment, nodeInfoList);
        ProtectedEnvironment newEnv = new ProtectedEnvironment();
        newEnv.setUuid(environment.getUuid());
        newEnv.setExtendInfo(environment.getExtendInfo());
        resourceService.updateSourceDirectly(Collections.singletonList(newEnv));
        log.info("end DamengClusterProvider scan");
        return Collections.emptyList();
    }

    private void updateInstanceStatus(ProtectedEnvironment environment, List<NodeInfo> nodeInfoList) {
        ProtectedResourceChecker resourceChecker = providerManager.findProviderOrDefault(ProtectedResourceChecker.class,
            environment, protectedResourceChecker);
        Map<ProtectedResource, List<ProtectedEnvironment>> resourceConnectableMap =
            resourceChecker.collectConnectableResources(environment);
        for (Map.Entry<ProtectedResource, List<ProtectedEnvironment>> entry : resourceConnectableMap.entrySet()) {
            ProtectedResource checkingResource = entry.getKey();
            for (ProtectedEnvironment env : entry.getValue()) {
                String uuidAndPort = damengService.connectUuidAndPort(env.getUuid(),
                    checkingResource.getExtendInfo().get(DatabaseConstants.PORT));
                checkingResource.setEnvironment(env);
                CheckResult checkResult = resourceChecker.generateCheckResult(checkingResource);
                ActionResult actionResult = checkResult.getResults();
                if (actionResult.getCode() == ActionResult.SUCCESS_CODE) {
                    updateStatus(uuidAndPort, nodeInfoList, LinkStatusEnum.ONLINE.getStatus().toString());
                } else {
                    updateStatus(uuidAndPort, nodeInfoList, LinkStatusEnum.OFFLINE.getStatus().toString());
                }
            }
        }
        environment.getExtendInfo().put(DamengConstant.NODES, JSONObject.writeValueAsString(nodeInfoList));
    }

    /**
     * 更新集群状态，存在主节点离线则置为离线
     *
     * @param environment 环境
     * @param nodeInfoList 节点列表
     * @return 连接状态
     */
    private String updateEnv(ProtectedEnvironment environment, List<NodeInfo> nodeInfoList) {
        boolean isExistPrimaryOffline = nodeInfoList.stream().anyMatch(nodeInfo -> checkInstanceStatus(nodeInfo));
        String linkStatus = LinkStatusEnum.ONLINE.getStatus().toString();
        if (isExistPrimaryOffline) {
            linkStatus = LinkStatusEnum.OFFLINE.getStatus().toString();
        }
        ProtectedEnvironment newEnv = new ProtectedEnvironment();
        newEnv.setUuid(environment.getUuid());
        newEnv.setLinkStatus(linkStatus);
        newEnv.setExtendInfo(environment.getExtendInfo());
        resourceService.updateSourceDirectly(Collections.singletonList(newEnv));
        return linkStatus;
    }

    private boolean checkInstanceStatus(NodeInfo nodeInfo) {
        Map<String, String> extendInfo = nodeInfo.getExtendInfo();
        return Objects.equals(extendInfo.get(DamengConstant.ROLE), DamengConstant.PRIMARY) && Objects.equals(
            extendInfo.get(DamengConstant.INSTANCESTATUS), LinkStatusEnum.OFFLINE.getStatus().toString());
    }

    private void updateStatus(String uuidAndPort, List<NodeInfo> nodeInfoList, String linkStatus) {
        nodeInfoList.stream().forEach(nodeInfo -> {
            String nodeUuidAndPort = damengService.connectUuidAndPort(nodeInfo.getUuid(),
                nodeInfo.getExtendInfo().get(DatabaseConstants.PORT));
            if (Objects.equals(nodeUuidAndPort, uuidAndPort)) {
                nodeInfo.getExtendInfo().put(DamengConstant.INSTANCESTATUS, linkStatus);
            }
        });
    }

    private void updateRole(ProtectedEnvironment environment, List<NodeInfo> nodeInfoList) {
        List<ProtectedResource> instances = environment.getDependencies().get(DatabaseConstants.CHILDREN);
        List<AppEnvResponse> appEnvResponseList = damengService.queryClusterInfo(instances);
        for (AppEnvResponse response : appEnvResponseList) {
            if (VerifyUtil.isEmpty(response.getNodes())) {
                continue;
            }
            NodeInfo resNodeInfo = response.getNodes()
                .stream()
                .filter(nodeInfo -> !VerifyUtil.isEmpty(nodeInfo.getUuid()))
                .findFirst()
                .orElse(null);
            if (VerifyUtil.isEmpty(resNodeInfo)) {
                continue;
            }
            Map<String, String> resExtendInfo = resNodeInfo.getExtendInfo();
            String resUuidAndPort = damengService.connectUuidAndPort(resNodeInfo.getUuid(),
                resExtendInfo.get(DatabaseConstants.PORT));
            nodeInfoList.forEach(nodeInfo -> {
                Map<String, String> extendInfo = nodeInfo.getExtendInfo();
                String nodeUuidAndPort = damengService.connectUuidAndPort(nodeInfo.getUuid(),
                    extendInfo.get(DatabaseConstants.PORT));
                if (Objects.equals(resUuidAndPort, nodeUuidAndPort)) {
                    extendInfo.put(DamengConstant.ROLE, resExtendInfo.get(DamengConstant.ROLE));
                }
            });
        }
        environment.getExtendInfo().put(DamengConstant.NODES, JSONObject.writeValueAsString(nodeInfoList));
    }
}
