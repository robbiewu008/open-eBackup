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
package com.huawei.oceanprotect.k8s.protection.access.provider;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.provider.DefaultResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

/**
 * k8s NameSpace资源类provider
 *
 */
@Slf4j
@AllArgsConstructor
@Component
public class K8sNameSpaceResourceProvider extends DefaultResourceProvider {
    @Override
    public void cleanUnmodifiableFieldsWhenUpdate(ProtectedResource resource) {
        resource.setRootUuid(null);
        resource.setSubType(null);
        resource.setUserId(null);
        resource.setAuthorizedUser(null);
        resource.setParentUuid(null);
        resource.setCreatedTime(null);
        resource.setProtectionStatus(null);
    }

    @Override
    public boolean supplyDependency(ProtectedResource resource) {
        return true;
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.KUBERNETES_NAMESPACE_COMMON.equalsSubType(object.getSubType());
    }
}
