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
package openbackup.exchange.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.exchange.protection.access.service.ExchangeService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * exchange资源provider, 目前只实现资源删除前检查功能
 *
 */
@Slf4j
@Component
public class ExchangeResourceProvider implements ResourceProvider {
    @Autowired
    private ExchangeService exchangeService;

    private ResourceService resourceService;

    @Autowired
    public void setResourceService(ResourceService resourceService) {
        this.resourceService = resourceService;
    }

    /**
     * 只适用于exchange 单机或可用性组
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.EXCHANGE_SINGLE_NODE.getType().equals(object.getSubType())
            || ResourceSubTypeEnum.EXCHANGE_GROUP.getType().equals(object.getSubType());
    }

    /**
     * 检查受保护资源，创建逻辑资源（文件集，NAS共享）时调用该接口
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码<br/>
     * 回调函数中不允许对资源的UUID等关键字段进行修改。
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeCreate(ProtectedResource resource) {
    }

    /**
     * 检查受保护资源， 修改逻辑资源（文件集，NAS共享）时调用该接口
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码<br/>
     * 回调函数中不允许对资源的UUID等关键字段进行修改。
     * <p>
     * 提供的资源不包含dependency信息，如果应用需要补齐depen信息，请调用 “补充资源的dependency信息” 接口
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeUpdate(ProtectedResource resource) {
    }

    /**
     * 资源删除前的处理
     *
     * @param resource ProtectedResource
     * @return ResourceDeleteContext
     */
    @Override
    public ResourceDeleteContext preHandleDelete(ProtectedResource resource) {
        exchangeService.checkCanDeleteResource(resource);
        log.info("exchange resource {} pre handle delete check pass.", resource.getUuid());
        return ResourceDeleteContext.defaultValue();
    }

    @Override
    public boolean supplyDependency(ProtectedResource resource) {
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> agents = resourceService.queryDependencyResources(true, DatabaseConstants.AGENTS,
            Collections.singletonList(resource.getUuid()));
        dependencies.put(DatabaseConstants.AGENTS, agents);
        resource.setDependencies(dependencies);
        return true;
    }
}
