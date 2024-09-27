/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.exchange.protection.access.service;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.protection.access.provider.sdk.backup.v2.PostBackupTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.copy.model.Copy;

import java.util.List;
import java.util.Optional;
import java.util.Set;

/**
 * ExchangeService
 *
 * @author s30036254
 * @since 2023-04-28
 */
public interface ExchangeService {
    /**
     * 获取受保护环境
     *
     * @param envId 环境id
     * @return 受保护环境
     */
    ProtectedEnvironment getEnvironmentById(String envId);

    /**
     * 获取受保护资源
     *
     * @param uuid 资源唯一id
     * @return 受保护资源
     */
    ProtectedResource getResourceById(String uuid);

    /**
     * 更新资源的状态
     *
     * @param resourceId resourceId
     * @param status status
     */
    void updateResourceLinkStatus(String resourceId, String status);

    /**
     * 检查连通性
     *
     * @param environment protectedResource
     */
    void checkConnection(ProtectedEnvironment environment);

    /**
     * 调用插件接口查询相关信息
     *
     * @param environment protectedResource
     * @param oldEnvironment oldEnvironment
     * @return 集群信息
     */
    AppEnvResponse queryClusterInfo(ProtectedEnvironment environment, ProtectedEnvironment oldEnvironment);

    /**
     * 判断是否已经存在副本
     *
     * @param resourceId 资源id
     * @return 是否存在副本
     */
    Boolean isExistCopy(String resourceId);

    /**
     * 若上一次日志备份失败, 设置下一次备份为全量备份
     *
     * @param postBackupTask 任务参数
     */
    void setNextBackupTypeWhenLogBackFail(PostBackupTask postBackupTask);

    /**
     * 查询最新的副本
     *
     * @param resourceId 资源id
     * @return 副本
     */
    Optional<Copy> getLatestCopy(String resourceId);

    /**
     * 邮箱扫描
     *
     * @param environment 环境
     * @param agentEnvironment agent
     * @param database 数据库
     * @return 邮箱列表
     */
    List<ProtectedResource> scanMailboxes(ProtectedEnvironment environment, ProtectedResource agentEnvironment,
        ProtectedResource database);

    /**
     * 检查域名和用户名
     *
     * @param environment 受保护环境
     */
    void checkDomainUser(ProtectedEnvironment environment);

    /**
     * 校验传入的任务并发数参数
     *
     * @param env 环境信息
     */
    void checkMaxConcurrentJobNumber(ProtectedEnvironment env);

    /**
     * 获取已经注册的主机uuid和实例端口集合
     *
     * @param environment 注册的单机或集群环境
     * @return 主机uuid和实例端口集合
     */
    Set<String> getExistingUuid(ProtectedEnvironment environment);

    /**
     * 将nodes转换为NodeInfo
     *
     * @param environment 环境信息
     * @return 节点信息列表
     */
    List<NodeInfo> getNodeInfoFromNodes(ProtectedEnvironment environment);

    /**
     * 拼接主机uuid和实例端口
     *
     * @param uuid 主机uuid
     * @param port 实例端口
     * @return 拼接后的uuid和实例端口
     */
    String connectUuidAndPort(String uuid, String port);

    /**
     * 从单实例的dependency里，获取对应的Agent主机
     *
     * @param instance 单实例
     * @return Agent主机信息
     */
    ProtectedEnvironment queryAgentEnvironment(ProtectedResource instance);

    /**
     * 删除资源时校验是否存在正在运行的任务，存在的话抛出错误码
     *
     * @param resource 资源
     */
    void checkCanDeleteResource(ProtectedResource resource);
}
