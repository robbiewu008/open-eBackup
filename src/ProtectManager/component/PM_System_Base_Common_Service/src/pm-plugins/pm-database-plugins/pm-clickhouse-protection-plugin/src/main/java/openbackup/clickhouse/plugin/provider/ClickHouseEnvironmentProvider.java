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
package openbackup.clickhouse.plugin.provider;

import lombok.extern.slf4j.Slf4j;
import openbackup.clickhouse.plugin.service.ClickHouseService;
import openbackup.clickhouse.plugin.util.ClickHouseValidator;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Optional;

/**
 * ClickHouse环境供应器
 *
 */
@Slf4j
@Component
public class ClickHouseEnvironmentProvider extends DatabaseEnvironmentProvider {
    private final ClickHouseService clickHouseService;

    /**
     * 构造方法
     *
     * @param providerManager providerManager
     * @param pluginConfigManager pluginConfigManager
     * @param clickHouseService clickHouseService
     */
    public ClickHouseEnvironmentProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        ClickHouseService clickHouseService) {
        super(providerManager, pluginConfigManager);
        this.clickHouseService = clickHouseService;
    }

    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        log.info("scan dataBases of ClickHouse cluster, environment name: {}", environment.getName());
        return clickHouseService.scanDataBases(environment);
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.CLICK_HOUSE.equalsSubType(resourceSubType);
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("check ClickHouse cluster, environment name: {}", environment.getName());
        ClickHouseValidator.checkClickHouseCluster(environment);
        clickHouseService.preCheck(environment);
        Optional<String> status = clickHouseService.healthCheckWithResultStatus(environment, false);
        environment.setLinkStatus(status.orElseGet(() -> LinkStatusEnum.ONLINE.getStatus().toString()));
        environment.setUuid(UUIDGenerator.getUUID());
    }

    @Override
    public PageListResponse<ProtectedResource> browse(ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions environmentConditions) {
        log.info("browse tables of cluster: {} and database: {}", environment.getUuid(),
            environmentConditions.getParentId());
        return clickHouseService.browseTables(environment, environmentConditions);
    }

    @Override
    public Optional<String> healthCheckWithResultStatus(ProtectedEnvironment environment) {
        log.info("health check of ClickHouse cluster: {}", environment.getName());
        return clickHouseService.healthCheckWithResultStatus(environment, true);
    }
}
