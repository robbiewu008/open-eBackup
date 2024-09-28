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
package openbackup.mongodb.protection.access.service;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.List;
import java.util.Map;

/**
 * mongodb 实际业务service
 *
 */
public interface MongoDBBaseService {
    /**
     * 检查实例节点是否存在情况
     *
     * @param environment 单实例资源信息
     * @param isRegistered 是否注册过
     */
    void checkMongoDBEnvironmentSize(ProtectedResource environment, boolean isRegistered);

    /**
     * 构造实例入参的MongoClusterNodesExtendInfo对象
     *
     * @param protectedResource 查询对象的扩展参数
     * @param protectedEnvironment 节点参数
     * @return AppEnvResponse
     */
    AppEnvResponse getAppEnvAgentInfo(ProtectedResource protectedResource, ProtectedEnvironment protectedEnvironment);

    /**
     * 获取uuid的环境信息
     *
     * @param envId uuid
     * @return 环境信息
     */
    ProtectedEnvironment getEnvironmentById(String envId);

    /**
     * 获取uuid的资源信息
     *
     * @param envId uuid
     * @return 资源信息
     */
    ProtectedResource getResource(String envId);

    /**
     * 更新检查信息
     *
     * @param protectedEnvironment 更新检查信息
     */
    void updateResourceService(ProtectedEnvironment protectedEnvironment);

    /**
     * 更新环境扩展参数的clusterNode信息
     *
     * @param appEnvExtendInfo 查询的env的map集合
     * @param clusterNodesCollect 查询的mongoDB的nodeInfo集合
     * @param protectedEnvironment env信息
     */
    void updateEnvironmentExtendInfoClusterNodes(List<Map<String, String>> appEnvExtendInfo,
        List<NodeInfo> clusterNodesCollect, ProtectedEnvironment protectedEnvironment);

    /**
     * 受保护环境对象内的资源Agent类表
     *
     * @param uuid 受保护环境uuid
     * @return 集群环境对应的节点信息列表
     */
    List<TaskEnvironment> buildBackupTaskNodes(String uuid);

    /**
     * 根据资源id获取当前锁情况
     *
     * @param envId 资源id
     * @return 锁情况
     */
    List<LockResourceBo> getRestoreLockResource(String envId);

    /**
     * 检查agent是否在线
     *
     * @param environment 入库资源
     */
    void checkAgentIsOnline(ProtectedEnvironment environment);

    /**
     * 判断主节点是否满足要求
     *
     * @param count 原主节点个数
     * @param clusterNodesCollect 查询的主节点个数
     */
    void checkPrimarySizeIsMeet(String count, List<NodeInfo> clusterNodesCollect);

    /**
     * 检查分片集群校验
     *
     * @param appEnvResponseList 查询到分片集群信息
     */
    void checkShardCLuster(List<AppEnvResponse> appEnvResponseList);

    /**
     * 检查复制集群校验
     *
     * @param protectedResources 资源信息
     * @param appEnvResponseList 查询到复制集群信息
     */
    void checkReplicationCluster(List<ProtectedResource> protectedResources, List<AppEnvResponse> appEnvResponseList);

    /**
     * 校验用户名密码参数信息
     *
     * @param username 用户名
     * @param password 密码
     */
    void checkKeyLength(String username, String password);

    /**
     * 获取集群信息
     *
     * @param protectedResource 资源信息
     * @param urlList url列表
     * @param isRegisterCheck 是否为注册
     * @return 查询到集群信息列表
     */
    List<AppEnvResponse> getAppEnvResponses(ProtectedResource protectedResource, List<String> urlList,
        boolean isRegisterCheck);

    /**
     * 获取注册资源列表
     *
     * @param environment 注册资源
     * @return 注册资源列表
     */
    List<String> getAllIpAndPortList(ProtectedEnvironment environment);
}
