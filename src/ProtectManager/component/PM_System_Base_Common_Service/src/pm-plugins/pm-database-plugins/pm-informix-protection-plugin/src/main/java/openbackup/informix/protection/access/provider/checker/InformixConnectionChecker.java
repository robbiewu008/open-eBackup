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
package openbackup.informix.protection.access.provider.checker;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.informix.protection.access.constant.InformixConstant;
import openbackup.informix.protection.access.service.InformixService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import io.jsonwebtoken.lang.Collections;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Informix资源连通性检查器
 *
 * @author hwx1164326
 * @version [OceanProtect X8000 1.2.1]
 * @since 2023-08-31
 */
@Slf4j
@Component
public class InformixConnectionChecker extends UnifiedResourceConnectionChecker {
    private final InformixService informixService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     * @param informixService informix服务
     */
    public InformixConnectionChecker(ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
                AgentUnifiedService agentUnifiedService, InformixService informixService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.informixService = informixService;
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.INFORMIX_CLUSTER_INSTANCE.getType().equals(protectedResource.getSubType());
    }

    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(ProtectedResource resource) {
        log.info("Informix collect connectable resources, uuid: {}", resource.getUuid());
        Map<ProtectedResource, List<ProtectedEnvironment>> resourceListMap = new HashMap<>();
        List<ProtectedResource> children = resource.getDependencies().get(ResourceConstants.CHILDREN);
        // register check
        if (Collections.isEmpty(children)) {
            resourceListMap = super.collectConnectableResources(resource);
            return resourceListMap;
        }

        // connect check
        for (ProtectedResource child : children) {
            if (InformixConstant.MASTER_STATUS_LIST
                    .contains(child.getExtendInfo().get(InformixConstant.INSTANCESTATUS))) {
                ProtectedResource childResource = informixService.getResourceById(child.getUuid());
                resourceListMap = super.collectConnectableResources(childResource);
                break;
            }
        }
        return resourceListMap;
    }
}

