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
package openbackup.informix.protection.access.provider.resource;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.informix.protection.access.service.InformixService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * Informix服务注册
 *
 * @author zwx951267
 * @version [DataBackup 1.5.0]
 * @since 2023-05-09
 */
@Component
@Slf4j
public class InformixProvider extends DatabaseEnvironmentProvider {
    private final InformixService informixService;

    private final JsonSchemaValidator jsonSchemaValidator;

    private final ResourceService resourceService;

    /**
     * InformixServiceProvider
     *
     * @param providerManager     provider manager
     * @param pluginConfigManager provider config manager
     * @param informixService     informixService
     * @param jsonSchemaValidator jsonSchemaValidator
     * @param resourceService resourceService
     */
    public InformixProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
                            InformixService informixService, JsonSchemaValidator jsonSchemaValidator,
                            ResourceService resourceService) {
        super(providerManager, pluginConfigManager);
        this.informixService = informixService;
        this.jsonSchemaValidator = jsonSchemaValidator;
        this.resourceService = resourceService;
    }

    /**
     * provider过滤器，过滤条件接口
     *
     * @param resourceSubType String 受保护资源类型
     * @return boolean true 获取该bean false过滤掉该bean
     */
    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.INFORMIX_SERVICE.getType().equals(resourceSubType);
    }

    /**
     * 注册集群服务时的check
     *
     * @param environment informix集群（服务）环境信息
     */
    @Override
    public void register(final ProtectedEnvironment environment) {
        log.info("Start to check the environment info, sub type: {}.", environment.getSubType());
        EnvironmentParamCheckUtil.checkEnvironmentNamePattern(environment.getName());
        jsonSchemaValidator.doValidate(environment, ResourceSubTypeEnum.INFORMIX_SERVICE.getType());
        // 校验主机信息
        informixService.checkHostInfo(environment);
        // 校验日志备份
        informixService.checkLogBackupItem(environment);
        // informix应用检查
        informixService.checkApplication(environment);
    }

    /**
     * 集群环境监控检查的时候，节点可用并且informix服务正常，即为集群在线
     * 抛出异常即可，框架自动去更新环境状态为离线
     *
     * @param environment 受保护资源
     */
    @Override
    public void validate(final ProtectedEnvironment environment) {
        log.info("Start to healthy check of environment, sub type: {}.", environment.getSubType());
        // 检查集群下的单实例状态
        checkSingleInstanceStatus(environment);
        // 检查集群注册的集群实例状态
        checkClusterInstanceStatus(environment);
        // 校验主机信息
        informixService.checkHostInfo(environment);
        // 更新环境信息
        updateProtectEnv(environment);
    }


    private void checkSingleInstanceStatus(ProtectedEnvironment environment) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.PARENT_UUID, environment.getUuid());
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.getType());
        PageListResponse<ProtectedResource> data;
        List<ProtectedResource> resources = new ArrayList<>();
        int pageNo = 0;
        do {
            data = resourceService.query(pageNo, IsmNumberConstant.HUNDRED, conditions);
            data.getRecords().forEach(item -> item.setEnvironment(environment));
            resources.addAll(data.getRecords());
            pageNo++;
        } while (data.getRecords().size() >= IsmNumberConstant.HUNDRED);
        resources.forEach(item -> {
            ResourceProvider provider = providerManager.findProvider(ResourceProvider.class, item, null);
            if (provider != null) {
                provider.healthCheck(item);
            }
        });
    }

    private void checkClusterInstanceStatus(ProtectedEnvironment environment) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.HOST_ID, environment.getUuid());
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.INFORMIX_CLUSTER_INSTANCE.getType());
        List<ProtectedResource> resources = new ArrayList<>();
        PageListResponse<ProtectedResource> result;
        int pageNo = 0;
        do {
            result = resourceService.query(pageNo, IsmNumberConstant.HUNDRED, conditions);
            resources.addAll(result.getRecords());
            pageNo++;
        } while (result.getRecords().size() >= IsmNumberConstant.HUNDRED);
        resources.stream()
                .map(instance -> informixService.getResourceById(instance.getParentUuid()))
                .collect(Collectors.toList())
                .forEach(item -> {
                    ResourceProvider provider = providerManager.findProvider(ResourceProvider.class, item, null);
                    if (provider != null) {
                        provider.healthCheck(item);
                    }
                });
    }

    private void updateProtectEnv(ProtectedEnvironment environment) {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(environment.getUuid());
        protectedEnvironment.setEndpoint(environment.getEndpoint());
        protectedEnvironment.setPort(environment.getPort());
        protectedEnvironment.setAuth(environment.getAuth());
        protectedEnvironment.setPath(environment.getPath());
        protectedEnvironment.setLinkStatus(environment.getLinkStatus());
        resourceService.updateSourceDirectly(Collections.singletonList(protectedEnvironment));
    }
}
