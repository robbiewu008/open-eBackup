package openbackup.ndmp.protection.access.service;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.List;
import java.util.Map;

/**
 * 功能描述
 *
 * @author t30021437
 * @since 2023-05-06
 */
public interface NdmpService {
    /**
     * 检查连通性
     *
     * @param protectedResource protectedResource
     */
    void checkConnention(ProtectedResource protectedResource);

    /**
     * 获取已存在的NDMP资源信息
     *
     * @param filter 查询条件
     * @return 已存在的GaussDb资源信息
     */
    List<ProtectedResource> getexistingNdmpresources(Map<String, Object> filter);

    /**
     * 获取数据库已经注册的资源
     *
     * @param subType 查询资源类型，项目或者实例
     * @param filter 查询条件信息入库
     * @return 已存在的GaussDB资源信息
     */
    List<ProtectedResource> getexistingNdmpresources(String subType, Map<String, Object> filter);

    /**
     * 获取agent对象
     *
     * @param envId agent的uuid
     * @return 查询资源对象
     */
    ProtectedEnvironment getEnvironmentById(String envId);

    /**
     * 获取agent信息
     *
     * @param agentResources agentResources
     * @param protectedResource protectedResource
     * @return AppEnvResponse AppEnvResponse agent 信息
     */
    AppEnvResponse getAppEnvResponse(List<ProtectedResource> agentResources, ProtectedResource protectedResource);

    /**
     * 返回节点信息列表
     *
     * @return 节点信息列表
     */
    List<TaskEnvironment> supplyNodes();

    /**
     * 修改task备份信息
     *
     * @param backupTask backupTask
     */
    void modifyBackupTaskParam(BackupTask backupTask);

    /**
     * delete 资源信息
     *
     * @param resources resources
     */
    void deleteResourses(String[] resources);

    /**
     * 获取内置agents资源信息
     *
     * @return List<ProtectedResource> 内置agents资源信息
     */
    List<ProtectedResource> getInterAgents();

    /**
     * 获取内置agents资源信息
     *
     * @param environment environment
     * @return List<ProtectedResource> 内置agents资源信息
     */
    List<ProtectedResource> getOneAgentHealthCheck(ProtectedEnvironment environment);

    /**
     * 获取可用的agent
     *
     * @param environment environment
     * @return List<ProtectedResource> 内置agents资源信息
     */
    List<ProtectedResource> getAvailableAgents(ProtectedEnvironment environment);

    /**
     * 根据NDMP设备的id获取可用的agent
     *
     * @param parentUuid parentUuid
     * @param agents agents
     * @return List<Endpoint> 内置agents资源信息
     */
    List<Endpoint> getAgents(String parentUuid, String agents);

    /**
     * 连通性检查
     *
     * @param environment environment
     * @param agent agent
     */
    void checkApplication(ProtectedEnvironment environment, ProtectedEnvironment agent);

    /**
     * 获取Endpoint信息
     *
     * @param agents agents
     * @return List<Endpoint> agents资源信息
     */
    List<Endpoint> getEndpointList(String agents);
}
