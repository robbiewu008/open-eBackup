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
package openbackup.oceanprotect.k8s.protection.access.provider;

import openbackup.oceanprotect.k8s.protection.access.service.K8sCommonService;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.agent.ProtectAgentSelector;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * k8s agent筛选器
 *
 */
@Component
@Slf4j
public class K8sAgentSelector implements ProtectAgentSelector {
    private final K8sCommonService k8sCommonService;
    private final ResourceService resourceService;

    /**
     * 构造器注入
     *
     * @param k8sCommonService k8sCommonService
     * @param resourceService resourceService
     */
    public K8sAgentSelector(K8sCommonService k8sCommonService, ResourceService resourceService) {
        this.k8sCommonService = k8sCommonService;
        this.resourceService = resourceService;
    }

    /**
     * 获取agent列表
     * 注册环境时的内置agent
     *
     * @param protectedResource 备份/恢复资源{@link ProtectedResource}
     * @param parameters 备份恢复参数
     * @return agent 列表
     */
    @Override
    public List<Endpoint> select(ProtectedResource protectedResource, Map<String, String> parameters) {
        String rootUuid = VerifyUtil.isEmpty(protectedResource.getRootUuid())
                ? protectedResource.getEnvironment().getUuid()
                : protectedResource.getRootUuid();
        ProtectedEnvironment environment = resourceService.getResourceById(rootUuid)
            .filter(env -> env instanceof ProtectedEnvironment)
            .map(env -> (ProtectedEnvironment) env)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Resource not exists!"));
        List<ProtectedEnvironment> agents = k8sCommonService.getConnectiveInternalAgentByParams(environment, false);
        return agents.stream()
                .peek(agent -> log.info("dependent agent is :{}", agent.getUuid()))
                .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort()))
                .collect(Collectors.toList());
    }

    @Override
    public boolean applicable(String type) {
        return ResourceTypeEnum.KUBERNETES_COMMON.equalsType(type);
    }
}
