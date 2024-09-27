package openbackup.opengauss.resources.access.service;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.List;

/**
 * OpenGaussAgentService与agent插件交互的接口
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-22
 */
public interface OpenGaussAgentService {
    /**
     * 查询集群节点信息
     *
     * @param protectedResource 检查的环境资源
     * @return AppEnvResponse 集群信息
     */
    AppEnvResponse getClusterNodeStatus(ProtectedResource protectedResource);

    /**
     * 获取agent端口信息
     *
     * @param envId 环境id
     * @return List<Endpoint> 端口信息
     */
    List<Endpoint> getAgentEndpoint(String envId);

    /**
     * 构建环境的nodes信息
     *
     * @param taskEnvironment taskEnvironment
     * @return 环境信息
     */
    TaskEnvironment buildEnvironmentNodes(TaskEnvironment taskEnvironment);
}
