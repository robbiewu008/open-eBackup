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

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceScanProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.utils.DatabaseScannerUtils;
import openbackup.sqlserver.common.SqlServerConstants;
import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.ObjectUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 集群环境健康检查
 *
 */
@Component
@Slf4j
public class SqlServerEnvironmentProvider implements ResourceScanProvider {
    private final ResourceService resourceService;

    private final SqlServerBaseService sqlServerBaseService;

    /**
     * SqlServerEnvironmentProvider 构造器
     *
     * @param resourceService 资源服务
     * @param sqlServerBaseService 扫描数据库
     */
    public SqlServerEnvironmentProvider(ResourceService resourceService, SqlServerBaseService sqlServerBaseService) {
        this.resourceService = resourceService;
        this.sqlServerBaseService = sqlServerBaseService;
    }

    /**
     * 扫描受保护环境， 可选实现。根据受保护保护环境决定是否实现该接口，
     * 如受保护的资源需要在ProtectManager进行持久化，则需实现，如VMware虚拟化环境；
     * 如不需要将资源在ProtectManager中实现，则无须实现，比如HDFS的目录、文件，HBase的命名空间，表等。
     *
     * @param environment 受保护环境(集群环境)
     * @return 受保护环境中的资源列表
     */
    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        return scanDatabases(environment);
    }

    /**
     * 扫描环境下数据库信息
     *
     * @param environment environment
     * @return 数据库列表
     */
    public List<ProtectedResource> scanDatabases(ProtectedEnvironment environment) {
        List<ProtectedResource> instances = DatabaseScannerUtils.getInstancesByEnvironment(environment.getUuid(),
            ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType(), resourceService);
        List<ProtectedResource> resources = new ArrayList<>(instances);
        instances.forEach(instance -> {
            instance.setEnvironment(environment);
            List<ProtectedResource> resourceList = sqlServerBaseService.getDatabaseInfoByAgent(environment,
                environment.getEndpoint(), environment.getPort(), instance,
                ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType());
            if (ObjectUtils.isEmpty(resourceList)) {
                log.info("[SQL Server] The instance {} does not have databases.", instance.getUuid());
                return;
            }
            resourceList.forEach(resource -> addProtectedResourceByAppResource(instance, resource, resources));
            resourceList.stream()
                .findFirst()
                .ifPresent(db -> instance.setVersion(
                    Optional.ofNullable(db.getExtendInfoByKey(DatabaseConstants.VERSION)).orElse(StringUtils.EMPTY)));
        });
        return resources;
    }

    /**
     * 将Agent返回数据转换为资源
     *
     * @param instance 实例信息
     * @param resource 返回数据
     * @param resources 资源列表
     */
    public void addProtectedResourceByAppResource(ProtectedResource instance, ProtectedResource resource,
        List<ProtectedResource> resources) {
        Map<String, String> databaseMap = getDatabaseMapByLocal(instance);
        log.info("[SQL Server] database: {} convert to ProtectedResource", resource.getName());
        ProtectedResource copy = BeanTools.copy(resource, ProtectedResource::new);
        String databaseId = resource.getExtendInfo().get(DatabaseConstants.DATABASE_ID);
        if (VerifyUtil.isEmpty(databaseId)) {
            log.error("[SQL Server] database: {} don't have id", resource.getName());
            return;
        }
        copy.setUuid(databaseMap.getOrDefault(databaseId, StringUtils.EMPTY));
        copy.setParentUuid(instance.getUuid());
        copy.setParentName(instance.getName());
        copy.setPath(copy.getName() + SqlServerConstants.RESOURCE_NAME_SPLIT + instance.getName()
            + SqlServerConstants.RESOURCE_NAME_SPLIT + instance.getEnvironment().getName());
        resources.add(copy);
    }

    private Map<String, String> getDatabaseMapByLocal(ProtectedResource instance) {
        // 扫描数据库
        Map<String, Object> databaseCondition = new HashMap<>();
        databaseCondition.put(DatabaseConstants.PARENT_UUID, instance.getUuid());
        databaseCondition.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType());
        return resourceService.queryAllResources(databaseCondition)
            .stream()
            .collect(Collectors.toMap(db -> db.getExtendInfo().get(DatabaseConstants.DATABASE_ID),
                ProtectedResource::getUuid));
    }

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(ProtectedEnvironment object) {
        return ResourceSubTypeEnum.U_BACKUP_AGENT.equalsSubType(object.getSubType());
    }
}
