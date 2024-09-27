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
package openbackup.access.framework.resource.service.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceScanProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import org.springframework.beans.factory.annotation.Autowired;

import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * 描述
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-14
 */
public abstract class AbstractResourceScanProvider implements ResourceScanProvider {
    @Autowired
    private ResourceService resourceService;

    @Override
    public List<ProtectedResource> queryResourceWhenException(String uuid) {
        Set<String> resourceUuids = resourceService.queryRelatedResourceUuids(uuid, null);
        resourceUuids.remove(uuid);
        if (resourceUuids.isEmpty()) {
            return Collections.emptyList();
        }
        return resourceUuids.stream().map(e -> {
            ProtectedResource resource = new ProtectedResource();
            resource.setUuid(e);
            return resource;
        }).collect(Collectors.toList());
    }
}
