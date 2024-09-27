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
package openbackup.openstack.protection.access.provider;

import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.access.framework.agent.ProtectAgentSelector;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.protection.common.constants.AgentKeyConstant;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.auth.UserInnerResponse;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.OptionalUtil;
import openbackup.system.base.util.StreamUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * Openstack agent筛选器
 *
 * @author c30016231
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-08
 */
@Component
@Slf4j
public class OpenstackAgentSelector implements ProtectAgentSelector {
    private final ResourceService resourceService;

    private final DefaultProtectAgentSelector defaultSelector;

    /**
     * 构造器注入
     *
     * @param resourceService resource数据库
     * @param defaultSelector 默认agent选择器
     */
    public OpenstackAgentSelector(ResourceService resourceService, DefaultProtectAgentSelector defaultSelector) {
        this.resourceService = resourceService;
        this.defaultSelector = defaultSelector;
    }

    /**
     * 获取agent列表
     *
     * @param protectedResource 备份/恢复资源{@link ProtectedResource}
     * @param parameters        备份恢复参数
     * @return agent 列表
     */
    @Override
    public List<Endpoint> select(ProtectedResource protectedResource, Map<String, String> parameters) {
        String agents = parameters.get(AgentKeyConstant.AGENTS_KEY);
        String userStr = parameters.get(AgentKeyConstant.USER_INFO);
        UserInnerResponse userInfo = JSONObject.cast(userStr, UserInnerResponse.class, false);
        log.info("Job exec userId: {}.", Optional.ofNullable(userInfo).map(UserInnerResponse::getUserId).orElse(null));
        List<Endpoint> endpointList = VerifyUtil.isEmpty(agents)
                ? selectByEnv(protectedResource)
                : defaultSelector.selectByAgentParameter(agents, userInfo);
        log.info("Openstack select available agent {}, endpointList:{}", agents,
            endpointList.stream().map(Endpoint::getId).collect(Collectors.joining(",")));
        return endpointList;
    }

    @Override
    public boolean applicable(String envType) {
        return ResourceTypeEnum.OPEN_STACK.equalsType(envType);
    }

    private List<Endpoint> selectByEnv(ProtectedResource protectedResource) {
        List<Endpoint> endpointList = new ArrayList<>();
        boolean shouldDecrypt = false;
        String envId = Optional.ofNullable(protectedResource.getEnvironment())
            .map(ProtectedEnvironment::getUuid)
            .orElseGet(protectedResource::getRootUuid);
        log.info("Select agent by env, env id is :{}", envId);
        resourceService.getResourceById(shouldDecrypt, envId)
            .flatMap(OptionalUtil.match(ProtectedEnvironment.class))
            .map(ProtectedEnvironment::getDependencies)
            .flatMap(dependency -> Optional.ofNullable(dependency.get(OpenstackConstant.AGENTS)))
            .ifPresent(agents -> endpointList.addAll(generateEndpoints(agents)));
        return endpointList;
    }

    private List<Endpoint> generateEndpoints(List<ProtectedResource> agents) {
        return agents.stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .filter(agent -> LinkStatusEnum.ONLINE.getStatus().toString()
            .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(agent)))
            .peek(agent -> log.debug("dependent online agent is :{}", agent.getUuid()))
            .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort()))
            .collect(Collectors.toList());
    }
}
