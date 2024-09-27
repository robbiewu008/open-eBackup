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

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.provider.DatabaseResourceProvider;
import openbackup.sqlserver.common.SqlServerConstants;
import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;

/**
 * SQL Server单实例注册provider
 *
 * @author xWX1016404
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-20
 */
@Component
@Slf4j
public class SqlServerInstanceProvider extends DatabaseResourceProvider {
    private final ResourceService resourceService;

    private final SqlServerBaseService sqlServerBaseService;

    /**
     * DatabaseResourceProvider 构造器注入
     *
     * @param providerManager provider管理器，获取bean和过滤bean
     * @param pluginConfigManager 配置文件管理器
     * @param resourceService 资源服务接口，查询数据用
     * @param sqlServerBaseService sqlserver资源注册公共方法类
     */
    public SqlServerInstanceProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        ResourceService resourceService, SqlServerBaseService sqlServerBaseService) {
        super(providerManager, pluginConfigManager);
        this.resourceService = resourceService;
        this.sqlServerBaseService = sqlServerBaseService;
    }

    /**
     * 资源重名检查配置
     *
     * @return SQL Server实例注册重名检查配置
     */
    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = new ResourceFeature();

        // SQL Server实例名称可以重复，不检查实例重名
        resourceFeature.setShouldCheckResourceNameDuplicate(false);
        // SQL Server集群实例扫描不需要刷新主机信息
        resourceFeature.setShouldUpdateDependencyHostInfoWhenScan(false);
        return resourceFeature;
    }

    /**
     * 检查SQL Server实例资源联通行
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeCreate(ProtectedResource resource) {
        checkAndSetResourceParam(resource);

        // 校验是否已经存在实例
        sqlServerBaseService.checkInstanceExist(resource);
        checkConnect(resource);
    }

    private void checkAndSetResourceParam(ProtectedResource resource) {
        Map<String, List<ProtectedResource>> dependencies = resource.getDependencies();
        ProtectedResource protectedResource = Optional.ofNullable(dependencies)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ERR_PARAM, "resource param is not enough."))
            .get(DatabaseConstants.AGENTS)
            .get(0);
        if (protectedResource == null || resource.getExtendInfo() == null) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "resource param is not enough.");
        }

        // 设置实例参数
        setInstanceParams(resource, protectedResource.getUuid());
    }

    private void checkConnect(ProtectedResource resource) {
        // 校验联通性
        providerManager.findProvider(ResourceConnectionCheckProvider.class, resource).checkConnection(resource);
    }

    private void setInstanceParams(ProtectedResource resource, String hostId) {
        resource.setPath(resource.getName() + SqlServerConstants.RESOURCE_NAME_SPLIT
            + resourceService.getResourceById(hostId).orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "resource is not exist")).getName());
    }

    /**
     * 检查受保护资源SQL Server实例修改联通性检查
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeUpdate(ProtectedResource resource) {
        checkConnect(resource);
        setInstanceParams(resource, resource.getParentUuid());
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
            ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType());
    }
}
