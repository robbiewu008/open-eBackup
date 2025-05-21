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
package com.huawei.oceanprotect.k8s.protection.access.provider;

import com.huawei.oceanprotect.k8s.protection.access.service.K8sCommonService;
import com.huawei.oceanprotect.k8s.protection.access.util.K8sUtil;

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
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * k8s连通性检测
 *
 */
@Component
public class K8sConnectionChecker extends UnifiedResourceConnectionChecker {
    private final AgentBusinessService agentBusinessService;
    private final ResourceService resourceService;
    private final K8sCommonService commonService;

    /**
     * 构造函数
     *
     * @param environmentRetrievalsService environmentRetrievalsService
     * @param agentUnifiedService agentUnifiedService
     * @param agentBusinessService agentBusinessService
     * @param resourceService resourceService
     * @param commonService commonService
     */
    public K8sConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
            AgentUnifiedService agentUnifiedService, AgentBusinessService agentBusinessService,
            ResourceService resourceService, K8sCommonService commonService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.agentBusinessService = agentBusinessService;
        this.resourceService = resourceService;
        this.commonService = commonService;
    }

    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(ProtectedResource resource) {
        List<ProtectedEnvironment> agents = agentBusinessService.queryInternalAgentEnv();
        HashMap<ProtectedResource, List<ProtectedEnvironment>> resourceMap = new HashMap<>();
        resourceMap.put(resource, agents);
        return resourceMap;
    }

    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource protectedResource) {
        try {
            commonService.addIpRule(protectedResource.getEndpoint(), protectedResource.getPort());
            return super.generateCheckResult(protectedResource);
        } finally {
            commonService.deleteIpRule(protectedResource.getEndpoint(), protectedResource.getPort());
        }
    }

    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReport, Map<String, Object> context) {
        List<CheckResult<Object>> checkResultList = checkReport.get(0).getResults();
        boolean isMatchSuccess = false;

        // 用于记录内置agent的连通性
        Map<String, String> internalAgentConnectionMap = new HashMap<>();
        for (CheckResult<Object> checkResult : checkResultList) {
            ActionResult actionResult = checkResult.getResults();
            boolean isSuccess = Objects.equals(actionResult.getCode(), ActionResult.SUCCESS_CODE);
            isMatchSuccess = isMatchSuccess || isSuccess;
            internalAgentConnectionMap.put(
                    K8sUtil.getInternalAgentConnectionKey(checkResult.getEnvironment().getUuid()),
                    String.valueOf(isSuccess));
        }

        ProtectedResource updateResource = new ProtectedResource();
        updateResource.setUuid(checkReport.get(0).getResource().getUuid());
        updateResource.setExtendInfo(internalAgentConnectionMap);
        resourceService.updateSourceDirectly(Collections.singletonList(updateResource));

        List<ActionResult> actionResults = super.collectActionResults(checkReport, context);
        if (isMatchSuccess) {
            return Collections.singletonList(new ActionResult(ActionResult.SUCCESS_CODE, "success"));
        }
        return actionResults;
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.KUBERNETES_CLUSTER_COMMON.getType().equals(object.getSubType());
    }
}
