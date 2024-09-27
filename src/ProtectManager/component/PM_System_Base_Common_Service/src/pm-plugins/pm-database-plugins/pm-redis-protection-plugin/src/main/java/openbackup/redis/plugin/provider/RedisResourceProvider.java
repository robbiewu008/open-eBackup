/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.redis.plugin.provider;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.provider.DatabaseResourceProvider;
import openbackup.redis.plugin.constant.RedisConstant;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

/**
 * The RedisResourceProvider
 *
 * @author x30028756
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-14
 */
@Slf4j
@Component
public class RedisResourceProvider extends DatabaseResourceProvider {
    /**
     * Constructor
     *
     * @param providerManager providerManager
     * @param pluginConfigManager pluginConfigManager
     */
    public RedisResourceProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager) {
        super(providerManager, pluginConfigManager);
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.REDIS.getType().equals(protectedResource.getSubType());
    }

    @Override
    public void beforeCreate(ProtectedResource protectedResource) {
        if (StringUtils.equals(MapUtils.getString(protectedResource.getExtendInfo(), RedisConstant.TYPE),
            DatabaseConstants.CLUSTER_TARGET)) {
            log.info("cluster,beforeCreate,name:{}", protectedResource.getName());
        } else {
            log.info("node,beforeCreate,name:{}", protectedResource.getName());
        }
    }

    @Override
    public void beforeUpdate(ProtectedResource protectedResource) {
        beforeCreate(protectedResource);
    }
}