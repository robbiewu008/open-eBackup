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
package openbackup.sqlserver.resources.access.provider;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.provider.DefaultResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.sqlserver.common.SqlServerConstants;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * SQL Server AlwaysOnProvider
 *
 */
@Component
@Slf4j
public class SqlServerAlwaysOnProvider extends DefaultResourceProvider {
    private ResourceService resourceService;

    @Autowired
    public void setResourceService(ResourceService resourceService) {
        this.resourceService = resourceService;
    }

    @Override
    public void cleanUnmodifiableFieldsWhenUpdate(ProtectedResource resource) {
        resource.setRootUuid(null);
        resource.setSubType(null);
        resource.setUserId(null);
        resource.setAuthorizedUser(null);
        resource.setParentUuid(null);
        resource.setCreatedTime(null);
        resource.setProtectionStatus(null);
    }

    @Override
    public boolean supplyDependency(ProtectedResource resource) {
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> databases = resourceService.queryDependencyResources(true, SqlServerConstants.DATABASE,
            Collections.singletonList(resource.getUuid()));
        dependencies.put(SqlServerConstants.DATABASE, databases);
        List<ProtectedResource> instances = resourceService.queryDependencyResources(true, DatabaseConstants.INSTANCE,
            Collections.singletonList(resource.getUuid()));
        dependencies.put(DatabaseConstants.INSTANCE, instances);
        resource.setDependencies(dependencies);
        return true;
    }

    /**
     * provider过滤器，过滤条件接口
     *
     * @param object object 受保护资源
     * @return boolean true 获取该bean false过滤掉该bean
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && Objects.equals(object.getSubType(),
            ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType());
    }
}
