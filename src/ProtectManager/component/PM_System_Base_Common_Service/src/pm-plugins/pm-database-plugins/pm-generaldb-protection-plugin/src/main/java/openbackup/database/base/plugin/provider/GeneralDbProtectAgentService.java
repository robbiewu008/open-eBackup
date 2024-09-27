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
package openbackup.database.base.plugin.provider;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.util.GeneralDbUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.stream.Collectors;

/**
 * 通用数据库AgentSelector
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2022-12-28
 */
@Component
public class GeneralDbProtectAgentService {
    private final ResourceService resourceService;

    public GeneralDbProtectAgentService(ResourceService resourceService) {
        this.resourceService = resourceService;
    }

    /**
     * 查找endpoint
     *
     * @param protectedResource 资源
     * @return endpoint
     */
    public List<Endpoint> select(ProtectedResource protectedResource) {
        ProtectedResource currentResource = protectedResource;
        if (VerifyUtil.isEmpty(protectedResource.getDependencies())) {
            currentResource = resourceService.getResourceById(protectedResource.getUuid())
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "resource can not found"));
        }
        return GeneralDbUtil.getHosts(currentResource)
            .stream()
            .map(e -> new Endpoint(e.getUuid(), e.getEndpoint(), e.getPort()))
            .collect(Collectors.toList());
    }
}
