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
package openbackup.antdb.protection.access.service.impl;

import openbackup.antdb.protection.access.service.AntDBInstanceService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.service.InstanceProtectionService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Service;

import java.util.List;
import java.util.stream.Collectors;

/**
 * AntDB实例服务
 *
 */
@Service
public class AntDBInstanceServiceImpl implements AntDBInstanceService {
    private final ResourceService resourceService;

    private final InstanceProtectionService instanceProtectionService;

    public AntDBInstanceServiceImpl(ResourceService resourceService,
        InstanceProtectionService instanceProtectionService) {
        this.resourceService = resourceService;
        this.instanceProtectionService = instanceProtectionService;
    }

    @Override
    public ProtectedResource getResourceById(String resourceId) {
        return resourceService.getResourceById(resourceId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                "Protected resource not exist."));
    }

    @Override
    public List<TaskEnvironment> getEnvNodesByInstanceResource(ProtectedResource instanceResource) {
        if (ResourceSubTypeEnum.ANT_DB_INSTANCE.equalsSubType(instanceResource.getSubType())) {
            return instanceProtectionService.extractEnvNodesBySingleInstance(instanceResource);
        }
        return instanceProtectionService.extractEnvNodesByClusterInstance(instanceResource);
    }

    @Override
    public List<Endpoint> getAgentsByInstanceResource(ProtectedResource instanceResource) {
        List<TaskEnvironment> nodeList = getEnvNodesByInstanceResource(instanceResource);
        return nodeList.stream()
            .map(node -> new Endpoint(node.getUuid(), node.getEndpoint(), node.getPort()))
            .collect(Collectors.toList());
    }
}
