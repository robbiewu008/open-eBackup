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
package openbackup.obs.plugin.service.impl;

import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import com.huawei.oceanprotect.repository.tapelibrary.common.util.JsonUtil;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.AgentBusinessService;
import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.obs.plugin.common.constants.EnvironmentConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.SymbolConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * ObjectStorageConnectionChecker
 *
 */
@Component
@Slf4j
public class ObjectStorageConnectionChecker extends UnifiedResourceConnectionChecker {
    @Autowired
    private ResourceService resourceService;

    @Autowired
    private AgentBusinessService agentBusinessService;

    @Autowired
    private EncryptorService encryptorService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     */
    public ObjectStorageConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        AgentUnifiedService agentUnifiedService) {
        super(environmentRetrievalsService, agentUnifiedService);
    }

    /**
     * 监测对象适用，该provider适用于对象存储
     *
     * @param object 资源
     * @return 检查结果
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return StringUtils.equals(ResourceSubTypeEnum.OBJECT_STORAGE.getType(), object.getSubType());
    }

    /**
     * 查询所有agent
     *
     * @param resource 需要检查的受保护资源
     * @return agent列表
     */
    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(ProtectedResource resource) {
        List<ProtectedEnvironment> agents = new ArrayList<>();
        String agentIds = resource.getExtendInfo().get(EnvironmentConstant.KEY_AGENTS);
        log.info("collectConnectableResources {}", agentIds);
        // 为空，使用内置agent
        if (StringUtils.isNotEmpty(agentIds)) {
            String[] agentIdArr = agentIds.split(SymbolConstant.SEMICOLON);
            for (String agentId : agentIdArr) {
                ProtectedEnvironment agent = resourceService.getResourceById(agentId)
                    .filter(env -> env instanceof ProtectedEnvironment)
                    .map(env -> (ProtectedEnvironment) env)
                    .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.RESOURCE_IS_NOT_EXIST,
                        "Protected environment is not exists!"));
                agents.add(agent);
            }
        } else {
            agents = agentBusinessService.queryInternalAgentEnv();
        }
        if (StringUtils.isNotEmpty(resource.getUuid())) {
            String decryptedSk = encryptorService.decrypt(
                resource.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_SK));

            String decryptedPwd = "";
            if (StringUtils.equals(resource.getAuth()
                    .getExtendInfo()
                    .getOrDefault(EnvironmentConstant.KEY_PROXY_ENABLE, EnvironmentConstant.PROXY_DISABLE_VALUE),
                EnvironmentConstant.PROXY_ENABLE_VALUE)) {
                decryptedPwd = encryptorService.decrypt(
                    resource.getAuth().getExtendInfo().get(EnvironmentConstant.KEY_PROXY_USER_PWD));
            }

            // 校验连通性需要使用明文，明文在用完后清理
            resource.getAuth().getExtendInfo().put(EnvironmentConstant.KEY_SK, decryptedSk);
            resource.getAuth().getExtendInfo().put(EnvironmentConstant.KEY_PROXY_USER_PWD, decryptedPwd);
        }
        Map<ProtectedResource, List<ProtectedEnvironment>> result = new HashMap<>();
        result.put(resource, agents);
        return result;
    }

    /**
     * 检查连通性
     *
     * @param protectedResource 需要检查的资源
     * @return 检查报告
     */
    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource protectedResource) {
        try {
            return super.generateCheckResult(protectedResource);
        } catch (LegoCheckedException | LegoUncheckedException e) {
            CheckResult<Object> checkResult = new CheckResult<>();
            checkResult.setEnvironment(protectedResource.getEnvironment());
            ActionResult actionResult = new ActionResult();
            actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
            actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            actionResult.setMessage(e.getMessage());
            checkResult.setResults(actionResult);
            return checkResult;
        }
    }

    /**
     * 获取检查结果
     *
     * @param checkReports 检查报告
     * @param context 上下文
     * @return 检查结果
     */
    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReports,
        Map<String, Object> context) {
        // 检查结果，只要有一个通过，即为ok，剔除异常结果
        List<ActionResult> actionResults = super.collectActionResults(checkReports, context);
        log.debug("collectActionResults {}", JsonUtil.toJsonString(actionResults));
        List<ActionResult> filterResults = actionResults.stream()
            .filter(result -> result.getCode() == ActionResult.SUCCESS_CODE)
            .collect(Collectors.toList());
        // 如果过滤为空，则返回原有结果，防止返回UI空结果体
        if (CollectionUtils.isEmpty(filterResults)) {
            return actionResults;
        }
        return filterResults;
    }
}
