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
