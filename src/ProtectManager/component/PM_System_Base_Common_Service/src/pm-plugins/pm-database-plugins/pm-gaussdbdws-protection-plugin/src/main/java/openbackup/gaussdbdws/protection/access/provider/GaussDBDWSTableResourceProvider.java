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
package openbackup.gaussdbdws.protection.access.provider;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.database.base.plugin.provider.DatabaseResourceProvider;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.gaussdbdws.protection.access.service.GaussDBBaseService;
import openbackup.gaussdbdws.protection.access.util.DwsValidator;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.stream.Collectors;

/**
 * GaussDBDWS Table资源相关接口的具体实现类
 * 实现了：健康状态检查，环境信息检查相关等接口
 *
 */
@Slf4j
@Component
public class GaussDBDWSTableResourceProvider extends DatabaseResourceProvider {
    private final GaussDBBaseService gaussDBBaseService;

    /**
     * Constructor
     *
     * @param providerManager providerManager
     * @param pluginConfigManager pluginConfigManager
     * @param gaussDBBaseService DWS 应用基本的Service
     */
    public GaussDBDWSTableResourceProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        GaussDBBaseService gaussDBBaseService) {
        super(providerManager, pluginConfigManager);
        this.gaussDBBaseService = gaussDBBaseService;
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType().equals(protectedResource.getSubType());
    }

    /**
     * 创建Table备份集前检查
     *
     * @param resource 资源
     */
    @Override
    public void beforeCreate(ProtectedResource resource) {
        // 检验备份集rootUuid是否存在
        DwsValidator.checkDwsValue(resource.getRootUuid());

        // 检验备份集名称是否合法
        DwsValidator.checkDwsNameFormat(resource.getName());

        // 检查 添加的table对象是否大于 1024个;
        String tableInfo = resource.getExtendInfo().get(DwsConstant.EXTEND_INFO_KEY_TABLE);
        String[] tableInfos = tableInfo.split(",");
        DwsValidator.checkDwsExistSameSchemaName(Arrays.stream(tableInfos).collect(Collectors.toList()));
        DwsValidator.checkDwsExistName(Arrays.stream(tableInfos).collect(Collectors.toList()));
        DwsValidator.checkDwsNoExistName(tableInfos, ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType());
        DwsValidator.checkDwsExistSameDatabaseName(tableInfos);
        // 检查 DWS 集群的状态是否为可用的在线状态;
        gaussDBBaseService.checkClusterLinkStatus(resource.getRootUuid());

        // 添加对象信息位置
        resource.setPath(gaussDBBaseService.getResourceById(resource.getRootUuid()).getPath());

        // 检查 添加的schema和数据库中的table集(过滤掉相同的table集对象)中table是否相同;
        gaussDBBaseService.checkSameTableInfo(resource, tableInfo, ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType());
    }

    /**
     * 创建Table备份集前检查
     *
     * @param resource 资源
     */
    @Override
    public void beforeUpdate(ProtectedResource resource) {
        beforeCreate(resource);
    }

    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = super.getResourceFeature();
        resourceFeature.setSupportedLanFree(false);
        return resourceFeature;
    }
}
