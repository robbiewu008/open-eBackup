/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.access.framework.resource.service;

import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.invoke.Invocation;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.AbstractMap;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.function.BiConsumer;
import java.util.function.Function;
import java.util.stream.Collectors;

/**
 * Protected Resource Decrypt Service
 *
 * @author l00272247
 * @since 2021-10-20
 */
@Component
public class ProtectedResourceDecryptService implements ProtectedResourceMonitor {
    private final JsonSchemaValidator jsonSchemaValidator;
    private final EncryptorService encryptorService;

    /**
     * constructor
     *
     * @param jsonSchemaValidator jsonSchemaValidator
     * @param encryptorService encryptorService
     */
    public ProtectedResourceDecryptService(JsonSchemaValidator jsonSchemaValidator, EncryptorService encryptorService) {
        this.jsonSchemaValidator = jsonSchemaValidator;
        this.encryptorService = encryptorService;
    }

    /**
     * invoke method
     *
     * @param invocation invocation
     * @param event protected resource event
     * @return result
     */
    @Override
    public Object invoke(Invocation<ProtectedResourceEvent, Object> invocation, ProtectedResourceEvent event) {
        ProtectedResource protectedResource = event.getResource();
        while (protectedResource != null) {
            decrypt(protectedResource);
            protectedResource = protectedResource.getEnvironment();
        }
        return invocation.invoke(event);
    }

    private void decrypt(ProtectedResource protectedResource) {
        List<String> fields = jsonSchemaValidator.getSecretFields(protectedResource.getSubType());
        for (String field : fields) {
            String value = protectedResource.getExtendInfoByKey(field);
            protectedResource.setExtendInfoByKey(field, decrypt(value));
        }
        decrypt(protectedResource.getAuth());
        if (protectedResource instanceof ProtectedEnvironment) {
            ProtectedEnvironment environment = (ProtectedEnvironment) protectedResource;
            String password = environment.getPassword();
            environment.setPassword(decrypt(password));
        }
    }

    /**
     * decrypt auth info
     *
     * @param auth auth info
     */
    public void decrypt(Authentication auth) {
        if (auth == null) {
            return;
        }
        decrypt(auth, Authentication::getAuthPwd, Authentication::setAuthPwd);
        Map<String, String> extendInfo =
                Optional.ofNullable(auth.getExtendInfo()).orElse(Collections.emptyMap()).entrySet().stream()
                        .filter(entry -> entry.getValue() != null)
                        .map(entry -> new AbstractMap.SimpleEntry<>(entry.getKey(), decrypt(entry.getValue())))
                        .filter(entry -> entry.getValue() != null)
                        .collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue));
        auth.setExtendInfo(extendInfo);
    }

    private <T> void decrypt(
            Authentication auth, Function<Authentication, T> getter, BiConsumer<Authentication, T> setter) {
        T value = getter.apply(auth);
        if (value instanceof String) {
            @SuppressWarnings("unchecked")
            T result = (T) decrypt((String) value);
            setter.accept(auth, result);
        } else {
            setter.accept(auth, value);
        }
    }

    private String decrypt(String value) {
        if (StringUtils.isEmpty(value)) {
            return value;
        }
        return encryptorService.decrypt(value);
    }

    @Override
    public List<String> getTypes() {
        return Collections.singletonList("decrypt");
    }
}
