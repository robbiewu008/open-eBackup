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
package openbackup.access.framework.resource.service;

import openbackup.access.framework.resource.service.proxy.ProxyFactory;
import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.invoke.Invocation;
import openbackup.system.base.util.SensitiveValidateUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.context.ApplicationContext;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;
import java.util.stream.Collectors;

/**
 * Protected Resource Watch Service
 *
 * @author l00272247
 * @since 2021-10-19
 */
@Component
@Slf4j
public class ProtectedResourceWatchService implements ProtectedResourceMonitor {
    private final ApplicationContext applicationContext;
    private final ProviderManager providerManager;
    private final JsonSchemaValidator jsonSchemaValidator;

    /**
     * constructor
     *
     * @param applicationContext resourceService
     * @param providerManager providerManager
     * @param jsonSchemaValidator jsonSchemaValidator
     */
    public ProtectedResourceWatchService(
            ApplicationContext applicationContext,
            ProviderManager providerManager,
            JsonSchemaValidator jsonSchemaValidator) {
        this.applicationContext = applicationContext;
        this.providerManager = providerManager;
        this.jsonSchemaValidator = jsonSchemaValidator;
    }

    /**
     * event type
     *
     * @return event type
     */
    @Override
    public List<String> getTypes() {
        return Arrays.asList("create", "update", "check");
    }

    /**
     * invoke method
     *
     * @param invocation invocation
     * @param event event
     * @return result
     */
    @Override
    public Object invoke(Invocation<ProtectedResourceEvent, Object> invocation, ProtectedResourceEvent event) {
        ResourceService resourceService = applicationContext.getBean(ResourceService.class);
        // replenishEnvironment方法返回对象可能是克隆对象，将event.getResource()最为首对象，用于回调赋值。
        // 针对有dependency依赖的资源，框架目前不去主动查询依赖并补齐，避免耗时严重
        List<Object> resources =
                Arrays.asList(
                    event.getResource(),
                    resourceService.replenishEnvironment(event.getResource()),
                    resourceService
                            .getBasicResourceById(event.getResource().getUuid())
                            .map(resourceService::replenishEnvironment)
                            .orElse(null)
                );
        Class<? extends ProtectedResource> clazz = event.getResource().getClass();
        ProtectedResource protectedResource = ProxyFactory.get(clazz).create(resources);
        callHookMethod(event, protectedResource);
        Object result = invocation.invoke(event);
        ProtectedResource resource =
                resourceService.getResourceById(protectedResource.getUuid()).orElse(protectedResource);
        String[] typeArr = new String[]{
                resource.getSubType(),
                resource.getSubType() + "_" + resource.getType(),
                resource.getType() + "_" + resource.getSubType()};
        jsonSchemaValidator.doValidate(resource, typeArr);
        return result;
    }

    private void doExtendInfoSensetiveValidate(ProtectedResource resource) {
        if (VerifyUtil.isEmpty(resource.getExtendInfo())) {
            return ;
        }
        List<String> allfields = resource.getExtendInfo().keySet().stream().collect(Collectors.toList());
        List<String> fieldsFromJsonSchema = jsonSchemaValidator.getSecretFields(resource.getSubType());
        List<String> otherFields = allfields.stream()
                .filter(item -> !fieldsFromJsonSchema.contains(item)).collect(Collectors.toList());
        SensitiveValidateUtil.doValidate(otherFields);
    }

    private void callHookMethod(ProtectedResourceEvent event, ProtectedResource resource) {
        Collection<ResourceProvider> providers =
                providerManager.findProviders(ResourceProvider.class).stream()
                        .filter(provider -> provider.applicable(resource))
                        .collect(Collectors.toList());
        String type = event.getType();
        for (ResourceProvider provider : providers) {
            Consumer<ProtectedResource> consumer;
            if ("create".equals(type)) {
                consumer = provider::beforeCreate;
            } else if ("update".equals(type)) {
                consumer = provider::beforeUpdate;
            } else {
                continue;
            }
            invoke(type, resource, consumer, provider);
        }
    }

    private void invoke(
            String type, ProtectedResource resource, Consumer<ProtectedResource> consumer, ResourceProvider provider) {
        long startTime = System.currentTimeMillis();
        try {
            consumer.accept(resource);
        } finally {
            long endTime = System.currentTimeMillis();
            long timeCost = endTime - startTime;
            if (timeCost > TimeUnit.SECONDS.toMillis(1)) {
                log.warn("{} process {} event, time cost is {}ms", provider.getClass().getName(), type, timeCost);
            }
        }
    }
}
