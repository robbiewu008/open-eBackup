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
package openbackup.ndmp.protection.access.provider;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.access.framework.agent.ProtectAgentSelector;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.protection.common.constants.AgentKeyConstant;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.ndmp.protection.access.service.NdmpService;
import openbackup.system.base.common.constants.SymbolConstant;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * NdmpAgentSelector
 *
 */
@Component
@Slf4j
public class NdmpAgentSelector implements ProtectAgentSelector {
    @Autowired
    private DefaultProtectAgentSelector defaultSelector;

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private NdmpService ndmpService;

    /**
     * 获取agent列表
     *
     * @param protectedResource 备份/恢复资源{@link ProtectedResource}
     * @param parameters 参数
     * @return agent列表
     */
    @Override
    public List<Endpoint> select(ProtectedResource protectedResource, Map<String, String> parameters) {
        // 索引任务如果找到副本的保护对象绑定的代理，且代理在线，使用此agent
        // 否则使用内置代理
        if (StringUtils.isNotEmpty(parameters.get(AgentKeyConstant.COPY_AGENT))) {
            String[] agentArr = parameters.get(AgentKeyConstant.COPY_AGENT).split(SymbolConstant.SEMICOLON);
            List<Endpoint> agentList = new ArrayList<>(agentArr.length);
            for (String agentId : agentArr) {
                Optional<ProtectedResource> resOptional = resourceService.getBasicResourceById(agentId);
                Optional<ProtectedEnvironment> environment = resOptional.filter(
                        resource -> resource instanceof ProtectedEnvironment)
                    .map(resource -> (ProtectedEnvironment) resource);
                if (environment.isPresent() && LinkStatusEnum.ONLINE.getStatus()
                    .toString()
                    .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(environment.get()))) {
                    agentList.add(
                        new Endpoint(environment.get().getUuid(), environment.get().getEndpoint(),
                            environment.get().getPort()));
                }
            }
            log.info("Ndmp index task get copy agent: {}", JsonUtil.json(agentList));
            if (CollectionUtils.isNotEmpty(agentList)) {
                return agentList;
            } else {
                // 查询内置代理
                List<ProtectedResource> interAgents = this.ndmpService.getInterAgents();
                if (CollectionUtils.isNotEmpty(interAgents)) {
                    return interAgents.stream()
                        .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort()))
                        .collect(Collectors.toList());
                }
            }
        }
        return defaultSelector.select(protectedResource, parameters);
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.NDMP_BACKUPSET.getType().equals(subType) || ResourceSubTypeEnum.NDMP.equalsSubType(
            subType);
    }
}
