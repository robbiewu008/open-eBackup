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
package openbackup.dameng.protection.access.service;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;

import java.util.List;
import java.util.Set;

/**
 * dameng查询、校验的接口定义
 *
 */
public interface DamengService {
    /**
     * 查询并校验集群实例
     *
     * @param resource 资源
     * @return 集群查询结果
     */
    List<AppEnvResponse> check(ProtectedResource resource);

    /**
     * 从单实例的dependency里，获取对应的Agent主机
     *
     * @param instance 单实例
     * @return Agent主机信息
     */
    ProtectedEnvironment queryAgentEnvironment(ProtectedResource instance);

    /**
     * 获取已经注册的主机uuid和实例端口集合
     *
     * @param environment 注册的单机或集群环境
     * @return 主机uuid和实例端口集合
     */
    Set<String> getExistingUuidAndPort(ProtectedEnvironment environment);

    /**
     * 拼接主机uuid和实例端口
     *
     * @param uuid 主机uuid
     * @param port 实例端口
     * @return 拼接后的uuid和实例端口
     */
    String connectUuidAndPort(String uuid, String port);

    /**
     * 将nodes转换为NodeInfo
     *
     * @param environment 环境信息
     * @return 节点信息列表
     */
    List<NodeInfo> getNodeInfoFromNodes(ProtectedEnvironment environment);

    /**
     * 针对集群实例，将子实例信息中的auth,实例端口和主备信息设置到nodes中
     *
     * @param uuid 资源uuid
     * @return nodes列表
     */
    List<TaskEnvironment> buildTaskNodes(String uuid);

    /**
     * 针对单机，添加nodes
     *
     * @param agents agents
     * @return nodes列表
     */
    List<TaskEnvironment> buildTaskHosts(List<Endpoint> agents);

    /**
     * 根据Agent主机信息，获取Agent主机的Endpoint
     *
     * @param environment Agent环境信息
     * @return Agent对应的Endpoint信息
     */
    Endpoint getAgentEndpoint(ProtectedEnvironment environment);

    /**
     * 查询集群信息
     *
     * @param instances 集群实例
     * @return 集群查询结果
     */
    List<AppEnvResponse> queryClusterInfo(List<ProtectedResource> instances);

    /**
     * 获取agent环境信息
     *
     * @param uuid 环境uuid
     * @return agent环境信息
     */
    ProtectedEnvironment getEnvironmentById(String uuid);

    /**
     * 恢复时校验数据库版本是否一致
     *
     * @param task 恢复任务
     */
    void checkDbVersion(RestoreTask task);

    /**
     * 设置恢复模式
     *
     * @param task 恢复任务
     */
    void setRestoreMode(RestoreTask task);

    /**
     * 获取agent列表
     *
     * @param uuid 环境的uuid
     * @return agent列表
     */
    List<Endpoint> getEndpointList(String uuid);

    /**
     * 设置恢复的高级参数
     *
     * @param task 恢复任务
     */
    void setRestoreAdvanceParams(RestoreTask task);
}
