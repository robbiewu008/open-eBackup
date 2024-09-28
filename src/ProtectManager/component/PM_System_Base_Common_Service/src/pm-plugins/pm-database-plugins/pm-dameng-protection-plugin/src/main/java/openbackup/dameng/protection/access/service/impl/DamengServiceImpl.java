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
package openbackup.dameng.protection.access.service.impl;

import openbackup.dameng.protection.access.constant.DamengConstant;
import openbackup.dameng.protection.access.service.DamengService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.utils.AgentDtoUtil;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;
import org.springframework.util.CollectionUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.RecursiveTask;
import java.util.stream.Collectors;

/**
 * dameng集群查询、校验的类
 *
 */
@Slf4j
@Service
public class DamengServiceImpl implements DamengService {
    private static final Integer THREAD_POOL_NUM = 5;

    private final ResourceService resourceService;

    private final AgentUnifiedService agentUnifiedService;

    private final CopyRestApi copyRestApi;

    /**
     * 构造方法
     *
     * @param resourceService 资源服务接口
     * @param agentUnifiedService agent接口实现
     * @param copyRestApi 副本查询接口
     */
    public DamengServiceImpl(ResourceService resourceService, AgentUnifiedService agentUnifiedService,
        CopyRestApi copyRestApi) {
        this.resourceService = resourceService;
        this.agentUnifiedService = agentUnifiedService;
        this.copyRestApi = copyRestApi;
    }

    /**
     * 查询并校验集群实例
     *
     * @param resource 资源
     * @return 集群查询结果
     */
    @Override
    public List<AppEnvResponse> check(ProtectedResource resource) {
        List<ProtectedResource> instances = resource.getDependencies().get(DatabaseConstants.CHILDREN);
        List<AppEnvResponse> appEnvResponseList = queryClusterInfo(instances);
        // 校验集群实例的一致性
        checkClusterConsistence(appEnvResponseList, instances);
        return appEnvResponseList;
    }

    @Override
    public List<AppEnvResponse> queryClusterInfo(List<ProtectedResource> instances) {
        updateAuth(instances);
        int taskNum = Math.min(THREAD_POOL_NUM, instances.size());
        ForkJoinPool forkJoinPool = new ForkJoinPool(taskNum);
        List<AppEnvResponse> appEnvResponseList = new ArrayList<>();
        try {
            appEnvResponseList = forkJoinPool.submit(new RecursiveTask<List<AppEnvResponse>>() {
                @Override
                protected List<AppEnvResponse> compute() {
                    return getClusterInfoFromAgent(instances);
                }
            }).get();
        } catch (InterruptedException | ExecutionException e) {
            log.error("Dameng query cluster info thread exception: ", e);
        }
        return appEnvResponseList;
    }

    private void updateAuth(List<ProtectedResource> instances) {
        instances.forEach(instance -> {
            if (instance.getAuth().getAuthType() != 1 && VerifyUtil.isEmpty(instance.getAuth().getAuthPwd())) {
                ProtectedResource oldInstance = resourceService.getResourceById(instance.getUuid())
                    .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));
                instance.getAuth().setAuthPwd(oldInstance.getAuth().getAuthPwd());
            }
        });
    }

    private List<AppEnvResponse> getClusterInfoFromAgent(List<ProtectedResource> instances) {
        return instances.parallelStream().map(instance -> {
            ProtectedEnvironment environment = queryAgentEnvironment(instance);
            return agentUnifiedService.getClusterInfoNoRetry(instance, environment);
        }).collect(Collectors.toList());
    }

    /**
     * 从单实例的dependency里，获取对应的Agent主机
     *
     * @param instance 单实例
     * @return Agent主机信息
     */
    @Override
    public ProtectedEnvironment queryAgentEnvironment(ProtectedResource instance) {
        List<ProtectedResource> agentResources = instance.getDependencies().get(DatabaseConstants.AGENTS);
        if (VerifyUtil.isEmpty(agentResources)) {
            log.error("Single instance dependency agent is empty.");
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Single instance dependency agent is empty.");
        }
        String agentUuid = agentResources.get(0).getUuid();
        return getEnvironmentById(agentUuid);
    }

    @Override
    public ProtectedEnvironment getEnvironmentById(String envId) {
        Optional<ProtectedResource> resOptional = resourceService.getResourceById(envId);
        if (resOptional.isPresent() && resOptional.get() instanceof ProtectedEnvironment) {
            return (ProtectedEnvironment) resOptional.get();
        }
        throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Protected environment is not exists!");
    }

    private void checkClusterConsistence(List<AppEnvResponse> appEnvResponseList, List<ProtectedResource> instances) {
        // 检查集群查询的返回结果是否为空
        checkClusterResponse(appEnvResponseList);

        List<AppEnvResponse> existingResponseList = appEnvResponseList.stream()
            .filter(response -> response.getNodes() != null)
            .collect(Collectors.toList());
        // 检查注册的实例是否属于同一个集群
        checkIsSameCluster(existingResponseList);
        // 检查注册的实例是否包含所有节点
        checkIsIncludeAllNodes(existingResponseList, instances);
        existingResponseList.forEach(response -> {
            // 检查节点的实例状态，节点离线不允许注册
            checkInstanceStatus(response);
        });
    }

    private void checkClusterResponse(List<AppEnvResponse> appEnvResponseList) {
        if (VerifyUtil.isEmpty(appEnvResponseList)) {
            log.error("The Dameng cluster query result is empty.");
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                "The Dameng cluster query result is empty.");
        }
        boolean isEmptyNodes = appEnvResponseList.stream()
            .anyMatch(response -> VerifyUtil.isEmpty(response.getNodes()));
        if (isEmptyNodes) {
            log.error("The Dameng cluster query nodes is empty.");
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                "The Dameng cluster query nodes is empty.");
        }
    }

    private void checkIsSameCluster(List<AppEnvResponse> existingResponseList) {
        Set<String> firstIpAndPortSet = getIpAndPortSet(existingResponseList, 0);
        for (int i = 1; i < existingResponseList.size(); i++) {
            Set<String> otherIpAndPortSet = getIpAndPortSet(existingResponseList, i);
            if (!otherIpAndPortSet.containsAll(firstIpAndPortSet)) {
                log.error("The Dameng registered instances are not in the same cluster.");
                throw new LegoCheckedException(CommonErrorCode.INSTANCES_NOT_BELONG_SAME_CLUSTER,
                    "The Dameng registered instances are not in the same cluster.");
            }
        }
    }

    private void checkIsIncludeAllNodes(List<AppEnvResponse> existingResponseList, List<ProtectedResource> instances) {
        boolean isIncludeAllNodes = existingResponseList.stream()
            .anyMatch(response -> instances.size() < response.getNodes().size());
        if (isIncludeAllNodes) {
            log.error("The Dameng registered instances are not included all node.");
            throw new LegoCheckedException(CommonErrorCode.NOT_INCLUDE_ALL_CLUSTER_INSTANCES,
                "The Dameng registered instances are not included all node.");
        }
    }

    private Set<String> getIpAndPortSet(List<AppEnvResponse> existingResponseList, int index) {
        return existingResponseList.get(index)
            .getNodes()
            .stream()
            .map(node -> connectIpAndPort(node.getEndpoint(), node.getExtendInfo().get(DatabaseConstants.PORT)))
            .collect(Collectors.toSet());
    }

    private void checkInstanceStatus(AppEnvResponse response) {
        List<String> offLineNodes = response.getNodes()
            .stream()
            .filter(nodeInfo -> isInstanceOffline(nodeInfo))
            .map(NodeInfo::getUuid)
            .collect(Collectors.toList());
        if (!VerifyUtil.isEmpty(offLineNodes)) {
            log.error("The Dameng nodes:{} is offline.", offLineNodes);
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "The Dameng nodes is offline.");
        }
    }

    private boolean isInstanceOffline(NodeInfo nodeInfo) {
        String instanceStatus = nodeInfo.getExtendInfo().get(DamengConstant.INSTANCESTATUS);
        return instanceStatus != null && Objects.equals(instanceStatus, LinkStatusEnum.OFFLINE.getStatus().toString());
    }

    private String connectIpAndPort(String ip, String port) {
        return ip + DatabaseConstants.IP_PORT_SPLIT_CHAR + port;
    }

    /**
     * 获取已经注册的主机uuid和实例端口集合
     *
     * @param environment 注册的单机或集群环境
     * @return 主机uuid和实例端口集合
     */
    @Override
    public Set<String> getExistingUuidAndPort(ProtectedEnvironment environment) {
        // 获取已经存在的dameng环境信息
        List<ProtectedEnvironment> existingEnv = new ArrayList<>();
        existingEnv.addAll(getExistingEnvironments(ResourceSubTypeEnum.DAMENG_CLUSTER.getType()));
        existingEnv.addAll(getExistingEnvironments(ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType()));

        if (!VerifyUtil.isEmpty(environment.getUuid())) {
            existingEnv.removeIf(env -> Objects.equals(environment.getUuid(), env.getUuid()));
        }
        return existingEnv.stream()
            .map(env -> getNodeInfoFromNodes(env).stream()
                .map(nodeInfo -> connectUuidAndPort(nodeInfo.getUuid(),
                    nodeInfo.getExtendInfo().get(DatabaseConstants.PORT)))
                .collect(Collectors.toSet()))
            .flatMap(Set::stream)
            .collect(Collectors.toSet());
    }

    /**
     * 将nodes转换为NodeInfo
     *
     * @param environment 环境信息
     * @return 节点信息列表
     */
    @Override
    public List<NodeInfo> getNodeInfoFromNodes(ProtectedEnvironment environment) {
        JSONArray jsonArray = JSONArray.fromObject(environment.getExtendInfo().get(DamengConstant.NODES));
        return JSONArray.toCollection(jsonArray, NodeInfo.class);
    }

    /**
     * 拼接主机uuid和实例端口
     *
     * @param uuid 主机uuid
     * @param port 实例端口
     * @return 拼接后的uuid和实例端口
     */
    @Override
    public String connectUuidAndPort(String uuid, String port) {
        return uuid + DamengConstant.UUID_INSTANCE_PORT_SPLIT_CHAR + port;
    }

    private List<ProtectedEnvironment> getExistingEnvironments(String subType) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("type", ResourceTypeEnum.DATABASE.getType());
        conditions.put("subType", subType);
        List<ProtectedResource> existingResources = resourceService.query(0, DamengConstant.DAMENG_CLUSTER_MAX_COUNT,
            conditions).getRecords();

        // 转成ProtectedEnvironment环境对象
        return existingResources.stream()
            .filter(existingResource -> existingResource instanceof ProtectedEnvironment)
            .map(existingResource -> (ProtectedEnvironment) existingResource)
            .collect(Collectors.toList());
    }

    /**
     * 针对集群实例，将子实例信息中的auth,实例端口和主备信息设置到nodes中
     *
     * @param uuid 资源uuid
     * @return nodes列表
     */
    @Override
    public List<TaskEnvironment> buildTaskNodes(String uuid) {
        List<ProtectedResource> subInstances = getSubInstanceByClusterInstance(uuid);
        HashMap<String, TaskEnvironment> nodeMap = new HashMap<>();
        for (ProtectedResource subInstance : subInstances) {
            ProtectedEnvironment agentEnv = queryAgentEnvironment(subInstance);
            List<NodeInfo> nodeInfoList = getNodeInfoList(subInstance, agentEnv.getUuid());
            nodeInfoList.stream().forEach(nodeInfo -> {
                String instancePort = nodeInfo.getExtendInfo().get(DatabaseConstants.PORT);

                HostDto hostDto = agentUnifiedService.getHost(agentEnv.getEndpoint(), agentEnv.getPort());
                TaskEnvironment nodeEnv = AgentDtoUtil.toTaskEnvironment(hostDto);
                // Authentication需要深拷贝，防止多节点场景重复加密
                nodeEnv.setAuth(JSONObject.fromObject(subInstance.getAuth()).toBean(Authentication.class));
                Map<String, String> extendInfo = new HashMap<>();
                extendInfo.put(DamengConstant.ROLE, nodeInfo.getExtendInfo().get(DamengConstant.ROLE));
                extendInfo.put(DatabaseConstants.PORT, instancePort);
                extendInfo.put(DamengConstant.DB_PATH, nodeInfo.getExtendInfo().get(DamengConstant.DB_PATH));
                extendInfo.put(DamengConstant.DB_NAME, nodeInfo.getExtendInfo().get(DamengConstant.DB_NAME));
                extendInfo.put(DamengConstant.GROUP_ID, nodeInfo.getExtendInfo().get(DamengConstant.GROUP_ID));
                extendInfo.put(DatabaseConstants.END_POINT, nodeInfo.getExtendInfo().get(DatabaseConstants.END_POINT));
                extendInfo.put(DamengConstant.DM_INI_PATH, nodeInfo.getExtendInfo().get(DamengConstant.DM_INI_PATH));
                nodeEnv.setExtendInfo(extendInfo);

                String uuidAndInstancePort = connectUuidAndPort(agentEnv.getUuid(), instancePort);
                nodeMap.put(uuidAndInstancePort, nodeEnv);
            });
        }
        return nodeMap.values().stream().collect(Collectors.toList());
    }

    /**
     * 针对单机，添加nodes
     *
     * @param agents agents
     * @return nodes列表
     */
    @Override
    public List<TaskEnvironment> buildTaskHosts(List<Endpoint> agents) {
        if (CollectionUtils.isEmpty(agents)) {
            return Collections.emptyList();
        }
        return agents.stream()
            .map(agent -> agentUnifiedService.getHost(agent.getIp(), agent.getPort()))
            .map(AgentDtoUtil::toTaskEnvironment)
            .collect(Collectors.toList());
    }

    private List<NodeInfo> getNodeInfoList(ProtectedResource subInstance, String agentUuid) {
        List<NodeInfo> nodeInfoList = getNodeInfoFromNodes(subInstance.getEnvironment());
        return nodeInfoList.stream()
            .filter(node -> Objects.equals(node.getUuid(), agentUuid))
            .collect(Collectors.toList());
    }

    /**
     * 从集群实例中获取子实例
     *
     * @param clusterInstanceUuid 集群实例uuid
     * @return 子实例列表
     */
    private List<ProtectedResource> getSubInstanceByClusterInstance(String clusterInstanceUuid) {
        ProtectedResource clusterInstanceResource = resourceService.getResourceById(clusterInstanceUuid)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));
        Map<String, List<ProtectedResource>> dependencies = clusterInstanceResource.getDependencies();
        return dependencies.get(DatabaseConstants.CHILDREN);
    }

    /**
     * 根据Agent主机信息，获取Agent主机的Endpoint
     *
     * @param environment Agent环境信息
     * @return Agent对应的Endpoint信息
     */
    @Override
    public Endpoint getAgentEndpoint(ProtectedEnvironment environment) {
        if (VerifyUtil.isEmpty(environment.getUuid()) || VerifyUtil.isEmpty(environment.getEndpoint())
            || VerifyUtil.isEmpty(environment.getPort())) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "dameng agent environment lack require msg.");
        }
        return new Endpoint(environment.getUuid(), environment.getEndpoint(), environment.getPort());
    }

    /**
     * 恢复时校验数据库版本是否一致
     *
     * @param task 恢复任务
     */
    @Override
    public void checkDbVersion(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        JSONObject resourceJson = JSONObject.fromObject(copy.getResourceProperties());
        Map<String, String> extendInfo = resourceJson.getJSONObject(DamengConstant.EXTEND_INFO).toMap(String.class);
        String targetVersion = extendInfo.get(DamengConstant.VERSION);
        ProtectedEnvironment environment = getEnvironmentById(task.getTargetEnv().getUuid());
        String currentVersion = environment.getExtendInfo().get(DamengConstant.VERSION);
        if (!Objects.equals(targetVersion, currentVersion)) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                "The Dameng target version " + targetVersion + " is different from the current version "
                    + currentVersion);
        }
    }

    /**
     * 设置恢复模式
     *
     * @param task 恢复任务
     */
    @Override
    public void setRestoreMode(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        String generatedBy = copy.getGeneratedBy();
        if (CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value().equals(generatedBy)
            || CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value().equals(generatedBy)) {
            task.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
        } else {
            task.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
        }
        log.info("dameng restore target uuid: {}. copy id: {}, mode: {}", task.getTargetObject().getUuid(),
            task.getCopyId(), task.getRestoreMode());
    }

    /**
     * 获取agent列表
     *
     * @param uuid 环境的uuid
     * @return agent列表
     */
    @Override
    public List<Endpoint> getEndpointList(String uuid) {
        List<Endpoint> endpointList = new ArrayList<>();
        List<ProtectedResource> subInstances = getSubInstanceByClusterInstance(uuid);
        // 遍历子实例信息
        for (ProtectedResource subInstance : subInstances) {
            // 从子实例的dependency里，获取子实例对应的Agent主机
            ProtectedEnvironment agentEnv = queryAgentEnvironment(subInstance);
            // 获取agent的endpoint信息
            Endpoint endpoint = getAgentEndpoint(agentEnv);
            boolean isEndpointExist = endpointList.stream()
                .anyMatch(subEndpoint -> subEndpoint.getId().equals(endpoint.getId()));
            if (!isEndpointExist) {
                endpointList.add(endpoint);
            }
        }
        return endpointList;
    }

    /**
     * 设置恢复的高级参数
     *
     * @param task 恢复任务
     */
    @Override
    public void setRestoreAdvanceParams(RestoreTask task) {
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElse(new HashMap<>());
        // 框架不会带targetLocation参数到dme,回填targetLocation高级参数中下发到插件
        advanceParams.put(DamengConstant.TARGET_LOCATION_KEY, task.getTargetLocation().getLocation());
        task.setAdvanceParams(advanceParams);
    }
}
