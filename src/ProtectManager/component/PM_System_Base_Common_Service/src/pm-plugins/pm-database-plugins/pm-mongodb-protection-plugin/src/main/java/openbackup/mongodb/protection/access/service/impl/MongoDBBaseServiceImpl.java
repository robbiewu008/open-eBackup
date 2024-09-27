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
package openbackup.mongodb.protection.access.service.impl;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.mongodb.protection.access.bo.MongoClusterNodesExtendInfo;
import openbackup.mongodb.protection.access.constants.MongoDBConstants;
import openbackup.mongodb.protection.access.enums.MongoDBClusterRoleEnum;
import openbackup.mongodb.protection.access.enums.MongoDBNodeTypeEnum;
import openbackup.mongodb.protection.access.service.MongoDBBaseService;
import openbackup.mongodb.protection.access.util.MongoDBConstructionUtils;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.ObjectUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * mongodb 实际业务service
 *
 * @author lwx1012372
 * @version [DataBackup 1.5.0]
 * @since 2023-04-07
 */
@Component
@Slf4j
public class MongoDBBaseServiceImpl implements MongoDBBaseService {
    private final ResourceService resourceService;

    private final AgentUnifiedService agentUnifiedService;

    private final ProtectedEnvironmentService protectedEnvironmentService;

    /**
     * 构造器
     *
     * @param resourceService resourceService
     * @param agentUnifiedService agentUnifiedService
     * @param protectedEnvironmentService protectedEnvironmentService
     */
    public MongoDBBaseServiceImpl(ResourceService resourceService, AgentUnifiedService agentUnifiedService,
        ProtectedEnvironmentService protectedEnvironmentService) {
        this.resourceService = resourceService;
        this.agentUnifiedService = agentUnifiedService;
        this.protectedEnvironmentService = protectedEnvironmentService;
    }

    /**
     * 检查实例节点是否存在情况
     *
     * @param environment 单实例资源信息
     * @param isRegistered 是否注册过
     */
    @Override
    public void checkMongoDBEnvironmentSize(ProtectedResource environment, boolean isRegistered) {
        int size = isRegistered ? IsmNumberConstant.ZERO : IsmNumberConstant.ONE;
        long commonErrorCode = isRegistered
            ? CommonErrorCode.INSTANCE_REGISTED_ERROR
            : CommonErrorCode.ERR_PARAM;
        String ip = environment.getExtendInfo().get(MongoDBConstants.SERVICE_IP);
        String port = environment.getExtendInfo().get(MongoDBConstants.SERVICE_PORT);
        String url = ip + ":" + port;
        int instanceCount = queryMongoDBInstanceNums(environment);
        log.info("This MongoDB instance url:{} instanceCount: {}, size: {}", url, instanceCount, size);
        if (instanceCount != size) {
            throw new LegoCheckedException(commonErrorCode, new String[] {url},
                "This MongoDB instance is exist size:" + size);
        }
    }

    /**
     * 构造实例入参的MongoClusterNodesExtendInfo对象
     *
     * @param protectedResource 查询对象的扩展参数
     * @param protectedEnvironment 节点参数
     * @return AppEnvResponse
     */
    @Override
    public AppEnvResponse getAppEnvAgentInfo(ProtectedResource protectedResource,
        ProtectedEnvironment protectedEnvironment) {
        return agentUnifiedService.getClusterInfoNoRetry(protectedResource, protectedEnvironment);
    }

    /**
     * 获取uuid的环境信息
     *
     * @param envId uuid
     * @return 环境信息
     */
    @Override
    public ProtectedEnvironment getEnvironmentById(String envId) {
        return protectedEnvironmentService.getEnvironmentById(envId);
    }

    /**
     * 获取uuid的资源信息
     *
     * @param envId uuid
     * @return 资源信息
     */
    @Override
    public ProtectedResource getResource(String envId) {
        Optional<ProtectedResource> resOptional = resourceService.getResourceById(envId);
        return resOptional.orElseThrow(
            () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Protected environment is not exists!"));
    }

    /**
     * 根据IP和PORT查询到单实例
     *
     * @param environment 查询资源env
     * @return 返回实例的个数
     */
    private int queryMongoDBInstanceNums(ProtectedResource environment) {
        HashMap<String, Object> filter = new HashMap<>();
        filter.put(DatabaseConstants.RESOURCE_TYPE, environment.getType());
        filter.put(DatabaseConstants.SUB_TYPE, environment.getSubType());
        filter.put(MongoDBConstants.SERVICE_IP, environment.getExtendInfo().get(MongoDBConstants.SERVICE_IP));
        filter.put(MongoDBConstants.SERVICE_PORT, environment.getExtendInfo().get(MongoDBConstants.SERVICE_PORT));
        int instanceCount = resourceService.query(IsmNumberConstant.ZERO, IsmNumberConstant.ONE, filter)
            .getTotalCount();
        log.info("start queryMongoDBInstance nums is: {}", instanceCount);
        return instanceCount;
    }

    /**
     * 更新检查信息
     *
     * @param protectedEnvironment 更新检查信息
     */
    @Override
    public void updateResourceService(ProtectedEnvironment protectedEnvironment) {
        resourceService.updateSourceDirectly(Collections.singletonList(protectedEnvironment));
    }

    @Override
    public void updateEnvironmentExtendInfoClusterNodes(List<Map<String, String>> appEnvExtendInfo,
        List<NodeInfo> clusterNodesCollect, ProtectedEnvironment environment) {
        List<MongoClusterNodesExtendInfo> oldNodesExtendInfos = com.alibaba.fastjson.JSONArray.parseArray(
            environment.getExtendInfo().get(MongoDBConstants.CLUSTER_NODES), MongoClusterNodesExtendInfo.class);
        checkAndReturn(appEnvExtendInfo, clusterNodesCollect, oldNodesExtendInfos);
        environment.getExtendInfo()
            .put(MongoDBConstants.CLUSTER_NODES, JSONArray.fromObject(oldNodesExtendInfos).toString());
    }

    private void checkAndReturn(List<Map<String, String>> appEnvExtendInfo, List<NodeInfo> clusterNodesCollect,
        List<MongoClusterNodesExtendInfo> oldNodesExtendInfos) {
        Map<String, MongoClusterNodesExtendInfo> oldNodesExtendInfosMap = new HashMap<>();
        for (MongoClusterNodesExtendInfo oldNodesExtendInfo : oldNodesExtendInfos) {
            if (oldNodesExtendInfo.getShardClusterType().equals(MongoDBNodeTypeEnum.MONGOS.getType())) {
                oldNodesExtendInfo.setNodeStatus(String.valueOf(LinkStatusEnum.OFFLINE.getStatus()));
            }
            oldNodesExtendInfosMap.put(oldNodesExtendInfo.getHostUrl(), oldNodesExtendInfo);
        }

        // 先更新 extendInfo;
        for (Map<String, String> extendInfo : appEnvExtendInfo) {
            String localHost = extendInfo.get(MongoDBConstants.LOCAL_HOST);
            if (StringUtils.isEmpty(localHost)) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "no find local host");
            }
            MongoClusterNodesExtendInfo nodesExtendInfo = oldNodesExtendInfosMap.get(localHost);
            if (ObjectUtils.isEmpty(nodesExtendInfo)) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "no find node info");
            }
            MongoDBConstructionUtils.setMongoClusterNodeForEnvExtendInfo(extendInfo, nodesExtendInfo);
        }
        // 在更新 nodeInfo
        for (NodeInfo nodeInfo : clusterNodesCollect) {
            String localHost = nodeInfo.getName();
            if (StringUtils.isEmpty(localHost)) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "no find local host");
            }
            MongoClusterNodesExtendInfo nodesExtendInfo = oldNodesExtendInfosMap.get(localHost);
            if (ObjectUtils.isEmpty(nodesExtendInfo)) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "no find node info");
            }
            MongoDBConstructionUtils.setMongoClusterNodeForNodeInfo(nodeInfo, nodesExtendInfo);
        }
    }

    /**
     * 受保护环境对象内的资源Agent类表
     *
     * @param uuid 受保护环境uuid
     * @return 集群环境对应的节点信息列表
     */
    @Override
    public List<TaskEnvironment> buildBackupTaskNodes(String uuid) {
        List<ProtectedResource> subInstances = getResource(uuid).getDependencies().get(DatabaseConstants.CHILDREN);
        return subInstances.stream().map(subInstance -> {
            subInstance.getExtendInfo()
                .putAll(subInstance.getDependencies()
                    .get(DatabaseConstants.AGENTS)
                    .get(IsmNumberConstant.ZERO)
                    .getExtendInfo());
            subInstance.getExtendInfo().put(MongoDBConstants.AGENT_UUID, subInstance.getDependencies()
                .get(DatabaseConstants.AGENTS)
                .get(IsmNumberConstant.ZERO).getUuid());
            return BeanTools.copy(subInstance, TaskEnvironment::new);
        }).collect(Collectors.toList());
    }

    /**
     * 根据资源id获取当前锁情况
     *
     * @param envId 资源id
     * @return 锁情况
     */
    @Override
    public List<LockResourceBo> getRestoreLockResource(String envId) {
        Set<String> lockResourceIds = resourceService.queryRelatedResourceUuids(envId, new String[0]);
        lockResourceIds.add(envId);
        return lockResourceIds.stream()
            .map(uuid -> new LockResourceBo(uuid, LockType.WRITE))
            .collect(Collectors.toList());
    }

    /**
     * 检查agent是否在线
     *
     * @param environment 入库资源
     */
    @Override
    public void checkAgentIsOnline(ProtectedEnvironment environment) {
        List<ProtectedResource> protectedResources = environment.getDependencies().get(DatabaseConstants.CHILDREN);
        long count = protectedResources.stream()
            .map(dependencyResource -> dependencyResource.getExtendInfo().get(MongoDBConstants.AGENT_UUID))
            .distinct()
            .map(this::getEnvironmentById)
            .filter(protectedEnvironment -> !String.valueOf(LinkStatusEnum.ONLINE.getStatus())
                .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(protectedEnvironment)))
            .count();
        if (count != 0) {
            log.error("Exist agent not online, count: {}", count);
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Exist agent not online");
        }
    }

    /**
     * 判断主节点是否满足要求
     *
     * @param count 原主节点个数
     * @param clusterNodesCollect 查询的主节点个数
     */
    public void checkPrimarySizeIsMeet(String count, List<NodeInfo> clusterNodesCollect) {
        List<Map<String, String>> primaryClusterList = clusterNodesCollect.stream()
            .map(NodeInfo::getExtendInfo)
            .filter(nodeExtendInfo -> MongoDBClusterRoleEnum.PRIMARY.getRole()
                .equals(nodeExtendInfo.get(MongoDBConstants.STATE_STR)))
            .collect(Collectors.toList());
        if (StringUtils.isEmpty(count) || !count.equals(String.valueOf(primaryClusterList.size()))) {
            log.error("mongodb need primary, count: {}, find count: {}", count, primaryClusterList.size());
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR,
                "mongodb need primary, count: " + count);
        }
    }

    /**
     * 检查分片集群校验
     *
     * @param appEnvResponseList 查询到分片集群信息
     */
    @Override
    public void checkShardCLuster(List<AppEnvResponse> appEnvResponseList) {
        List<Map<String, String>> appEnvExtendInfo = getAppEnvExtendInfo(appEnvResponseList);
        if (MongoDBNodeTypeEnum.REPLICATION.getType()
            .equals(appEnvResponseList.get(0).getExtendInfo().get(MongoDBConstants.SHARD_CLUSTER_TYPE))) {
            log.info("Cluster is replication cluster");
            return;
        }
        List<String> collect = appEnvExtendInfo.stream()
            .map(extendInfo -> extendInfo.get(MongoDBConstants.SHARD_CLUSTER_TYPE))
            .distinct()
            .collect(Collectors.toList());
        if (collect.size() > 0 && collect.contains(MongoDBNodeTypeEnum.MONGOS.getType()) && collect.contains(
            MongoDBNodeTypeEnum.CONFIG.getType()) && collect.contains(MongoDBNodeTypeEnum.SHARD.getType())) {
            return;
        }
        throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Exist agent not online");
    }

    private List<Map<String, String>> getAppEnvExtendInfo(List<AppEnvResponse> appEnvResponseList) {
        return appEnvResponseList.stream().map(AppEnvResponse::getExtendInfo).collect(Collectors.toList());
    }

    /**
     * 检查复制集群校验
     *
     * @param protectedResources 资源信息
     * @param appEnvResponseList 查询到复制集群信息
     */
    @Override
    public void checkReplicationCluster(List<ProtectedResource> protectedResources,
        List<AppEnvResponse> appEnvResponseList) {
        if (!MongoDBNodeTypeEnum.REPLICATION.getType()
            .equals(appEnvResponseList.get(0).getExtendInfo().get(MongoDBConstants.SHARD_CLUSTER_TYPE))) {
            log.info("Cluster is sharding cluster");
            return;
        }
        List<Map<String, String>> onlineNodes = getNodeInfos(appEnvResponseList).stream()
            .map(NodeInfo::getExtendInfo)
            .collect(Collectors.toList());
        long nodeCount = onlineNodes.stream()
            .filter(nodeMap -> MongoDBClusterRoleEnum.PRIMARY.getRole().equals(nodeMap.get(MongoDBConstants.STATE_STR)))
            .count();
        if (nodeCount == 0) {
            log.error("Exist agent not online, nodeCount: {}", nodeCount);
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Exist agent not online");
        }
        if (onlineNodes.size() * 2 < protectedResources.size()) {
            log.error("Exist agent not online, onlineNodes size: {}, resource size: {}", onlineNodes.size(),
                protectedResources.size());
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Exist agent not online");
        }
    }

    private List<NodeInfo> getNodeInfos(List<AppEnvResponse> appEnvResponseList) {
        List<String> nameList = new ArrayList<>();
        return appEnvResponseList.stream()
            .filter(appEnvResponse -> Objects.equals(appEnvResponse.getExtendInfo().get(MongoDBConstants.EXIST_NODES),
                String.valueOf(IsmNumberConstant.ONE)))
            .map(AppEnvResponse::getNodes)
            .flatMap(Collection::stream)
            .filter(nodeInfo -> checkNodeInfo(nodeInfo, nameList))
            .collect(Collectors.toList());
    }

    private boolean checkNodeInfo(NodeInfo nodeInfo, List<String> nameList) {
        if (StringUtils.isEmpty(nodeInfo.getName())) {
            throw new LegoCheckedException(CommonErrorCode.HOST_OFFLINE, new String[] {},
                "mongoDB cluster nodeInfo is empty!");
        }
        if (nameList.contains(nodeInfo.getName())) {
            return false;
        }
        nameList.add(nodeInfo.getName());
        return true;
    }

    /**
     * 校验auth认证的长度
     *
     * @param username 用户名
     * @param password 密码
     */
    @Override
    public void checkKeyLength(String username, String password) {
        if (StringUtils.isNotBlank(username) && username.length() > MongoDBConstants.AUTH_KEY_LENGTH) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "This MongoDB auth key length > 32");
        }
        if (StringUtils.isNotBlank(password) && password.length() > MongoDBConstants.AUTH_KEY_LENGTH) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "This MongoDB auth pwd length > 32");
        }
    }

    /**
     * 获取注册资源列表
     *
     * @param environment 注册资源
     * @return 注册资源列表
     */
    @Override
    public List<String> getAllIpAndPortList(ProtectedEnvironment environment) {
        List<Map<String, String>> infos = environment.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .map(protectedResourceData -> Optional.of(protectedResourceData.getExtendInfo())
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "do not extendInfo")))
            .distinct()
            .collect(Collectors.toList());
        return infos.stream().map(this::getExtendInfo).distinct().collect(Collectors.toList());
    }

    private String getExtendInfo(Map<String, String> extendInfos) {
        return Optional.of(extendInfos.get(MongoDBConstants.SERVICE_IP))
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "do not serverIp")) + ":"
            + Optional.of(extendInfos.get(MongoDBConstants.SERVICE_PORT))
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "do not serverPort"));
    }

    /**
     * 获取集群信息
     *
     * @param protectedResource 资源信息
     * @param urlList url列表
     * @param isRegisterCheck 是否为注册
     * @return 查询到集群信息列表
     */
    @Override
    public List<AppEnvResponse> getAppEnvResponses(ProtectedResource protectedResource, List<String> urlList,
        boolean isRegisterCheck) {
        List<ProtectedResource> resources = protectedResource.getDependencies().get(DatabaseConstants.CHILDREN);
        for (ProtectedResource resource : resources) {
            ProtectedEnvironment hostEnv = getEnvironmentById(
                resource.getExtendInfo().get(MongoDBConstants.AGENT_UUID));
            if ("windows".equals(hostEnv.getOsType())) {
                throw new LegoCheckedException(DatabaseErrorCode.ENV_OS_TYPE_ERROR,
                    new String[] {hostEnv.getEndpoint(), hostEnv.getOsType()}, "This agent do not need osType");
            }
        }
        if (resources.size() < 2) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "This MongoDB instance size < 2");
        }
        resources.forEach(instance -> {
            if (instance.getAuth().getAuthType() == Authentication.APP_PASSWORD && VerifyUtil.isEmpty(
                instance.getAuth().getAuthPwd())) {
                ProtectedResource resource = getResource(instance.getUuid());
                instance.getAuth().setAuthPwd(resource.getAuth().getAuthPwd());
            }
        });
        String clusterType = protectedResource.getExtendInfoByKey(MongoDBConstants.CLUSTE_TYPE);
        return resources.stream()
            .map(dependencyResource -> getAppEnvResponse(dependencyResource, protectedResource, urlList, clusterType,
                isRegisterCheck))
            .filter(appEnvResponse -> !MongoDBConstants.FAILED_MARK.equals(appEnvResponse.getName()))
            .collect(Collectors.toList());
    }

    private AppEnvResponse getAppEnvResponse(ProtectedResource dependencyResource, ProtectedResource protectedResource,
        List<String> urlList, String clusterType, boolean isRegisterCheck) {
        ProtectedEnvironment env = getEnvironmentById(
            dependencyResource.getExtendInfo().get(MongoDBConstants.AGENT_UUID));
        protectedResource.setEnvironment(env);
        // 从前端参数拿到用户名密码和extendInfo里面的信息
        protectedResource.setAuth(dependencyResource.getAuth());
        Map<String, String> map = new HashMap<>(dependencyResource.getExtendInfo());
        map.put(MongoDBConstants.ENDPOINT_LIST, JSONArray.fromObject(urlList).toString());
        map.put(MongoDBConstants.AGENT_CLUSTE_TYPE, clusterType);
        protectedResource.setExtendInfo(map);
        AppEnvResponse appEnvAgentInfo = getAppEnvAgentInfo(protectedResource, env);

        if (MongoDBConstants.FAILED_MARK.equals(appEnvAgentInfo.getName()) && isRegisterCheck) {
            if (StringUtils.isEmpty(appEnvAgentInfo.getExtendInfo().get(MongoDBConstants.ERROR_CODE))) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "This MongoDB instance connect agent failed");
            }
            String errorCode = appEnvAgentInfo.getExtendInfo().get(MongoDBConstants.ERROR_CODE);
            String errorParam = appEnvAgentInfo.getExtendInfo().get(MongoDBConstants.ERROR_PARAM);
            throw new LegoCheckedException(Long.parseLong(errorCode), errorParam.split(","),
                "This MongoDB instance connect agent failed");
        }
        return appEnvAgentInfo;
    }
}
