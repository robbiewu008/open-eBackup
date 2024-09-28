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

import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.invoke.Invocation;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;

/**
 * Protected Resource Desensitize Service
 *
 */
@Component
public class ProtectedResourceDesensitizeService implements ProtectedResourceMonitor {
    private final JsonSchemaValidator jsonSchemaValidator;

    /**
     * constructor
     *
     * @param jsonSchemaValidator jsonSchemaValidator
     */
    public ProtectedResourceDesensitizeService(JsonSchemaValidator jsonSchemaValidator) {
        this.jsonSchemaValidator = jsonSchemaValidator;
    }

    /**
     * event type
     *
     * @return event type
     */
    @Override
    public List<String> getTypes() {
        return Collections.singletonList("desensitize");
    }

    /**
     * handle protected resource event
     *
     * @param invocation invocation
     * @param event event
     * @return result
     */
    @Override
    public Object invoke(Invocation<ProtectedResourceEvent, Object> invocation, ProtectedResourceEvent event) {
        ProtectedResource protectedResource = event.getResource();
        while (protectedResource != null) {
            desensitize(protectedResource);
            protectedResource = protectedResource.getEnvironment();
        }
        return invocation.invoke(event);
    }

    private void desensitize(ProtectedResource protectedResource) {
        List<String> fields = jsonSchemaValidator.getEditableFields(protectedResource.getSubType());
        for (String field : fields) {
            protectedResource.setExtendInfoByKey(field, null);
        }
        Authentication authentication = protectedResource.getAuth();
        if (authentication != null) {
            authentication.setAuthPwd(null);
            authentication.setExtendInfo(null);
        }
    }
}
