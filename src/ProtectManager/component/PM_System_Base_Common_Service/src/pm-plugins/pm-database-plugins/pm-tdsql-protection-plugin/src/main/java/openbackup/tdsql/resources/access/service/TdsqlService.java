package openbackup.tdsql.resources.access.service;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.tdsql.resources.access.dto.cluster.OssNode;
import openbackup.tdsql.resources.access.dto.cluster.SchedulerNode;
import openbackup.tdsql.resources.access.dto.instance.DataNode;
import openbackup.tdsql.resources.access.dto.instance.TdsqlGroup;

import java.util.List;

/**
 * 功能描述  TDSQL服务
 *
 * @author z30047175
 * @since 2023-05-22
 */
public interface TdsqlService {
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
     * 获取oss节点信息
     *
     * @param environment 受保护环境
     * @return 管理数据库节点
     */
    List<OssNode> getOssNode(ProtectedEnvironment environment);

    /**
     * 获取scheduler节点信息
     *
     * @param environment 受保护环境
     * @return 管理数据库节点
     */
    List<SchedulerNode> getSchedulerNode(ProtectedEnvironment environment);

    /**
     * 获取单个Oss节点的连通性
     *
     * @param ossNode oss节点
     * @param environment 受保护环境
     * @return 是否连接
     */
    boolean singleOssNodeConnectCheck(OssNode ossNode, ProtectedEnvironment environment);

    /**
     * 获取单个scheduler节点的连通性
     *
     * @param schedulerNode scheduler节点
     * @param environment 受保护环境
     * @return 是否连接
     */
    boolean singleSchedulerNodeConnectCheck(SchedulerNode schedulerNode, ProtectedEnvironment environment);

    /**
     * 获取单个dataNode节点的连通性
     *
     * @param dataNode instance data节点
     * @param environment 受保护环境
     * @return 是否连接
     */
    boolean singleDataNodeConnectCheck(DataNode dataNode, ProtectedEnvironment environment);

    /**
     * 校验数据节点的ip是否属于所关联的代理主机
     *
     * @param dataNode instance data节点
     * @param environment 受保护环境
     * @return 是否连接
     */
    boolean checkDataNodeIsMatchAgent(DataNode dataNode, ProtectedEnvironment environment);

    /**
     * 校验实例信息（下发到数据节点执行OSS查询实例数据和入参进行比较）
     *
     * @param tdsqlGroup 分布式实例数据
     * @param environment 受保护环境
     * @return 实例信息是否一致
     */
    boolean checkGroupInfo(TdsqlGroup tdsqlGroup, ProtectedEnvironment environment);

    /**
     * 得到环境下面的所有子实例
     *
     * @param parentUuid 环境uuid
     * @param subType 实例subType
     * @return 子资源
     */
    List<ProtectedResource> getChildren(String parentUuid, String subType);

    /**
     * 更新实例及其数据节点的linkStatus
     *
     * @param updateResource updateResource
     * @param status status
     */
    void updateInstanceLinkStatus(ProtectedResource updateResource, String status);

    /**
     * 更新实例及其数据节点的linkStatus
     *
     * @param updateResource updateResource
     * @param status status
     */
    void updateClusterGroupLinkStatus(ProtectedResource updateResource, String status);

    /**
     * 获取计算节点对应的dataNode
     *
     * @param instanceResource 实例资源
     * @return 计算节点
     */
    List<DataNode> getInstanceDataNodes(ProtectedResource instanceResource);

    /**
     * 获取clusterInfo
     *
     * @param resource 受保护环境
     * @return 计算节点
     */
    ProtectedEnvironment getClusterEnv(ProtectedResource resource);

    /**
     * 查询集群实例信息
     *
     * @param environmentConditions environmentConditions
     * @param environment environment
     * @return PageListResponse
     */
    PageListResponse<ProtectedResource> getBrowseResult(BrowseEnvironmentResourceConditions environmentConditions,
        ProtectedEnvironment environment);

    /**
     * 查询实例机型信息
     *
     * @param environmentConditions environmentConditions
     * @param environment environment
     * @param queryType queryType
     * @return PageListResponse
     */
    PageListResponse<ProtectedResource> getClusterHosts(BrowseEnvironmentResourceConditions environmentConditions,
        ProtectedEnvironment environment, String queryType);

    /**
     * 查询集群信息
     *
     * @param environment environment
     * @param agentId agentId
     * @return AppEnvResponse
     */
    AppEnvResponse queryClusterInfo(ProtectedEnvironment environment, String agentId);

    /**
     * 获取单个节点的连通性
     *
     * @param dataNode instance data节点
     * @param environment 受保护环境
     * @return 是否连接
     */
    boolean singleDataNodeHealthCheck(DataNode dataNode, ProtectedEnvironment environment);

    /**
     * 根据Agent主机信息，获取Agent主机的Endpoint
     *
     * @param env Agent环境信息
     * @return Agent对应的Endpoint信息
     */
    Endpoint getAgentEndpoint(ProtectedEnvironment env);

    /**
     * 检验subtype是否一致
     *
     * @param copy 源
     * @param targetResource 目标资源
     */
    void checkSubType(Copy copy, TaskResource targetResource);

    /**
     * 移除资源的数据存储仓白名单
     *
     * @param resourceId resource id
     */
    void removeDataRepoWhiteListOfResource(String resourceId);

    /**
     * 持续挂载仓解挂载
     *
     * @param tdsqlGroup tdsqlGroup
     * @param resource resource
     */
    void umountDataRepo(TdsqlGroup tdsqlGroup, ProtectedResource resource);
}
