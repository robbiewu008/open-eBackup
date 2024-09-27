package openbackup.db2.protection.access.service;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.List;

/**
 * db2实例服务
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-01-03
 */
public interface Db2InstanceService {
    /**
     * 检查单实例是否已经被注册
     *
     * @param resource 单实例资源信息
     */
    void checkSingleInstanceIsRegistered(ProtectedResource resource);

    /**
     * 检查单实例的名称是否被修改
     *
     * @param resource 单实例资源信息
     */
    void checkSingleInstanceNameIsChanged(ProtectedResource resource);

    /**
     * 校验资源是否属于集群实例
     *
     * @param resource 集群实例资源信息
     * @return 校验结果
     */
    AgentBaseDto checkIsClusterInstance(ProtectedResource resource);

    /**
     * 检查集群实例是否已经被注册
     *
     * @param resource 集群实例资源信息
     */
    void checkClusterInstanceIsRegistered(ProtectedResource resource);

    /**
     * 检查集群实例的名称是否被修改
     *
     * @param resource 集群实例资源信息
     */
    void checkClusterInstanceNameIsChanged(ProtectedResource resource);


    /**
     * 过滤集群实例信息
     *
     * @param clusterInstance 集群实例资源信息
     */
    void filterClusterInstance(ProtectedResource clusterInstance);

    /**
     * 校验hadr集群实例
     *
     * @param resource 集群实例资源信息
     */
    void checkHadrClusterInstance(ProtectedResource resource);

    /**
     * 扫描数据库
     *
     * @param clusterInstance 集群实例资源信息
     * @param environment 集群实例资源信息
     * @return 扫描结果
     */
    List<ProtectedResource> scanDatabase(ProtectedResource clusterInstance, ProtectedEnvironment environment);
}
