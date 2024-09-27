/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
 * @author l00272247
 * @since 2021-10-19
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
