package openbackup.opengauss.resources.access.service.impl;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.opengauss.resources.access.service.OpenGaussAgentService;
import openbackup.opengauss.resources.access.util.OpenGaussClusterUtil;
import openbackup.opengauss.resources.access.util.OpenGaussRestoreUtil;
import openbackup.system.base.util.StreamUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * OpenGaussAgentService与agent插件交互的接口实现类
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-22
 */
@Service
@Slf4j
public class OpenGaussAgentServiceImpl implements OpenGaussAgentService {
    private final AgentUnifiedService agentUnifiedService;
    private final ProtectedEnvironmentService protectedEnvironmentService;

    public OpenGaussAgentServiceImpl(AgentUnifiedService agentUnifiedService,
        ProtectedEnvironmentService protectedEnvironmentService) {
        this.agentUnifiedService = agentUnifiedService;
        this.protectedEnvironmentService = protectedEnvironmentService;
    }

    @Override
    public AppEnvResponse getClusterNodeStatus(ProtectedResource protectedResource) {
        ProtectedEnvironment environment = protectedResource.getEnvironment();
        return agentUnifiedService.getClusterInfo(protectedResource, environment);
    }

    @Override
    public List<Endpoint> getAgentEndpoint(String envId) {
        ProtectedEnvironment protectedEnvironment = protectedEnvironmentService.getEnvironmentById(envId);
        return OpenGaussClusterUtil.buildAgentEndpointFromEnv(protectedEnvironment);
    }

    @Override
    public TaskEnvironment buildEnvironmentNodes(TaskEnvironment taskEnvironment) {
        ProtectedEnvironment protectedEnvironment = protectedEnvironmentService.getEnvironmentById(
            taskEnvironment.getUuid());

        // 回填部署类型到拓展字段
        Map<String, String> envExtendInfo = Optional.ofNullable(taskEnvironment.getExtendInfo())
            .orElseGet(HashMap::new);
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE,
            protectedEnvironment.getExtendInfoByKey(DatabaseConstants.DEPLOY_TYPE));
        taskEnvironment.setExtendInfo(envExtendInfo);

        List<ProtectedEnvironment> agentsEnvironments = protectedEnvironment.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .collect(Collectors.toList());

        List<TaskEnvironment> newNodes = new ArrayList<>();
        OpenGaussRestoreUtil.checkEnvironmentExistAndBuildNodes(taskEnvironment);
        List<TaskEnvironment> nodes = taskEnvironment.getNodes();
        for (TaskEnvironment node : nodes) {
            Optional<ProtectedEnvironment> protectedEnv = agentsEnvironments.stream()
                .filter(agent -> agent.getName().equals(node.getName()))
                .findFirst();
            protectedEnv.ifPresent(environment -> {
                node.setUuid(environment.getUuid());
                node.setPort(environment.getPort());
            });
            newNodes.add(node);
        }
        taskEnvironment.setNodes(newNodes);
        return taskEnvironment;
    }
}
