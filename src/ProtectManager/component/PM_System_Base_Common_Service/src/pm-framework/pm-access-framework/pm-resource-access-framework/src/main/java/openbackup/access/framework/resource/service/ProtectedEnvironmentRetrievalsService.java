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

import openbackup.access.framework.resource.util.ResourceUtil;
import openbackup.data.protection.access.provider.sdk.plugin.CollectableConfig;
import openbackup.data.protection.access.provider.sdk.plugin.PluginExtensionInvokeContext;
import openbackup.data.protection.access.provider.sdk.plugin.ResourceExtensionManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceBase;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.EmeiStorDefaultExceptionHandler;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Queue;
import java.util.Set;
import java.util.function.Function;
import java.util.stream.Collectors;

/**
 * The ProtectedEnvironmentRetrievalService
 *
 */
@Slf4j
@Component
public class ProtectedEnvironmentRetrievalsService {
    private final ResourceExtensionManager resourceExtensionManager;
    private final ResourceService resourceService;
    private final ProtectedEnvironmentService environmentService;

    private final String dependencies = "dependencies";
    private final Set<String> resourceLogKeys = new HashSet<>(Arrays.asList("name", "uuid", dependencies));


    /**
     * 有参构造
     *
     * @param resourceExtensionManager 插件配置管理
     * @param resourceService 资源服务
     * @param protectedEnvironmentService 环境服务
     */
    public ProtectedEnvironmentRetrievalsService(final ResourceExtensionManager resourceExtensionManager,
        final ResourceService resourceService, final ProtectedEnvironmentService protectedEnvironmentService) {
        this.resourceExtensionManager = resourceExtensionManager;
        this.resourceService = resourceService;
        this.environmentService = protectedEnvironmentService;
    }

    /**
     * 解析出可连接的资源和环境矩阵信息
     *
     * @param resourceId resourceId
     * @return 资源和环境矩阵信息
     */
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(String resourceId) {
        Optional<ProtectedResource> resourceOptional = resourceService.getResourceByIdIgnoreOwner(resourceId);
        if (!resourceOptional.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "can not find resource by id: " + resourceId);
        }
        return collectConnectableResources(resourceOptional.get());
    }

    /**
     * 解析出可连接的资源和环境矩阵信息
     * 解析结构依据传入的resource结构
     *
     * @param resource 资源
     * @return 资源和环境矩阵信息
     */
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(ProtectedResource resource) {
        List<CollectableConfig> collectableConfigs = resourceExtensionManager.invoke(resource.getSubType(),
            "functions.connection.dependency", new PluginExtensionInvokeContext<Object, List<CollectableConfig>>() {
            });
        if (collectableConfigs.isEmpty()) {
            log.warn("can not find connection config. resource subType: {}", resource.getSubType());
            return new HashMap<>();
        }
        logProtectResourceDependencyStructure(resource);
        Map<String, ProtectedResource> protectedResourceMap = generateAllDependencyResourcesMap(resource);

        Map<ProtectedResource, List<ProtectedEnvironment>> resourceEnvironmentMap = new HashMap<>();
        Map<ProtectedResource, ProtectedResource> resourceCache = new HashMap<>();

        for (CollectableConfig collectableConfig : collectableConfigs) {
            String[] resourcePaths = Optional.ofNullable(collectableConfig.getResource()).orElse("").split("\\.");
            String[] environmentPaths = Optional.ofNullable(collectableConfig.getEnvironment())
                .map(str -> str.split("\\."))
                .orElse(new String[0]);
            List<ProtectedResource> protectedResources = searchProtectedResources(resourceCache, protectedResourceMap,
                Collections.singletonList(resource), resourcePaths, 0);
            if (protectedResources.isEmpty()) {
                log.warn("can not find resource by config. resourceId: {}, configResource: {}", resource.getUuid(),
                    collectableConfig.getResource());
                continue;
            }
            for (ProtectedResource protectedResource : protectedResources) {
                List<ProtectedEnvironment> environments = getSearchProtectedEnvironments(resourceCache,
                    protectedResourceMap, protectedResource, environmentPaths);
                if (environments.isEmpty()) {
                    log.warn("can not find environment by config. resourceId: {}, configEnvironment: {}",
                        protectedResource.getUuid(), collectableConfig.getEnvironment());
                    continue;
                }
                log.debug("find enviroment({}) by resource: {}",
                    environments.stream().map(ResourceBase::getUuid).collect(Collectors.toList()),
                    protectedResource.getUuid());
                List<ProtectedEnvironment> envFromMap = resourceEnvironmentMap.computeIfAbsent(protectedResource,
                    e -> new ArrayList<>());
                envFromMap.addAll(environments);
            }
        }
        return resourceEnvironmentMap;
    }

    private Map<String, ProtectedResource> generateAllDependencyResourcesMap(ProtectedResource resource) {
        List<String> uuids = generateAllProtectedResourceUuids(resource);
        if (uuids.isEmpty()) {
            return new HashMap<>();
        }
        return resourceService.query(0, uuids.size(), Collections.singletonMap("uuid", uuids))
            .getRecords()
            .stream()
            .collect(Collectors.toMap(ProtectedResource::getUuid, Function.identity()));
    }

    private List<String> generateAllProtectedResourceUuids(ProtectedResource resource) {
        if (resource.getDependencies() == null || resource.getDependencies().isEmpty()) {
            return Collections.emptyList();
        }

        List<ProtectedResource> nextLevelProtectedResources = resource.getDependencies()
            .values()
            .stream()
            .flatMap(List::stream)
            .collect(Collectors.toList());

        List<String> uuids = nextLevelProtectedResources.stream()
            .map(ProtectedResource::getUuid)
            .filter(Objects::nonNull)
            .collect(Collectors.toList());

        nextLevelProtectedResources.stream().map(this::generateAllProtectedResourceUuids).forEach(uuids::addAll);
        return uuids;
    }

    private List<ProtectedEnvironment> getSearchProtectedEnvironments(
        Map<ProtectedResource, ProtectedResource> resourceCache, Map<String, ProtectedResource> protectedResourceMap,
        ProtectedResource resource, String[] searchPath) {
        if (searchPath.length == 0) {
            ProtectedEnvironment environment = resource.getEnvironment();
            if (environment == null) {
                environment = environmentService.getEnvironmentById(resource.getRootUuid());
            }
            return Collections.singletonList(environment);
        }
        return searchProtectedResources(resourceCache, protectedResourceMap, Collections.singletonList(resource),
            searchPath, 0).stream()
            .filter(ProtectedEnvironment.class::isInstance)
            .map(ProtectedEnvironment.class::cast)
            .collect(Collectors.toList());
    }

    private List<ProtectedResource> searchProtectedResources(Map<ProtectedResource, ProtectedResource> resourceCache,
        Map<String, ProtectedResource> protectedResourceMap, List<ProtectedResource> protectedResources, String[] paths,
        int level) {
        List<ProtectedResource> protectedResourceList = new ArrayList<>();
        if (level == paths.length) {
            protectedResources.stream()
                .map(resource -> getAvailableProtectedResource(resourceCache, protectedResourceMap, resource))
                .forEach(protectedResourceList::add);
            return protectedResourceList;
        }
        String path = paths[level];
        for (ProtectedResource protectedResource : protectedResources) {
            ProtectedResource resource = getAvailableProtectedResource(resourceCache, protectedResourceMap,
                protectedResource);
            List<ProtectedResource> nextProtectedResources = StringUtils.isNotEmpty(path) ? generateProtectedResources(
                resourceCache, protectedResourceMap, path, resource) : Collections.singletonList(resource);
            protectedResourceList.addAll(
                searchProtectedResources(resourceCache, protectedResourceMap, nextProtectedResources, paths,
                    level + 1));
        }
        return protectedResourceList;
    }

    private List<ProtectedResource> generateProtectedResources(Map<ProtectedResource, ProtectedResource> resourceCache,
        Map<String, ProtectedResource> protectedResourceMap, String path, ProtectedResource resource) {
        List<ProtectedResource> dependencyResources = Optional.ofNullable(resource.getDependencies())
            .orElseGet(HashMap::new)
            .get(path);
        if (Objects.isNull(dependencyResources)) {
            return Collections.emptyList();
        }
        return dependencyResources.stream()
            .map(dependencyResource -> getAvailableProtectedResource(resourceCache, protectedResourceMap,
                dependencyResource))
            .collect(Collectors.toList());
    }

    private ProtectedResource getAvailableProtectedResource(Map<ProtectedResource, ProtectedResource> resourceCache,
        Map<String, ProtectedResource> protectedResourceMap, ProtectedResource protectedResource) {
        if (resourceCache.containsKey(protectedResource)) {
            return resourceCache.get(protectedResource);
        }
        ProtectedResource resource = protectedResource;
        if (StringUtils.isNotEmpty(resource.getUuid())) {
            ProtectedResource protectedResourceInDb = protectedResourceMap.get(resource.getUuid());
            if (protectedResourceInDb != null) {
                resource = ResourceUtil.combineProtectedResource(protectedResource, protectedResourceInDb);
            }
        }
        resourceCache.put(protectedResource, resource);
        return resource;
    }

    @SuppressWarnings({"rawtypes"})
    private void logProtectResourceDependencyStructure(ProtectedResource resource) {
        try {
            Map resourceMap = JSONObject.toBean(JSONObject.stringify(resource), Map.class);
            resolveResourceMapLog(resourceMap);
            String resourceStructure = JSONObject.stringify(resourceMap);
            log.info("retrieval resource structure: {}", resourceStructure);
        } catch (LegoCheckedException | EmeiStorDefaultExceptionHandler e) {
            log.warn("log retrieval resource structure occurs error. " + ExceptionUtil.getErrorMessage(e));
        } finally {
            if (log.isDebugEnabled()) {
                if (resource.getUuid() != null) {
                    log.debug("log retrieval resource structure end. resource uuid is {}", resource.getUuid());
                } else {
                    log.debug("log retrieval resource structure end. resource name is {}", resource.getName());
                }
            }
        }
    }

    @SuppressWarnings({"rawtypes"})
    private void resolveResourceMapLog(Map resourceMap) {
        Queue<Object> queue = new LinkedList<>();
        queue.offer(resourceMap);
        while (queue.size() > 0) {
            Object poll = queue.poll();
            if (!(poll instanceof Map)) {
                continue;
            }
            Map pollMap = (Map) poll;
            Iterator keyIter = pollMap.keySet().iterator();
            while (keyIter.hasNext()) {
                String key = keyIter.next().toString();
                // 只保留简易信息字段
                if (!resourceLogKeys.contains(key)) {
                    keyIter.remove();
                    continue;
                }
                if (dependencies.equals(key) && pollMap.get(key) instanceof Map) {
                    resolveProtectResoruceMapByDependencyMap((Map) pollMap.get(key), queue);
                }
            }
        }
    }

    @SuppressWarnings({"rawtypes"})
    private void resolveProtectResoruceMapByDependencyMap(Map dependencyMap, Queue<Object> queue) {
        Collection dependencyValues = dependencyMap.values();
        for (Object dependencyValue : dependencyValues) {
            if (dependencyValue instanceof List) {
                for (Object resource : (List) dependencyValue) {
                    queue.offer(resource);
                }
            }
        }
    }
}
