/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.clickhouse.plugin.provider;

import openbackup.clickhouse.plugin.constant.ClickHouseConstant;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.provider.DatabaseResourceProvider;

import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

/**
 * ClickHouse资源供应器
 *
 * @author q00464130
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-27
 */
@Slf4j
@Component
public class ClickHouseResourceProvider extends DatabaseResourceProvider {
    /**
     * DatabaseResourceProvider
     *
     * @param providerManager provider manager
     * @param pluginConfigManager provider config manager
     */
    public ClickHouseResourceProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager) {
        super(providerManager, pluginConfigManager);
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.CLICK_HOUSE.equalsSubType(protectedResource.getSubType());
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
        ProtectedEnvironment cluster = resource.getEnvironment();
        resource.setPath(cluster != null ? cluster.getName() : resource.getName());
        if (cluster != null) {
            resource.setAuthorizedUser(cluster.getAuthorizedUser());
            resource.setUserId(cluster.getUserId());
        }
        if (StringUtils.equals(MapUtils.getString(resource.getExtendInfo(), ClickHouseConstant.TYPE),
            DatabaseConstants.CLUSTER_TARGET)) {
            log.info("cluster,beforeCreate,name:{}", resource.getName());
        } else {
            log.info("node,beforeCreate,name:{}", resource.getName());
        }
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        beforeCreate(resource);
    }
}
