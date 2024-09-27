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
package openbackup.tpops.protection.access.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.StreamUtil;
import openbackup.tpops.protection.access.constant.TpopsGaussDBConstant;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-28
 */
@Slf4j
@Component
public class TpopsGaussDBAgentProvider extends DataBaseAgentSelector {
    private final ProtectedEnvironmentRetrievalsService envRetrievalsService;

    private final ResourceService resourceService;

    /**
     * 构造器
     *
     * @param envRetrievalsService envRetrievalsService
     * @param resourceService resourceService
     */
    public TpopsGaussDBAgentProvider(ProtectedEnvironmentRetrievalsService envRetrievalsService,
        ResourceService resourceService) {
        this.envRetrievalsService = envRetrievalsService;
        this.resourceService = resourceService;
    }

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        Optional<ProtectedResource> resourceOptional = resourceService.getResourceById(
            agentSelectParam.getResource().getRootUuid());
        if (!resourceOptional.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST);
        }
        ProtectedResource resource = resourceOptional.get();
        if (Objects.equals(agentSelectParam.getJobType(), JobTypeEnum.BACKUP.getValue())) {
            return buildAgentEndpointFromEnv(resource);
        } else if (Objects.equals(agentSelectParam.getJobType(), JobTypeEnum.RESOURCE_SCAN.getValue())) {
            return resource.getDependencies()
                .get(TpopsGaussDBConstant.GAUSSDB_AGENTS)
                .stream()
                .map(this::getAgentEndpoint)
                .collect(Collectors.toList());
        } else if (Objects.equals(agentSelectParam.getJobType(), JobTypeEnum.RESTORE.getValue())) {
            Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap = new HashMap<>();
            protectedResourceMap = envRetrievalsService.collectConnectableResources(resource.getUuid());
            return protectedResourceMap.values()
                .stream()
                .flatMap(List::stream)
                .filter(agentEnv -> scan(agentEnv))
                .map(this::getAgentEndpoint)
                .collect(Collectors.toList());
        } else {
            return super.getSelectedAgents(agentSelectParam);
        }
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        return ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.equalsSubType(agentSelectParam.getResource().getSubType());
    }

    // 代理信息
    private boolean scan(ProtectedEnvironment agentResource) {
        try {
            String linkStatus = EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(agentResource);
            log.info("Get agent {}, linStatus: {}", agentResource.getEndpoint(), linkStatus);
            if (linkStatus.equals(LinkStatusEnum.ONLINE.getStatus().toString())) {
                return true;
            }
        } catch (LegoCheckedException exception) {
            log.error("Get tpops resource linStatus failed, ", exception);
        }
        return false;
    }

    /**
     * 从环境中获取agent端口信息
     *
     * @param protectedEnvironment 环境信息
     * @return List<Endpoint>
     */
    private List<Endpoint> buildAgentEndpointFromEnv(ProtectedResource protectedEnvironment) {
        Map<String, List<ProtectedResource>> dependencies = protectedEnvironment.getDependencies();
        List<ProtectedResource> resourceList = dependencies.get(DatabaseConstants.AGENTS);
        return resourceList.stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .map(this::getAgentEndpoint)
            .collect(Collectors.toList());
    }

    private Endpoint getAgentEndpoint(ProtectedResource agentEnv) {
        return new Endpoint(agentEnv.getUuid(), agentEnv.getEndpoint(),
            Optional.ofNullable(agentEnv.getPort()).orElse(0));
    }
}
