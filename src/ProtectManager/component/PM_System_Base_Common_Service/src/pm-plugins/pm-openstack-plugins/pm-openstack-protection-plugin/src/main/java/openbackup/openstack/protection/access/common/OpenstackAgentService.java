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
package openbackup.openstack.protection.access.common;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * OpenStack所需的agent公共服务
 *
 */
@Slf4j
@Component
public class OpenstackAgentService {
    private final ProtectedEnvironmentRetrievalsService envRetrievalsService;

    private final AgentUnifiedService agentUnifiedService;

    public OpenstackAgentService(ProtectedEnvironmentRetrievalsService envRetrievalsService,
        AgentUnifiedService agentUnifiedService) {
        this.envRetrievalsService = envRetrievalsService;
        this.agentUnifiedService = agentUnifiedService;
    }

    /**
     * 获取环境的agent列表
     *
     * @param env 环境
     * @return agent列表
     */
    public List<ProtectedEnvironment> getAllAgents(ProtectedEnvironment env) {
        Map<ProtectedResource, List<ProtectedEnvironment>> map = envRetrievalsService.collectConnectableResources(env);
        return map.values().stream().flatMap(List::stream).collect(Collectors.toList());
    }

    /**
     * 检查环境依赖的agent连通性
     *
     * @param environment 注册环境
     */
    public void checkConnectivity(ProtectedEnvironment environment) {
        List<ProtectedResource> availableAgents = new ArrayList<>();
        AgentBaseDto response = null;
        List<ProtectedEnvironment> agents = getAllAgents(environment);
        for (ProtectedEnvironment agent : agents) {
            response = agentUnifiedService.checkApplication(environment, agent);
            if (!VerifyUtil.isEmpty(response) && OpenstackConstant.SUCCESS_CODE.equals(response.getErrorCode())) {
                availableAgents.add(agent);
                break;
            }
        }
        if (!availableAgents.isEmpty()) {
            log.info("openstack check connectivity success, available agent: {}", availableAgents.get(0).getUuid());
            return;
        }
        if (VerifyUtil.isEmpty(response) || VerifyUtil.isEmpty(response.getErrorMessage())) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "available agent is empty");
        }
        ActionResult result = JsonUtil.read(response.getErrorMessage(), ActionResult.class);
        throw new LegoCheckedException(Long.parseLong(result.getBodyErr()), result.getMessage());
    }

    /**
     * 查询环境信息
     *
     * @param environment 注册环境
     * @return 响应体 {@link AppEnvResponse}
     */
    public AppEnvResponse queryClusterInfo(ProtectedEnvironment environment) {
        List<ProtectedEnvironment> agents = getAllAgents(environment);
        for (ProtectedEnvironment agent : agents) {
            try {
                return agentUnifiedService.getClusterInfo(environment, agent);
            } catch (LegoCheckedException e) {
                log.error("query cluster info failed, envId:{}, agentId:{}.",
                    environment.getUuid(), agent.getUuid(), ExceptionUtil.getErrorMessage(e));
            }
        }
        throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "query cluster info error.");
    }
}
