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
package openbackup.data.access.framework.agent;

import com.huawei.oceanprotect.system.base.user.service.UserService;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.mng.constant.CopyResourcePropertiesConstant;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.common.constants.AgentKeyConstant;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelector;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.SelectorManager;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.json.JSONObjectCovert;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 代理选择管理器
 *
 **/
@Slf4j
@Component
public class AgentSelectorManager implements SelectorManager {
    private final ProviderManager providerManager;

    private final DefaultProtectAgentSelector defaultSelector;

    private final UserService userService;
    private DataBaseAgentSelector dataBaseAgentSelector;

    /**
     * agent选择管理器构造函数
     *
     * @param providerManager provider管理器
     * @param userService 用户服务
     * @param dataBaseAgentSelector 基础数据库插件选择agent
     * @param defaultSelector 默认选择agent
     */
    public AgentSelectorManager(
            ProviderManager providerManager,
            UserService userService,
            DataBaseAgentSelector dataBaseAgentSelector,
            DefaultProtectAgentSelector defaultSelector) {
        this.providerManager = providerManager;
        this.userService = userService;
        this.dataBaseAgentSelector = dataBaseAgentSelector;
        this.defaultSelector = defaultSelector;
    }

    /**
     * 根据资源信息选择agent
     *
     * @param resource 资源
     * @param jobType 任务类型
     * @param parameters 扩展参数
     * @return 代理
     */
    @Override
    public List<Endpoint> selectAgentByResource(
            ProtectedResource resource, String jobType, Map<String, String> parameters) {
        AgentSelectParam agentSelectParam =
                AgentSelectParam.builder().resource(resource).jobType(jobType).parameters(parameters).build();
        AgentSelector agentSelector =
                providerManager.findProvider(AgentSelector.class, agentSelectParam, null);
        if (agentSelector == null && ResourceTypeEnum.DATABASE.getType().equals(resource.getType())) {
            agentSelector = dataBaseAgentSelector;
        }
        if (agentSelector != null) {
            return agentSelector.getSelectedAgents(agentSelectParam);
        }
        ProtectAgentSelector selector = providerManager.findProviderOrDefault(
            ProtectAgentSelector.class,
            resource instanceof ProtectedEnvironment ? resource.getType() : resource.getEnvironment().getType(),
            defaultSelector);
        return selector.select(resource, parameters);
    }

    /**
     * 根据副本信息选择agent
     *
     * @param copy 副本信息
     * @return Endpoint对象列表
     */
    public List<Endpoint> selectAgentsByCopy(Copy copy) {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(copy.getResourceSubType());
        ProtectedEnvironment environment = buildEnvFromCopy(copy.getResourceProperties());
        protectedResource.setEnvironment(environment);
        // 设置保护代理
        ProtectAgentSelector selector =
                providerManager.findProvider(ProtectAgentSelector.class, environment.getType(), null);
        if (selector == null) {
            selector = defaultSelector;
        }
        Map<String, String> parameters = new HashMap<>();
        TokenBo tokenBo = TokenBo.get(null);
        if (tokenBo != null) {
            parameters.put(
                    AgentKeyConstant.USER_INFO,
                    JSONObject.writeValueAsString(userService.getUserInfoByUserId(tokenBo.getUser().getId())));
        }
        return selector.select(protectedResource, parameters);
    }

    private ProtectedEnvironment buildEnvFromCopy(String resourceProperties) {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        final JSONObject targetJsonObject =
                JSONObjectCovert.covertLowerUnderscoreKeyToLowerCamel(JSONObject.fromObject(resourceProperties));
        protectedEnvironment.setSubType(
                targetJsonObject.getString(CopyResourcePropertiesConstant.ENVIRONMENT_SUB_TYPE));
        protectedEnvironment.setType(targetJsonObject.getString(CopyResourcePropertiesConstant.ENVIRONMENT_TYPE));
        return protectedEnvironment;
    }
}
