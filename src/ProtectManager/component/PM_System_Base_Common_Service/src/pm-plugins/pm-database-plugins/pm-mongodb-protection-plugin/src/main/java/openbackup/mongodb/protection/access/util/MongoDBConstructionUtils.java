/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.mongodb.protection.access.util;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.mongodb.protection.access.bo.MongoClusterNodesExtendInfo;
import openbackup.mongodb.protection.access.constants.MongoDBConstants;
import openbackup.mongodb.protection.access.enums.MongoDBClusterRoleEnum;
import openbackup.mongodb.protection.access.enums.MongoDBNodeTypeEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import org.apache.commons.lang3.StringUtils;
import org.springframework.util.ObjectUtils;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.stream.Collectors;

/**
 * mongodb 参数构造工具类
 *
 * @author lwx1012372
 * @version [DataBackup 1.5.0]
 * @since 2023-04-27
 */
public class MongoDBConstructionUtils {
    /**
     * 获取环境对象转换的资源对象
     *
     * @param environment 环境对象
     * @return 资源对象
     */
    public static ProtectedResource getProtectedResource(ProtectedEnvironment environment) {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(environment.getSubType());
        protectedResource.setType(environment.getType());
        protectedResource.setExtendInfo(environment.getExtendInfo());
        protectedResource.setAuth(environment.getAuth());
        protectedResource.setName(environment.getName());
        protectedResource.setDependencies(environment.getDependencies());
        return protectedResource;
    }

    /**
     * 根据节点信息更新扩展参数
     *
     * @param nodeInfo 节点信息
     * @param nodesExtendInfo 扩展参数
     */
    public static void setMongoClusterNodeForNodeInfo(NodeInfo nodeInfo, MongoClusterNodesExtendInfo nodesExtendInfo) {
        Map<String, String> nodeExtendInfo = nodeInfo.getExtendInfo();
        // 集群情况下存在该extendInfo;
        nodesExtendInfo.setPriority(nodeExtendInfo.get(MongoDBConstants.PRIORITY));
        nodesExtendInfo.setVoteRight(nodeExtendInfo.get(MongoDBConstants.VOTE_RIGHT));
        nodesExtendInfo.setNodeStatus(nodeExtendInfo.get(MongoDBConstants.NODE_HEALTH));
        if (!MongoDBClusterRoleEnum.ARBITER.equals(MongoDBClusterRoleEnum.getValue(nodesExtendInfo.getStateStr()))) {
            MongoDBClusterRoleEnum value = MongoDBClusterRoleEnum.getValue(
                nodeExtendInfo.get(MongoDBConstants.STATE_STR));
            nodesExtendInfo.setStateStr(value.getRole());
            nodesExtendInfo.setRole(MongoDBConstants.CLUSTER_ROLE_MAP.get(value));
        }
        nodesExtendInfo.setInstanceNameInfos(nodeExtendInfo.get(MongoDBConstants.INSTANCE_NAME_LIST));
        nodesExtendInfo.setInstanceId(nodeExtendInfo.get(MongoDBConstants.ID));
    }

    /**
     * 根据节点扩展信息更新扩展参数
     *
     * @param extendInfo 节点扩展信息
     * @param nodesExtendInfo 扩展参数
     */
    public static void setMongoClusterNodeForEnvExtendInfo(Map<String, String> extendInfo,
        MongoClusterNodesExtendInfo nodesExtendInfo) {
        if (nodesExtendInfo.getShardClusterType().equals(MongoDBNodeTypeEnum.MONGOS.getType())) {
            nodesExtendInfo.setNodeStatus(String.valueOf(LinkStatusEnum.ONLINE.getStatus()));
        }
        nodesExtendInfo.setDataPath(extendInfo.get(MongoDBConstants.DATA_PATH));
        nodesExtendInfo.setClusterInstanceName(extendInfo.get(MongoDBConstants.INSTANCE_NAME));
        nodesExtendInfo.setShardClusterType(extendInfo.get(MongoDBConstants.SHARD_CLUSTER_TYPE));
        nodesExtendInfo.setArgv(extendInfo.get(MongoDBConstants.ARGV));
        nodesExtendInfo.setParsed(extendInfo.get(MongoDBConstants.PARSED));
        nodesExtendInfo.setConfigPath(extendInfo.get(MongoDBConstants.CONFIG_PATH));
    }

    /**
     * 构造实例入参的MongoClusterNodesExtendInfo对象
     *
     * @param appEnvResponseExtendInfo 查询对象的扩展参数
     * @param nodeInfo 节点参数
     * @return MongoClusterNodesExtendInfo对象
     */
    public static MongoClusterNodesExtendInfo buildMongoClusterNodesExtendInfo(
        Map<String, String> appEnvResponseExtendInfo, NodeInfo nodeInfo) {
        MongoClusterNodesExtendInfo nodesExtendInfo = new MongoClusterNodesExtendInfo();
        nodesExtendInfo.setDataPath(appEnvResponseExtendInfo.get(MongoDBConstants.DATA_PATH));
        nodesExtendInfo.setClusterInstanceName(appEnvResponseExtendInfo.get(MongoDBConstants.INSTANCE_NAME));
        nodesExtendInfo.setShardInstanceName("");
        nodesExtendInfo.setShardIndex("");
        nodesExtendInfo.setShardClusterType(appEnvResponseExtendInfo.get(MongoDBConstants.SHARD_CLUSTER_TYPE));
        nodesExtendInfo.setArgv(appEnvResponseExtendInfo.get(MongoDBConstants.ARGV));
        nodesExtendInfo.setParsed(appEnvResponseExtendInfo.get(MongoDBConstants.PARSED));
        nodesExtendInfo.setConfigPath(appEnvResponseExtendInfo.get(MongoDBConstants.CONFIG_PATH));
        nodesExtendInfo.setHostUrl(appEnvResponseExtendInfo.get(MongoDBConstants.LOCAL_HOST));
        nodesExtendInfo.setAgentHost(appEnvResponseExtendInfo.get(MongoDBConstants.AGENT_HOST));
        // 单机情况下没有nodeInfo，需要手动配置该参数
        if (ObjectUtils.isEmpty(nodeInfo.getExtendInfo())) {
            nodesExtendInfo.setPriority("");
            nodesExtendInfo.setVoteRight("");
            nodesExtendInfo.setNodeStatus(String.valueOf(LinkStatusEnum.ONLINE.getStatus()));
            nodesExtendInfo.setRole("1");
            nodesExtendInfo.setInstanceNameInfos("");
            nodesExtendInfo.setInstanceId("");
            return nodesExtendInfo;
        }
        Map<String, String> nodeExtendInfo = nodeInfo.getExtendInfo();
        // 集群情况下存在该extendInfo;
        nodesExtendInfo.setPriority(nodeExtendInfo.get(MongoDBConstants.PRIORITY));
        nodesExtendInfo.setVoteRight(nodeExtendInfo.get(MongoDBConstants.VOTE_RIGHT));
        nodesExtendInfo.setNodeStatus(nodeExtendInfo.get(MongoDBConstants.NODE_HEALTH));
        nodesExtendInfo.setStateStr(nodeExtendInfo.get(MongoDBConstants.STATE_STR));
        nodesExtendInfo.setRole(String.valueOf(nodeExtendInfo.get(MongoDBConstants.ROLE)));
        nodesExtendInfo.setInstanceNameInfos(nodeExtendInfo.get(MongoDBConstants.INSTANCE_NAME_LIST));
        nodesExtendInfo.setInstanceId(nodeExtendInfo.get(MongoDBConstants.ID));
        return nodesExtendInfo;
    }

    /**
     * 根据查询信息获取节点信息
     *
     * @param appEnvResponseList 查询信息
     * @return 节点信息
     */
    public static List<NodeInfo> getNodeInfos(List<AppEnvResponse> appEnvResponseList) {
        List<String> nameList = new ArrayList<>();
        return appEnvResponseList.stream()
            .filter(appEnvResponse -> Objects.equals(appEnvResponse.getExtendInfo().get(MongoDBConstants.EXIST_NODES),
                String.valueOf(IsmNumberConstant.ONE)))
            .map(AppEnvResponse::getNodes)
            .flatMap(Collection::stream)
            .filter(nodeInfo -> checkNodeInfo(nodeInfo, nameList))
            .collect(Collectors.toList());
    }

    private static boolean checkNodeInfo(NodeInfo nodeInfo, List<String> nameList) {
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
}
