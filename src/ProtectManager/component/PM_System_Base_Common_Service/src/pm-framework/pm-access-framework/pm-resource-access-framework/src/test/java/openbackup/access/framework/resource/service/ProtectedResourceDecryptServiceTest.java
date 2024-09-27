/*
 *
 *  * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 */

package openbackup.access.framework.resource.service;

import openbackup.access.framework.resource.service.ProtectedResourceDecryptService;
import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.HashMap;

/**
 * ProtectedResourceDecryptServiceTest
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-29
 */
public class ProtectedResourceDecryptServiceTest {
    /**
     * 用例场景： 解密auth成功<br/>
     * 前置条件： 无<br/>
     * 检查 点： 当encryptorService解密返回null时，无异常<br/>
     */
    @Test
    public void decrypt_success_when_encryptor_return_null() {
        JsonSchemaValidator jsonSchemaValidator = Mockito.mock(JsonSchemaValidator.class);
        EncryptorService encryptorService = Mockito.mock(EncryptorService.class);
        Mockito.when(encryptorService.decrypt(Mockito.anyString())).thenReturn(null);
        ProtectedResourceDecryptService protectedResourceDecryptService = new ProtectedResourceDecryptService(
            jsonSchemaValidator, encryptorService);
        Authentication authentication = new Authentication();
        authentication.setExtendInfo(new HashMap<>());
        authentication.getExtendInfo().put("key1", "value1");

        protectedResourceDecryptService.decrypt(authentication);
        Assert.assertEquals(authentication.getExtendInfo().size(), 0);
    }
}
