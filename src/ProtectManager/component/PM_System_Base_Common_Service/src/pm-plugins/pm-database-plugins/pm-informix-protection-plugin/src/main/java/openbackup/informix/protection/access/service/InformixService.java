package openbackup.informix.protection.access.service;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.List;
import java.util.Map;

/**
 * InformixService
 *
 * @author zwx951267
 * @version [DataBackup 1.5.0]
 * @since 2023-05-09
 */
public interface InformixService {
    /**
     * 注册服务时校验日志备份路径
     *
     * @param environment 服务注册信息，有问题时直接抛异常
     */
    void checkLogBackupItem(ProtectedEnvironment environment);

    /**
     * 注册服务时校验主机信息
     *
     * @param environment 服务注册信息，有问题时直接抛异常
     */
    void checkHostInfo(ProtectedEnvironment environment);

    /**
     * 注册实例时调用的agent接口
     *
     * @param resource 实例注册信息
     * @param host 实例注册信息
     * @param type 实例注册信息
     * @return PageListResponse<ProtectedResource>
     */
    PageListResponse<ProtectedResource> listResource(
            ProtectedResource resource, ProtectedEnvironment host, String type);

    /**
     * 注册实例前检查是不是已经注册过该实例
     *
     * @param resource 实例注册信息，有问题直接抛出异常
     */
    void checkInstanceExist(ProtectedResource resource);

    /**
     * 更新注册实例时返回给框架的参数信息
     *
     * @param resource 实例注册信息
     * @param clusterExtendInfo 实例注册信息
     */
    void updateResource(ProtectedResource resource, Map<String, Map<String, String>> clusterExtendInfo);

    /**
     * 获取注册实例时插件返回的参数信息
     *
     * @param resource 实例注册信息
     * @param isRegister 是否为注册
     * @return  List<PageListResponse<ProtectedResource>> 实例注册信息，有问题直接抛出异常
     */
    List<PageListResponse<ProtectedResource>> getResponsesList(ProtectedResource resource, boolean isRegister);

    /**
     * 获取agent环境信息
     *
     * @param uuid 环境uuid
     * @return agent环境信息
     */
    ProtectedEnvironment getEnvironmentById(String uuid);

    /**
     * 连通性检查接口
     *
     * @param resource 实例注册信息，有问题直接抛出异常
     */
    void checkInstanceConnection(ProtectedResource resource);

    /**
     * 服务连通性检查接口
     *
     * @param environment 服务注册信息，有问题直接抛出异常
     */
    void checkServiceConnection(ProtectedEnvironment environment);

    /**
     * 处理主备集群场景下返回的各个主机的信息，比较信息并返回主节点的extendInfo
     *
     * @param responsesList 实例注册信息，有问题直接抛出异常
     * @return  Map<String, Map<String, String>> 实例注册信息
     */
    Map<String, Map<String, String>> getClusterExtendInfo(List<PageListResponse<ProtectedResource>> responsesList);

    /**
     * 注册单机实例
     *
     * @param resource 实例注册信息，有问题直接抛出异常
     * @param copyResource copyResource
     */
    void registerSingleInstance(ProtectedResource resource, ProtectedResource copyResource);

    /**
     * 从单实例的dependency里，获取对应的Agent主机
     *
     * @param instance 单实例
     * @return Agent主机信息
     */
    ProtectedEnvironment queryAgentEnvironment(ProtectedResource instance);

    /**
     * 操作集群实例注册/修改动作
     *
     * @param resource 实例信息
     * @param isRegister 注册为true，修改为false
     */
    void doClusterInstanceAction(ProtectedResource resource, boolean isRegister);

    /**
     * 操作单机实例注册/修改动作
     *
     * @param resource 实例信息
     * @param isRegister 注册为true，修改为false
     */
    void doSingleInstanceAction(ProtectedResource resource, boolean isRegister);

    /**
     * 获取nodes信息
     *
     * @param resource 实例信息
     * @return List<TaskEnvironment>
     */
    List<TaskEnvironment> getEnvNodesByInstanceResource(ProtectedResource resource);

    /**
     * 获取agents信息
     *
     * @param resource 实例信息
     * @return List<Endpoint>
     */
    List<Endpoint> getAgentsByInstanceResource(ProtectedResource resource);

    /**
     * 获取资源信息
     *
     * @param resourceId 资源Id
     * @return  ProtectedResource
     */
    ProtectedResource getResourceById(String resourceId);

    /**
     * 集群实例健康检查
     *
     * @param resource 实例信息
     */
    void healthCheckOfClusterInstance(ProtectedResource resource);

    /**
     * 检查应用
     *
     * @param environment 检查的资源或环境
     */
    void checkApplication(ProtectedEnvironment environment);

    /**
     * 移除资源的数据仓
     *
     * @param resourceId resource id
     */
    void removeRepositoryOfResource(String resourceId);
}
