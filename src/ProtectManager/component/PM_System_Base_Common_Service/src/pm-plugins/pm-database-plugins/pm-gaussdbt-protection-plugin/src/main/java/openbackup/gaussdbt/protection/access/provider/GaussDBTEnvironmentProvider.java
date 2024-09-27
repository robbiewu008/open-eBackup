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

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTClusterStateEnum;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTConstant;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTNodeStatusEnum;
import openbackup.gaussdbt.protection.access.provider.util.GaussDBTClusterUtil;
import openbackup.gaussdbt.protection.access.provider.util.GaussDBTValidator;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.OptionalUtil;
import openbackup.system.base.util.StreamUtil;

import com.fasterxml.jackson.core.type.TypeReference;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * GaussDBT资源健康检查
 *
 * @author hwx1144169
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-31
 */
@Slf4j
@Component
public class GaussDBTEnvironmentProvider extends DatabaseEnvironmentProvider {
    private final ResourceService resourceService;

    /**
     * Constructor
     *
     * @param providerManager providerManager
     * @param pluginConfigManager pluginConfigManager
     * @param resourceService resourceService
     */
    public GaussDBTEnvironmentProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        ResourceService resourceService) {
        super(providerManager, pluginConfigManager);
        this.resourceService = resourceService;
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.GAUSSDBT.getType().equals(resourceSubType);
    }

    /**
     * 检查: 1、添加唯一UUID；2、检查主机是否已注册；3.检查资源是否重复
     *
     * @param environment environment
     */
    @Override
    public void register(ProtectedEnvironment environment) {
        EnvironmentParamCheckUtil.checkEnvironmentNamePattern(environment.getName());
        if (VerifyUtil.isEmpty(environment.getUuid())) {
            log.info("start to register GaussDBT cluster preCheck.");
            preCheckWhenRegister(environment);
        } else {
            log.info("start to update GaussDBT cluster preCheck. uuid:{}", environment.getUuid());
            preCheckWhenUpdate(environment);
        }
    }

    /**
     * 注册前检查
     *
     * @param environment 环境信息
     */
    private void preCheckWhenRegister(ProtectedEnvironment environment) {
        // 获取GaussDBT环境资源,校验数量
        List<ProtectedEnvironment> existEnvironments = getGaussDBTEnvironments();
        GaussDBTValidator.verifyCount(existEnvironments.size());
        // 获取前端传入的agents,检查agent主机是否已经注册了GaussDBT
        Set<String> agentsUuids = getAgents(environment);
        checkHostRegistered(agentsUuids, existEnvironments);
        // 生成环境资源唯一UUID,检查uuid是否已经存在
        String uuid = buildUniqueUuid(agentsUuids);
        checkDuplicatedEnvironment(uuid);
        environment.setUuid(uuid);
        // 连通性检测,检查集群信息是否正确
        ResourceCheckContext checkContext = checkClusterConnection(environment);
        AppEnvResponse appEnvResponse = getCheckResult(checkContext);
        checkClusterInfo(environment, appEnvResponse, agentsUuids);
        // 构建集群参数
        GaussDBTClusterUtil.setClusterInfo(environment, appEnvResponse, checkContext);
        log.info("preCreate, build GaussDBT environment info success.uuid:{}", environment.getUuid());
    }

    /**
     * 更新前检查
     *
     * @param environment 环境信息
     */
    private void preCheckWhenUpdate(ProtectedEnvironment environment) {
        // 检查资源是否存在
        ProtectedEnvironment oldEnvironment = resourceService.getResourceById(environment.getUuid())
            .flatMap(OptionalUtil.match(ProtectedEnvironment.class))
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "GaussDBT resource not exist"));
        // 检查agent主机是否已经注册了GaussDBT（不包含自己的）
        Set<String> agentsUuids = getAgents(environment);
        // 移除自身包含的agents uuid
        Set<String> oldAgents = getSingleEnvironmentAgents(oldEnvironment);
        Set<String> newAgents = agentsUuids.stream()
            .filter(uuid -> !oldAgents.contains(uuid))
            .collect(Collectors.toSet());
        checkHostRegistered(newAgents, getGaussDBTEnvironments());
        // 连通性检测,检查集群信息是否正确
        ResourceCheckContext checkContext = checkClusterConnection(environment);
        AppEnvResponse appEnvResponse = getCheckResult(checkContext);
        checkClusterInfo(environment, appEnvResponse, agentsUuids);
        // 构建集群参数
        GaussDBTClusterUtil.setClusterInfo(environment, appEnvResponse, checkContext);
        log.info("preUpdate, build GaussDBT environment info success.uuid:{}", environment.getUuid());
    }

    /**
     * 获取前端传入的agents
     *
     * @param environment 注册的环境信息
     * @return agent的uuid集合
     */
    private Set<String> getAgents(ProtectedEnvironment environment) {
        return environment.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toSet());
    }

    /**
     * 查询数据库中GaussDBT资源信息
     *
     * @return ProtectedResource
     */
    private List<ProtectedEnvironment> getGaussDBTEnvironments() {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.GAUSSDBT.getType());
        List<ProtectedEnvironment> environments = new ArrayList<>();
        int num = GaussDBTConstant.GAUSSDBT_CLUSTER_MAX_COUNT / DatabaseConstants.PAGE_SIZE;
        for (int size = 0; size < num; size++) {
            List<ProtectedEnvironment> pageEnvironments = resourceService.query(0,
                DatabaseConstants.PAGE_SIZE, conditions)
                .getRecords()
                .stream()
                .flatMap(StreamUtil.match(ProtectedEnvironment.class))
                .collect(Collectors.toList());
            environments.addAll(pageEnvironments);
        }
        return environments;
    }

    /**
     * 检查agent主机是否已经注册了GaussDBT
     *
     * @param agentsUuids 前端传入的agent主机uuid
     * @param existEnvironments 已经存在的集群信息
     */
    private void checkHostRegistered(Set<String> agentsUuids, List<ProtectedEnvironment> existEnvironments) {
        // 获取已经注册的agent主机
        Set<String> alreadyRegisteredHost = existEnvironments.stream()
            .map(this::getSingleEnvironmentAgents)
            .flatMap(Set::stream)
            .collect(Collectors.toSet());

        // 如果有主机已经注册了GaussDBT资源就抛出异常
        List<String> conflicts = agentsUuids.stream()
            .filter(alreadyRegisteredHost::contains)
            .collect(Collectors.toList());
        if (!conflicts.isEmpty()) {
            log.error("agent host is already registered GaussDBT resource. uuids:{}", conflicts);
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODE_IS_REGISTERED,
                "agent host is already registered GaussDBT resource. uuids:" + conflicts);
        }
    }

    /**
     * 获取单个环境资源的agents
     *
     * @param environment 环境信息
     * @return agents
     */
    private Set<String> getSingleEnvironmentAgents(ProtectedEnvironment environment) {
        Map<String, String> extendInfo = Optional.ofNullable(environment.getExtendInfo()).orElse(new HashMap<>());
        String nodesString = extendInfo.get(GaussDBTConstant.NODES_KEY);
        List<NodeInfo> nodeInfoList = JsonUtil.read(nodesString, new TypeReference<List<NodeInfo>>() {
        });
        return nodeInfoList.stream().map(NodeInfo::getUuid).collect(Collectors.toSet());
    }

    /**
     * 检查uuid是否重复
     *
     * @param uuid 生成的唯一uuid
     */
    private void checkDuplicatedEnvironment(String uuid) {
        resourceService.getResourceById(uuid).ifPresent(env -> {
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODE_IS_REGISTERED,
                "The GaussDBT already exist.uuid:" + uuid);
        });
    }

    /**
     * 根据agent主机的uuid和资源类型生成环境的唯一UUID
     *
     * @param agentsUuids agent主机的uuid集合
     * @return 唯一UUID
     */
    private String buildUniqueUuid(Set<String> agentsUuids) {
        // 设置唯一UUID
        String uuidTag = agentsUuids.stream().sorted().collect(Collectors.joining(";"));
        String envIdentity = ResourceSubTypeEnum.GAUSSDBT.getType() + uuidTag;
        String uuid = UUID.nameUUIDFromBytes(envIdentity.getBytes(Charset.defaultCharset())).toString();
        log.info("start to register new GaussDBT environment, uuid: {}", uuid);
        return uuid;
    }

    private ResourceCheckContext checkClusterConnection(ProtectedEnvironment environment) {
        return providerManager.findProvider(ResourceConnectionCheckProvider.class, environment)
            .checkConnection(environment);
    }

    private void checkClusterInfo(ProtectedEnvironment environment, AppEnvResponse appEnvResponse, Set<String> uuids) {
        Map<String, String> appExtendInfo = Optional.ofNullable(appEnvResponse.getExtendInfo()).orElse(new HashMap<>());
        // 如果集群的部署状态和实际查询的不一致则抛出异常
        String deployType = environment.getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE);
        String realDeployType = appExtendInfo.get(DatabaseConstants.DEPLOY_TYPE);
        GaussDBTValidator.verifyDeployType(deployType, realDeployType);
        // 如果节点数量少于实际的节点数量不允许注册
        GaussDBTValidator.verityNodesCount(uuids.size(), appEnvResponse.getNodes().size());
    }

    /**
     * 检查联调性获取检查结果
     *
     * @param resourceCheckContext 资源检查上下文
     * @return AppEnvResponse
     */
    private AppEnvResponse getCheckResult(ResourceCheckContext resourceCheckContext) {
        // 查询集群节点失败，则更新集群状态和节点状态到离线，节点状态到异常
        Map<String, Object> context = Optional.ofNullable(resourceCheckContext.getContext()).orElse(new HashMap<>());
        Object clusterObject = context.get(GaussDBTConstant.CLUSTER_INFO_KEY);
        List<AppEnvResponse> appEnvResponses = JsonUtil.read(clusterObject, new TypeReference<List<AppEnvResponse>>() {
        });
        // 判断是不是同一个集群
        Set<String> ips = appEnvResponses.stream().map(AppEnvResponse::getNodes).flatMap(List::stream)
            .map(NodeInfo::getEndpoint).collect(Collectors.toSet());
        Set<String> singIps = appEnvResponses.get(0).getNodes().stream()
            .map(NodeInfo::getEndpoint).collect(Collectors.toSet());
        if (!singIps.containsAll(ips)) {
            log.error("The gaussdbt registered nodes are not in the same cluster.");
            throw new LegoCheckedException(CommonErrorCode.INSTANCES_NOT_BELONG_SAME_CLUSTER,
                "The gaussdbt registered nodes are not in the same cluster.");
        }
        return appEnvResponses.get(0);
    }

    /**
     * GaussDBT健康检查并更新状态和资源信息
     *
     * @param environment 资源
     */
    @Override
    public void validate(ProtectedEnvironment environment) {
        String uuid = environment.getUuid();
        Map<String, String> envExtendInfo = Optional.ofNullable(environment.getExtendInfo()).orElse(new HashMap<>());
        try {
            ResourceCheckContext checkContext = checkClusterConnection(environment);
            AppEnvResponse appEnvResponse = getCheckResult(checkContext);
            log.info("GaussDBT health check: cluster nodes query success, update cluster info.uuid: {}", uuid);
            // 只做增量更新
            ProtectedEnvironment newEnv = new ProtectedEnvironment();
            newEnv.setUuid(uuid);
            GaussDBTClusterUtil.setClusterInfo(newEnv, appEnvResponse, checkContext);
            resourceService.updateSourceDirectly(Collections.singletonList(newEnv));
        } catch (LegoCheckedException e) {
            // 异常情况下，将集群的节点状态强制置为离线
            log.error("GaussDBT health check: cluster nodes query failed, update cluster info.uuid: {}", uuid);
            checkConnectionFailedUpdate(environment, envExtendInfo);
        }
    }

    /**
     * 连接GaussDBT失败更新到离线状态
     *
     * @param environment 环境信息
     * @param envExtendInfo 环境扩展参数
     */
    private void checkConnectionFailedUpdate(ProtectedEnvironment environment, Map<String, String> envExtendInfo) {
        envExtendInfo.put(GaussDBTConstant.CLUSTER_STATE_KEY, GaussDBTClusterStateEnum.UNAVAILABLE.getState());
        String nodesString = envExtendInfo.get(GaussDBTConstant.NODES_KEY);
        List<NodeInfo> nodeInfoList = JsonUtil.read(nodesString, new TypeReference<List<NodeInfo>>() {
        });
        // 如果连接集群直接失败，则强制将环境中的节点置为离线
        List<NodeInfo> nodes = nodeInfoList.stream().peek(nodeInfo -> {
            HashMap<String, String> nodesExtendInfo = new HashMap<>();
            nodesExtendInfo.put(GaussDBTConstant.NODE_STATUS_KEY, GaussDBTNodeStatusEnum.OFFLINE.getStatus());
            nodeInfo.setExtendInfo(nodesExtendInfo);
        }).collect(Collectors.toList());
        envExtendInfo.put(GaussDBTConstant.NODES_KEY, JSONObject.writeValueAsString(nodes));
        // 框架只会做集群状态离线，所以手动更新节点状态到数据库中，只做增量更新
        ProtectedEnvironment newEnv = new ProtectedEnvironment();
        newEnv.setExtendInfo(envExtendInfo);
        newEnv.setUuid(environment.getUuid());
        resourceService.updateSourceDirectly(Collections.singletonList(newEnv));
        log.error("the gaussDBT cluster nodes query failed, update cluster and nodes status to offline.uuid:{}",
            environment.getUuid());
    }
}
