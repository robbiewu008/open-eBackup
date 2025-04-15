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