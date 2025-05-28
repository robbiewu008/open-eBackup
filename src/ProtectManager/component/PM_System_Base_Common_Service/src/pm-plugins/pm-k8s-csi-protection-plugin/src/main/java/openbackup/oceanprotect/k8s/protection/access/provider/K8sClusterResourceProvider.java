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
package openbackup.oceanprotect.k8s.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.oceanprotect.k8s.protection.access.service.K8sCommonService;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.springframework.stereotype.Component;

/**
 * 功能描述: K8sEnvironmentResourceProvider
 *
 */
@Slf4j
@AllArgsConstructor
@Component
public class K8sClusterResourceProvider implements ResourceProvider {
    private final K8sCommonService commonService;

    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.KUBERNETES_CLUSTER_COMMON.equalsSubType(object.getSubType());
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
        log.debug("resource provider default. no need execute before create");
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        log.debug("resource provider default. no need execute before create");
    }

    @Override
    public ResourceDeleteContext preHandleDelete(ProtectedResource resource) {
        ProtectedEnvironment environment = BeanTools.copy(resource, ProtectedEnvironment::new);
        commonService.deleteIpRule(environment);
        return ResourceDeleteContext.defaultValue();
    }

    @Override
    public boolean supplyDependency(ProtectedResource resource) {
        return true;
    }
}