/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.access.framework.resource.service.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;

import java.util.List;
import java.util.Map;

/**
 * The AbstractResourceChecker
 *
 * @author g30003063
 * @since 2022-05-28
 */
public abstract class AbstractResourceChecker<T> implements ProtectedResourceChecker<T> {
    /**
     * 插件配置管理
     */
    protected final ProtectedEnvironmentRetrievalsService environmentRetrievalsService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境检索服务
     */
    public AbstractResourceChecker(final ProtectedEnvironmentRetrievalsService environmentRetrievalsService) {
        this.environmentRetrievalsService = environmentRetrievalsService;
    }

    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(ProtectedResource resource) {
        return environmentRetrievalsService.collectConnectableResources(resource);
    }
}