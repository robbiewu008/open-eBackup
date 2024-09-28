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
package openbackup.mongodb.protection.access.provider.resource;

import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.mongodb.protection.access.bo.MongoClusterNodesExtendInfo;
import openbackup.mongodb.protection.access.constants.MongoDBConstants;
import openbackup.mongodb.protection.access.enums.MongoDBClusterRoleEnum;
import openbackup.mongodb.protection.access.enums.MongoDBClusterTypeEnum;
import openbackup.mongodb.protection.access.enums.MongoDBNodeTypeEnum;
import openbackup.mongodb.protection.access.service.MongoDBBaseService;
import openbackup.mongodb.protection.access.util.MongoDBConstructionUtils;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.ObjectUtils;
import org.springframework.stereotype.Component;

import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.function.Function;
import java.util.stream.Collectors;

/**
 * MongoDB集群provider
 *
 */
@Component
@Slf4j
public class MongoDBClusterProvider extends DatabaseEnvironmentProvider {
    private final JsonSchemaValidator jsonSchemaValidator;

    private final MongoDBBaseService mongoDBBaseService;

    /**
     * 构造方法
     *
     * @param providerManager providerManager
     * @param pluginConfigManager pluginConfigManager
     * @param jsonSchemaValidator jsonSchemaValidator
     * @param mongoDBBaseService mongodb 实际业务service
     */
    public MongoDBClusterProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        JsonSchemaValidator jsonSchemaValidator, MongoDBBaseService mongoDBBaseService) {
        super(providerManager, pluginConfigManager);
        this.jsonSchemaValidator = jsonSchemaValidator;
        this.mongoDBBaseService = mongoDBBaseService;
    }

    /**
     * 实现校验和查询联调性检查接口
     *
     * @param environment 入库资源
     */
    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("Start check MongoDB cluster data. resource name: {}, uuid: {}", environment.getName(),
            environment.getUuid());
        jsonSchemaValidator.doValidate(environment, ResourceSubTypeEnum.MONGODB_CLUSTER.getType());
        ProtectedResource protectedResource = MongoDBConstructionUtils.getProtectedResource(environment);
        List<ProtectedResource> instanceList = environment.getDependencies().get(DatabaseConstants.CHILDREN);

        // 认证方式一致性校验
        checkAuthTypeIsSame(instanceList);
        // 单机、副本和主从、分片都不支持添加/刪除节点, 只允许修改名称和认证信息, 集群类型不允许修改。
        instanceList.forEach(instance -> mongoDBBaseService.checkMongoDBEnvironmentSize(instance,
            VerifyUtil.isEmpty(environment.getUuid())));
        if (!VerifyUtil.isEmpty(environment.getUuid())) {
            checkUpdateParam(environment);
        }

        // 前端下发的所有的ip和port
        List<String> urlList = mongoDBBaseService.getAllIpAndPortList(environment);
        List<AppEnvResponse> appEnvResponseList = mongoDBBaseService.getAppEnvResponses(protectedResource, urlList,
            true);
        environment.setVersion(appEnvResponseList.get(0).getExtendInfo().get(DatabaseConstants.VERSION));
        List<NodeInfo> clusterNodesCollect = MongoDBConstructionUtils.getNodeInfos(appEnvResponseList);
        List<Map<String, String>> appEnvExtendInfo = getAppEnvExtendInfo(appEnvResponseList);

        checkClusterStatus(instanceList, appEnvResponseList);

        // 校验是否属于同一集群
        checkAllNodesIsSameCluster(clusterNodesCollect, urlList, appEnvExtendInfo,
            environment.getExtendInfoByKey(MongoDBConstants.CLUSTE_TYPE));

        List<MongoClusterNodesExtendInfo> nodesExtendInfos = updateClusterNodes(environment, clusterNodesCollect,
            appEnvExtendInfo);
        environment.getExtendInfo()
            .put(MongoDBConstants.CLUSTER_NODES, JSONArray.fromObject(nodesExtendInfos).toString());
        long count = nodesExtendInfos.stream()
            .filter(mongoClusterNodesExtendInfo -> MongoDBClusterRoleEnum.PRIMARY.getRole()
                .equals(mongoClusterNodesExtendInfo.getStateStr()))
            .count();
        environment.getExtendInfo().put(DatabaseConstants.VERSION, environment.getVersion());
        environment.getExtendInfo().put(DatabaseConstants.NODE_COUNT, String.valueOf(count));
        environment.setEndpoint("127.0.0.1");
        environment.setPath(environment.getName());
        environment.setCluster(true);
        environment.setLinkStatus(String.valueOf(LinkStatusEnum.ONLINE.getStatus()));
        if (VerifyUtil.isEmpty(environment.getUuid())) {
            String envIdentity = environment.getName() + environment.getSubType();
            environment.setUuid(UUID.nameUUIDFromBytes(envIdentity.getBytes(Charset.defaultCharset())).toString());
        }
        log.info("End check MongoDB cluster. resource name: {}, uuid: {}", environment.getName(),
            environment.getUuid());
    }

    private void checkClusterStatus(List<ProtectedResource> instanceList, List<AppEnvResponse> appEnvResponseList) {
        try {
            // 副本集集群在线小于一半节点或者主节点健康状态为离线;
            mongoDBBaseService.checkReplicationCluster(instanceList, appEnvResponseList);
            // 分片集群缺少 route config 或者 shard服务信息; 离线
            mongoDBBaseService.checkShardCLuster(appEnvResponseList);
        } catch (LegoCheckedException e) {
            if (e.getErrorCode() == CommonErrorCode.AGENT_NETWORK_ERROR) {
                throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_INCONSISTENT,
                    "Exist agent not online");
            }
            throw e;
        }
    }

    private List<MongoClusterNodesExtendInfo> updateClusterNodes(ProtectedEnvironment environment,
        List<NodeInfo> clusterNodesCollect, List<Map<String, String>> appEnvExtendInfo) {
        List<MongoClusterNodesExtendInfo> clusterInfo = getClusterNodes(clusterNodesCollect, appEnvExtendInfo);
        if (MongoDBClusterTypeEnum.SHARD.getType()
            .equals(environment.getExtendInfoByKey(MongoDBConstants.CLUSTE_TYPE))) {
            updateNodesInstanceName(appEnvExtendInfo, clusterInfo);
            updateShardIndex(clusterInfo);
        }
        return clusterInfo;
    }

    private void updateShardIndex(List<MongoClusterNodesExtendInfo> clusterInfo) {
        List<String> shardInfos = clusterInfo.stream()
            .filter(clusterInfos -> MongoDBNodeTypeEnum.SHARD.getType().equals(clusterInfos.getShardClusterType()))
            .map(MongoClusterNodesExtendInfo::getInstanceNameInfos)
            .distinct()
            .collect(Collectors.toList());
        Map<String, String> updateShardIndexMap = new HashMap<>();
        for (int count = 0; count < shardInfos.size(); count++) {
            updateShardIndexMap.put(shardInfos.get(count), String.valueOf(count + 1));
        }

        for (MongoClusterNodesExtendInfo mongoClusterNodesExtendInfo : clusterInfo) {
            mongoClusterNodesExtendInfo.setShardIndex(
                Optional.ofNullable(updateShardIndexMap.get(mongoClusterNodesExtendInfo.getInstanceNameInfos()))
                    .orElse(""));
        }
    }

    private void updateNodesInstanceName(List<Map<String, String>> appEnvExtendInfo,
        List<MongoClusterNodesExtendInfo> clusterInfo) {
        Map<String, String> mongosMap = appEnvExtendInfo.stream()
            .filter(appEnvResponseExtendInfo -> MongoDBNodeTypeEnum.MONGOS.getType()
                .equals(appEnvResponseExtendInfo.get(MongoDBConstants.SHARD_CLUSTER_TYPE)))
            .findFirst()
            .orElse(new HashMap<>());
        Map<String, String> shardIpAndNameMap = Arrays.stream(
            mongosMap.get(MongoDBConstants.AGENT_SHARD_LIST).split(";"))
            .map(ipAndNameString -> ipAndNameString.split("/"))
            .collect(Collectors.toMap(ipAndNameList -> ipAndNameList[1], ipAndNameList -> ipAndNameList[0]));
        List<String> shardIpList = Arrays.stream(mongosMap.get(MongoDBConstants.AGENT_SHARD_LIST).split(";"))
            .map(ipAndNameString -> ipAndNameString.split("/")[1])
            .collect(Collectors.toList());
        for (MongoClusterNodesExtendInfo mongoClusterNodesExtendInfo : clusterInfo) {
            updateInstanceName(shardIpAndNameMap, shardIpList, mongoClusterNodesExtendInfo);
        }
    }

    private void updateInstanceName(Map<String, String> shardIpAndNameMap, List<String> shardIpList,
        MongoClusterNodesExtendInfo mongoClusterNodesExtendInfo) {
        for (String shardIp : shardIpList) {
            if (mongoClusterNodesExtendInfo.getInstanceNameInfos().contains(shardIp)) {
                mongoClusterNodesExtendInfo.setShardInstanceName(shardIpAndNameMap.get(shardIp));
                return;
            }
        }
    }

    private void checkAuthTypeIsSame(List<ProtectedResource> instanceList) {
        long count = instanceList.stream()
            .map(ProtectedResource::getAuth)
            .map(Authentication::getAuthType)
            .distinct()
            .count();
        if (count == 1) {
            instanceList.forEach(resource -> {
                if (!VerifyUtil.isEmpty(resource.getAuth())
                    && resource.getAuth().getAuthType() == Authentication.APP_PASSWORD) {
                    mongoDBBaseService.checkKeyLength(resource.getAuth().getAuthKey(), resource.getAuth().getAuthPwd());
                }
            });
            return;
        }
        throw new LegoCheckedException(DatabaseErrorCode.AUTH_TYPE_NO_CONSISTENT, "This MongoDB auth type count > 1");
    }

    private void checkAllNodesIsSameCluster(List<NodeInfo> clusterNodesCollect, List<String> urlList,
        List<Map<String, String>> appEnvExtendInfo, String clusterType) {
        List<String> agentIpAndPortList = appEnvExtendInfo.stream()
            .map(extendInfo -> extendInfo.get(MongoDBConstants.AGENT_HOST))
            .distinct()
            .collect(Collectors.toList());
        log.info("Agent ip size: {}, register ip size: {}", agentIpAndPortList.size(), urlList.size());
        checkPortsIsStream(urlList, agentIpAndPortList);
        checkPortsIsStream(clusterNodesCollect.stream().map(NodeInfo::getName).distinct().collect(Collectors.toList()),
            appEnvExtendInfo.stream()
                .filter(extendInfo -> !MongoDBNodeTypeEnum.MONGOS.getType()
                    .equals(extendInfo.get(MongoDBConstants.SHARD_CLUSTER_TYPE)))
                .map(extendInfo -> extendInfo.get(MongoDBConstants.LOCAL_HOST))
                .distinct()
                .collect(Collectors.toList()));
        List<String> agentNodesList = appEnvExtendInfo.stream()
            .filter(agentNodes -> ObjectUtils.isNotEmpty(agentNodes.get(MongoDBConstants.AGENT_NODES)))
            .map(agentNodes -> agentNodes.get(MongoDBConstants.AGENT_NODES))
            .collect(Collectors.toList());
        if (agentNodesList.size() == 0) {
            // 如果节点不属于同一集群报错
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_INCONSISTENT, new String[] {},
                "check node error!");
        }
        if (MongoDBClusterTypeEnum.SHARD.getType().equals(clusterType)) {
            checkShardIsSameCluster(clusterNodesCollect, appEnvExtendInfo, agentNodesList);
        } else {
            checkReplicationIsSameCluster(appEnvExtendInfo, agentNodesList);
        }
    }

    private void checkReplicationIsSameCluster(List<Map<String, String>> appEnvExtendInfo,
        List<String> agentNodesList) {
        List<String> allAgentNodesList = agentNodesList.stream()
            .map(agentNodes -> Arrays.stream(agentNodes.split(",")).collect(Collectors.toList()))
            .flatMap(Collection::stream)
            .distinct()
            .collect(Collectors.toList());
        List<String> agentList = appEnvExtendInfo.stream()
            .map(extendInfo -> extendInfo.get(MongoDBConstants.LOCAL_HOST))
            .distinct()
            .collect(Collectors.toList());
        log.info("Agent ip size: {}, agent all ip size: {}", allAgentNodesList.size(), agentList.size());
        checkPortsIsStream(agentList, allAgentNodesList);
    }

    private void checkShardIsSameCluster(List<NodeInfo> clusterNodesCollect, List<Map<String, String>> appEnvExtendInfo,
        List<String> agentNodesList) {
        List<String> allAgentNodesList = agentNodesList.stream()
            .map(agentNodes -> Arrays.stream(agentNodes.split(",")).collect(Collectors.toList()))
            .flatMap(Collection::stream)
            .distinct()
            .collect(Collectors.toList());
        appEnvExtendInfo.stream()
            .filter(extendInfo -> MongoDBNodeTypeEnum.MONGOS.getType()
                .equals(extendInfo.get(MongoDBConstants.SHARD_CLUSTER_TYPE)))
            .map(extendInfo -> extendInfo.get(MongoDBConstants.AGENT_HOST))
            .distinct()
            .forEach(allAgentNodesList::remove);

        List<String> agentList = clusterNodesCollect.stream()
            .filter(clusterNodes -> ObjectUtils.isNotEmpty(clusterNodes.getExtendInfo()))
            .map(NodeInfo::getExtendInfo)
            .filter(clusterNodes -> ObjectUtils.isNotEmpty(clusterNodes.get(MongoDBConstants.INSTANCE_NAME_LIST)))
            .map(clusterNodes -> clusterNodes.get(MongoDBConstants.INSTANCE_NAME_LIST))
            .map(agentNodes -> Arrays.stream(agentNodes.split(",")).collect(Collectors.toList()))
            .flatMap(Collection::stream)
            .distinct()
            .collect(Collectors.toList());
        clusterNodesCollect.stream()
            .filter(clusterNodes -> ObjectUtils.isNotEmpty(clusterNodes.getExtendInfo()))
            .filter(
                clusterNodes -> ObjectUtils.isNotEmpty(clusterNodes.getExtendInfo().get(MongoDBConstants.STATE_STR)))
            .filter(clusterNodes -> MongoDBClusterRoleEnum.ARBITER.getRole()
                .equals(clusterNodes.getExtendInfo().get(MongoDBConstants.STATE_STR)))
            .forEach(clusterNodes -> agentList.remove(clusterNodes.getName()));
        log.info("Agent ip size: {}, agent all ip size: {}", allAgentNodesList.size(), agentList.size());
        checkPortsIsStream(agentList, allAgentNodesList);
    }

    private void checkPortsIsStream(List<String> urlList, List<String> agentIpAndPortList) {
        if (urlList.size() != agentIpAndPortList.size() || !new HashSet<>(agentIpAndPortList).containsAll(urlList)) {
            // 如果节点不属于同一集群报错
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_INCONSISTENT, new String[] {},
                "check node error!");
        }
    }

    private List<MongoClusterNodesExtendInfo> getClusterNodes(List<NodeInfo> clusterNodesCollect,
        List<Map<String, String>> appEnvExtendInfo) {
        List<MongoClusterNodesExtendInfo> nodesExtendInfos = new ArrayList<>();
        Map<String, Map<String, String>> appEnvExtendInfoMap = appEnvExtendInfo.stream()
            .collect(Collectors.toMap(extendInfo -> extendInfo.get(MongoDBConstants.LOCAL_HOST), Function.identity(),
                (key1, key2) -> key2));
        clusterNodesCollect.forEach(nodeInfo -> {
            nodesExtendInfos.add(buildClusterExtendInfoByNode(nodeInfo, appEnvExtendInfoMap));
            appEnvExtendInfoMap.remove(nodeInfo.getName());
        });
        Set<String> nameList = appEnvExtendInfoMap.keySet();
        for (String name : nameList) {
            nodesExtendInfos.add(
                MongoDBConstructionUtils.buildMongoClusterNodesExtendInfo(appEnvExtendInfoMap.get(name),
                    new NodeInfo()));
        }
        return nodesExtendInfos;
    }

    private static MongoClusterNodesExtendInfo buildClusterExtendInfoByNode(NodeInfo nodeInfo,
        Map<String, Map<String, String>> appEnvExtendInfoMap) {
        return MongoDBConstructionUtils.buildMongoClusterNodesExtendInfo(appEnvExtendInfoMap.get(nodeInfo.getName()),
            nodeInfo);
    }

    private List<Map<String, String>> getAppEnvExtendInfo(List<AppEnvResponse> appEnvResponseList) {
        return appEnvResponseList.stream().map(AppEnvResponse::getExtendInfo).collect(Collectors.toList());
    }

    private void checkUpdateParam(ProtectedEnvironment environment) {
        log.info("update shard cluster name");
        List<ProtectedResource> protectedResources = environment.getDependencies()
            .get(MongoDBConstants.DELETE_NODES_KEY);
        // -children 框架会根据此字段删除实例节点,校验以免误删除
        if (CollectionUtils.isNotEmpty(protectedResources) && protectedResources.size() != 0) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "can not update cluster instance");
        }
    }

    @Override
    public void validate(ProtectedEnvironment environment) {
        try {
            mongodbHealthCheck(environment);
        } catch (LegoCheckedException e) {
            log.error("health check failed LegoCheckedException: ", ExceptionUtil.getErrorMessage(e));
            List<MongoClusterNodesExtendInfo> nodesExtendInfos = com.alibaba.fastjson.JSONArray.parseArray(
                environment.getExtendInfo().get(MongoDBConstants.CLUSTER_NODES), MongoClusterNodesExtendInfo.class);
            updateExtendInfos(nodesExtendInfos);
            environment.getExtendInfo()
                .put(MongoDBConstants.CLUSTER_NODES, JSONArray.fromObject(nodesExtendInfos).toString());
            ProtectedEnvironment newEnv = new ProtectedEnvironment();
            newEnv.setUuid(environment.getUuid());
            newEnv.setExtendInfo(environment.getExtendInfo());
            mongoDBBaseService.updateResourceService(newEnv);
            throw e;
        }
    }

    private void updateExtendInfos(List<MongoClusterNodesExtendInfo> nodesExtendInfos) {
        for (MongoClusterNodesExtendInfo nodesExtendInfo : nodesExtendInfos) {
            nodesExtendInfo.setNodeStatus(String.valueOf(LinkStatusEnum.OFFLINE.getStatus()));
        }
    }

    private void mongodbHealthCheck(ProtectedEnvironment environment) {
        log.info("Start healthCheck MongoDB cluster online. resource name: {}, uuid: {}", environment.getName(),
            environment.getUuid());
        List<ProtectedResource> childProtectedResources = environment.getDependencies().get(DatabaseConstants.CHILDREN);
        mongoDBBaseService.checkAgentIsOnline(environment);
        // 获取当前所有查询集群列表
        ProtectedResource clusterProtectedResource = MongoDBConstructionUtils.getProtectedResource(environment);
        List<String> urlList = mongoDBBaseService.getAllIpAndPortList(environment);
        List<AppEnvResponse> appEnvResponseList = mongoDBBaseService.getAppEnvResponses(clusterProtectedResource,
            urlList, false);

        checkClusterStatus(childProtectedResources, appEnvResponseList);
        log.info("Before updateClusterNodes. resource name: {}, uuid: {}", environment.getName(),
            environment.getUuid());
        List<NodeInfo> clusterNodesCollect = MongoDBConstructionUtils.getNodeInfos(appEnvResponseList);
        List<Map<String, String>> appEnvExtendInfo = getAppEnvExtendInfo(appEnvResponseList);
        // 判断是否满足足够的主节点;
        mongoDBBaseService.checkPrimarySizeIsMeet(environment.getExtendInfo().get(DatabaseConstants.NODE_COUNT),
            clusterNodesCollect);
        // 更新数据;
        mongoDBBaseService.updateEnvironmentExtendInfoClusterNodes(appEnvExtendInfo, clusterNodesCollect, environment);
        log.info("Before updateResourceService. resource name: {}, uuid: {}.", environment.getName(),
            environment.getUuid());
        // 检查信息数据入库：
        ProtectedEnvironment newEnv = new ProtectedEnvironment();
        newEnv.setUuid(environment.getUuid());
        newEnv.setExtendInfo(environment.getExtendInfo());
        mongoDBBaseService.updateResourceService(newEnv);
        log.info("End healthCheck MongoDB online. resource name: {}, uuid: {}", environment.getName(),
            environment.getUuid());
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.MONGODB_CLUSTER.equalsSubType(resourceSubType);
    }
}
