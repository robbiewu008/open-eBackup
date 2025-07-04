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

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.util.ResourceCheckContextUtil;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;

import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;

/**
 * The UnifiedConnectionCheckProvider
 *
 */
@Slf4j
@Component
public class UnifiedConnectionCheckProvider implements ResourceConnectionCheckProvider {
    private ProviderManager providerManager;

    private UnifiedResourceConnectionChecker unifiedResourceConnectionChecker;

    private PluginConfigManager pluginConfigManager;

    /**
     * 有参构造
     *
     * @param providerManager providerManager
     * @param unifiedResourceConnectionChecker unifiedResourceConnectionChecker
     * @param pluginConfigManager 插件配置管理者
     */
    public UnifiedConnectionCheckProvider(final ProviderManager providerManager,
        @Qualifier("unifiedResourceConnectionChecker")
        final UnifiedResourceConnectionChecker unifiedResourceConnectionChecker,
        final PluginConfigManager pluginConfigManager) {
        this.providerManager = providerManager;
        this.unifiedResourceConnectionChecker = unifiedResourceConnectionChecker;
        this.pluginConfigManager = pluginConfigManager;
    }

    @Override
    public ResourceCheckContext tryCheckConnection(ProtectedResource protectedResource,
        ProtectedResourceChecker protectedResourceChecker) {
        try {
            return doCheckConnection(protectedResource, protectedResourceChecker);
        } catch (LegoCheckedException e) {
            log.error("check connection failed. " + e.getMessage(), e);
            return buildErrorContext(e.getErrorCode(), e.getMessage(), e.getParameters());
        } catch (Throwable e) {
            log.error("check connection failed. " + e.getMessage(), e);
            return buildErrorContext(ResourceCheckContextUtil.UNION_ERROR, e.getMessage());
        }
    }

    private ResourceCheckContext doCheckConnection(ProtectedResource protectedResource,
        ProtectedResourceChecker protectedResourceChecker) {
        ProtectedResourceChecker resourceChecker = protectedResourceChecker;
        if (resourceChecker == null) {
            resourceChecker = providerManager.findProviderOrDefault(ProtectedResourceChecker.class, protectedResource,
                unifiedResourceConnectionChecker);
        }
        log.debug("ProtectedResourceChecker is {} by resourceId {}", resourceChecker.getClass(),
            protectedResource.getUuid());
        Map<ProtectedResource, List<ProtectedEnvironment>> resourceConnectableMap
            = resourceChecker.collectConnectableResources(protectedResource);
        if (resourceConnectableMap.isEmpty()) {
            log.warn("can not find resource and environment by connection config. resourceId: {}",
                protectedResource.getUuid());
        }
        log.info("Resource connectable map size is: {}, resource id is: {}.",
            resourceConnectableMap.size(), protectedResource.getUuid());

        List<CheckReport> checkReportList = new ArrayList<>();

        for (Map.Entry<ProtectedResource, List<ProtectedEnvironment>> entry : resourceConnectableMap.entrySet()) {
            ProtectedResource checkingResource = entry.getKey();

            List<CheckResult> checkResultList = new ArrayList<>();
            for (ProtectedEnvironment environment : entry.getValue()) {
                checkingResource.setEnvironment(environment);
                CheckResult checkResult = resourceChecker.generateCheckResult(checkingResource);
                checkResultList.add(checkResult);
            }

            CheckReport checkReport = new CheckReport(checkingResource, checkResultList);
            checkReportList.add(checkReport);
        }

        ResourceCheckContext context = new ResourceCheckContext();
        context.setResourceConnectableMap(resourceConnectableMap);
        List<ActionResult> actionResults = resourceChecker.collectActionResults(checkReportList, context.getContext());

        context.setActionResults(actionResults);
        return context;
    }

    @Override
    public boolean applicable(final ProtectedResource resource) {
        return pluginConfigManager.getPluginConfig(resource.getSubType()).isPresent();
    }

    private ResourceCheckContext buildErrorContext(Long errorCode, String message) {
        return buildErrorContext(errorCode, message, new String[0]);
    }

    private ResourceCheckContext buildErrorContext(Long errorCode, String message, String[] params) {
        ActionResult actionResult = new ActionResult(errorCode, message);
        if (!VerifyUtil.isEmpty(params)) {
            actionResult.setDetailParams(Arrays.asList(params));
        }
        actionResult.setBodyErr(errorCode.toString());
        ResourceCheckContext context = new ResourceCheckContext();
        List<ActionResult> actionResults = new ArrayList<>();
        actionResults.add(actionResult);
        context.setActionResults(actionResults);
        return context;
    }
}