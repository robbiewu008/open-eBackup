/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
 */

package openbackup.access.framework.resource.service.provider;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentHealthCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 功能描述: 默认健康检查 provider
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-23
 */
@Slf4j
@Component
public class UnifiedHealthCheckProvider implements EnvironmentHealthCheckProvider {
    // agent返回的成功码
    private static final String SUCCESS = "0";
    private static final String AGENTS = "agents";

    private final ResourceService resourceService;
    private final AgentUnifiedService agentService;

    /**
     * 构造器注入
     *
     * @param resourceService resourceService
     * @param agentService agentService
     */
    public UnifiedHealthCheckProvider(ResourceService resourceService, AgentUnifiedService agentService) {
        this.resourceService = resourceService;
        this.agentService = agentService;
    }

    @Override
    public boolean applicable(ProtectedEnvironment environment) {
        return false;
    }

    @Override
    public void healthCheck(ProtectedEnvironment environment) {
        log.info("Health check start, name: {}, uuid: {}", environment.getName(), environment.getUuid());

        boolean isConnectSuccess = checkConnectivity(environment);
        if (!isConnectSuccess) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Health check failed");
        }

        log.info("Health check finish, name: {}, uuid: {}, status: {}.",
                environment.getName(), environment.getUuid(), environment.getLinkStatus());
    }

    private boolean checkConnectivity(ProtectedEnvironment environment) {
        List<ProtectedResource> agentResources = environment.getDependencies().get(AGENTS);
        List<ProtectedEnvironment> agentEnvList = agentResources.stream()
                .map(agentResource -> resourceService.getResourceById(agentResource.getUuid()))
                .filter(Optional::isPresent)
                .map(Optional::get)
                .filter(resource -> resource instanceof ProtectedEnvironment)
                .map(resource -> (ProtectedEnvironment) resource)
                .collect(Collectors.toList());
        log.info("Prepare to health check, agent num:{}.", agentEnvList.size());
        for (ProtectedEnvironment agent : agentEnvList) {
            if (!EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(agent)
                    .equals(LinkStatusEnum.ONLINE.getStatus().toString())) {
                log.warn("Agent is offline, uuid: {}, status: {}.", agent.getUuid(),
                        EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(agent));
                continue;
            }
            AgentBaseDto response = agentService.checkApplication(environment, agent);
            if (!VerifyUtil.isEmpty(response) && SUCCESS.equals(response.getErrorCode())) {
                return true;
            }
        }
        return false;
    }
}