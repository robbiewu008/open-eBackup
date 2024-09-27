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
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.invoke.Invocation;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * Protected Resource Encryptor
 *
 * @author l00272247
 * @since 2021-10-19
 */
@Slf4j
@Component
public class ProtectedResourceEncryptService implements ProtectedResourceMonitor {
    private final JsonSchemaValidator jsonSchemaValidator;
    private final EncryptorService encryptorService;

    /**
     * constructor
     *
     * @param jsonSchemaValidator jsonSchemaValidator
     * @param encryptorService    encryptorService
     */
    public ProtectedResourceEncryptService(JsonSchemaValidator jsonSchemaValidator, EncryptorService encryptorService) {
        this.jsonSchemaValidator = jsonSchemaValidator;
        this.encryptorService = encryptorService;
    }

    /**
     * event type
     *
     * @return event type
     */
    @Override
    public List<String> getTypes() {
        return Arrays.asList("create", "update");
    }

    /**
     * handle protected resource event
     *
     * @param invocation invocation
     * @param event      event
     * @return result
     */
    @Override
    public Object invoke(Invocation<ProtectedResourceEvent, Object> invocation, ProtectedResourceEvent event) {
        ProtectedResource protectedResource = event.getResource();
        while (protectedResource != null) {
            encrypt(protectedResource);
            protectedResource = protectedResource.getEnvironment();
        }
        return invocation.invoke(event);
    }

    private void encrypt(ProtectedResource protectedResource) {
        encrypt(protectedResource.getAuth());
        List<String> fields = jsonSchemaValidator.getSecretFields(protectedResource.getSubType());
        encrypt(protectedResource.getExtendInfo(), fields);
        if (protectedResource instanceof ProtectedEnvironment) {
            ProtectedEnvironment environment = (ProtectedEnvironment) protectedResource;
            String password = environment.getPassword();
            environment.setPassword(encrypt(password));
        }
    }

    private void encrypt(Authentication authentication) {
        if (authentication == null) {
            return;
        }
        authentication.setAuthPwd(encrypt(authentication.getAuthPwd()));
        encrypt(authentication.getExtendInfo(), null);
    }

    private void encrypt(Map<String, String> data, List<String> keys) {
        if (data == null) {
            return;
        }
        List<String> fields = Optional.ofNullable(keys).orElseGet(() -> new ArrayList<>(data.keySet()));
        for (String field : fields) {
            if (data.containsKey(field)) {
                data.put(field, encrypt(data.get(field)));
            }
        }
    }

    private String encrypt(String value) {
        if (StringUtils.isBlank(value)) {
            return null;
        }
        return encryptorService.encrypt(value);
    }
}
