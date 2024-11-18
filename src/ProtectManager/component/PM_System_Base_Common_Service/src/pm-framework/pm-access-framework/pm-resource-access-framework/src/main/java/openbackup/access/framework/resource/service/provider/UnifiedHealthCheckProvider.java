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