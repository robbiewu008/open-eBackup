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
package openbackup.system.base.util;

import static org.mockito.ArgumentMatchers.any;

import openbackup.system.base.sdk.kmc.EncryptorRestApi;
import openbackup.system.base.sdk.kmc.model.PlaintextVo;
import openbackup.system.base.util.KeyToolUtil;

import org.apache.commons.io.FileUtils;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.io.File;
import java.io.FileInputStream;
import java.lang.reflect.Field;
import java.nio.charset.StandardCharsets;
import java.security.KeyStore;
import java.security.PrivateKey;
import java.security.cert.X509Certificate;

/**
 * KeyToolUtilTest
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {KeyToolUtil.class, FileUtils.class, KeyStore.class})
public class KeyToolUtilTest {
    private static final String type = "PKCS12";
    private final EncryptorRestApi encryptorRestApi = PowerMockito.mock(EncryptorRestApi.class);
    private final KeyStore keyStore = PowerMockito.mock(KeyStore.class);

    @InjectMocks
    private KeyToolUtil keyToolUtil;

    @Before
    public void initTest() throws Exception {
        PowerMockito.mockStatic(FileUtils.class);
        PowerMockito.mockStatic(KeyStore.class);
        Field field = PowerMockito.field(KeyToolUtil.class, "type");
        field.set(keyToolUtil, type);
        FileInputStream fileInputStream = PowerMockito.mock(FileInputStream.class);
        PowerMockito.whenNew(FileInputStream.class).withAnyArguments().thenReturn(fileInputStream);
    }

    /**
     * 用例场景：获取解密的keystore密码
     * 前置条件：服务正常
     * 检查点：获取解密后的密码成功
     */
    @Test
    public void get_keystore_password_success() throws Exception {
        File keyStoreAuthFile = PowerMockito.mock(File.class);
        PowerMockito.whenNew(File.class).withAnyArguments().thenReturn(keyStoreAuthFile);
        PowerMockito.when(FileUtils.readFileToString(keyStoreAuthFile, StandardCharsets.UTF_8)).thenReturn("cipherText");
        PlaintextVo plaintextVo = new PlaintextVo();
        plaintextVo.setPlaintext("plainText");
        PowerMockito.when(encryptorRestApi.decrypt(any())).thenReturn(plaintextVo);
        String plainText = keyToolUtil.getKeyStorePassword("keyStoreAuthFile");
        Assert.assertEquals("plainText", plainText);
    }

    /**
     * 用例场景：生成keystore
     * 前置条件：服务正常
     * 检查点：生成keystore成功
     */
    @Test
    public void gen_keystore_success() throws Exception {
        PowerMockito.when(KeyStore.getInstance("PKCS12")).thenReturn(keyStore);
        KeyToolUtil.genKeyStore("storePwd", "storePath");
        Mockito.verify(keyStore, Mockito.times(1)).load(any(), any());
        Mockito.verify(keyStore, Mockito.times(1)).store(any(), any());
    }

    /**
     * 用例场景：加载keystore
     * 前置条件：服务正常
     * 检查点：加载keystore成功
     */
    @Test
    public void load_keystore_success() throws Exception {
        PowerMockito.when(KeyStore.getInstance(type)).thenReturn(keyStore);
        keyToolUtil.loadKeystore("storePwd", "storePath");
        Mockito.verify(keyStore, Mockito.times(1)).load(any(), any());
    }

    /**
     * 用例场景：生成token entry
     * 前置条件：服务正常
     * 检查点：生成token entry成功
     */
    @Test
    public void update_token_entry_success() throws Exception {
        PowerMockito.when(KeyStore.getInstance(type)).thenReturn(keyStore);
        X509Certificate x509Certificate = PowerMockito.mock(X509Certificate.class);
        PrivateKey privateKey = PowerMockito.mock(PrivateKey.class);
        keyToolUtil.updateTokenEntry("storePwd", x509Certificate,"storePath", privateKey, "privatePwd");
    }

    /**
     * 用例场景：导入证书
     * 前置条件：服务正常
     * 检查点：导入证书成功
     */
    @Test
    public void import_cert_success() throws Exception {
        PowerMockito.when(KeyStore.getInstance(type)).thenReturn(keyStore);
        X509Certificate x509Certificate = PowerMockito.mock(X509Certificate.class);
        keyToolUtil.importCert("storePwd", x509Certificate,"storePath", "privatePwd");
    }
}
