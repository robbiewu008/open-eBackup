package openbackup.kingbase.protection.access.service;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.List;

/**
 * KingBase服务
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-22
 */
public interface KingBaseService {
    /**
     * 根据资源UUID获取资源信息
     *
     * @param resourceId 资源的UUID
     * @return ProtectedResource 资源信息
     */
    ProtectedResource getResourceById(String resourceId);

    /**
     * 根据实例资源获取对应的agents
     *
     * @param instanceResource 实例资源信息
     * @return List<Endpoint> agents信息
     */
    List<Endpoint> getAgentsByInstanceResource(ProtectedResource instanceResource);

    /**
     * 根据实例资源获取对应的环境nodes
     *
     * @param instanceResource 实例资源信息
     * @return List<TaskEnvironment> agents信息
     */
    List<TaskEnvironment> getEnvNodesByInstanceResource(ProtectedResource instanceResource);

    /**
     * 根据实例资源获取子实例信息
     *
     * @param instanceResource 实例资源信息
     * @return List<TaskResource> 子实例
     */
    List<TaskResource> getSubInstances(ProtectedResource instanceResource);

    /**
     * 根据实例资源类型获取部署类型值
     *
     * @param subType 实例资源类型
     * @return String 部署类型
     */
    String getDeployType(String subType);
}
