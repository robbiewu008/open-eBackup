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
package com.huawei.emeistor.console.service.impl;


import com.huawei.emeistor.console.bean.CiphertextVo;
import com.huawei.emeistor.console.bean.PlaintextVo;
import com.huawei.emeistor.console.kmchelp.KmcHelper;
import com.huawei.emeistor.console.kmchelp.kmc.KmcInstance;
import com.huawei.kmc.crypt.CryptoAPI;


import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * 供common调用KMS加解密实现 测试类
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {KmcHelper.class, KmcInstance.class, CryptoAPI.class})
public class EncryptorServiceImplTest {

    /**
     * 用例场景：检查解密调用是否正常
     * 前置条件：mock
     * 检查点：正常调用方法
     */
    @Test
    public void should_decrypt_successful() {
        // mock
        EncryptorServiceImpl encryptorService = new EncryptorServiceImpl();
        CiphertextVo ciphertextVo = new CiphertextVo();
        ciphertextVo.setCiphertext("test string.");

        // mock static method
        PowerMockito.mockStatic(KmcHelper.class);
        KmcHelper kmcHelper = PowerMockito.mock(KmcHelper.class);
        PowerMockito.when(KmcHelper.getInstance()).thenReturn(kmcHelper);

        // then
        PlaintextVo plaintextVo = encryptorService.decrypt(ciphertextVo);
        Assert.assertNotNull(plaintextVo);
    }

    /**
     * 用例场景：检查加密调用是否正常
     * 前置条件：mock
     * 检查点：正常调用方法
     */
    @Test
    public void should_encrypt_successful() {
        // mock
        EncryptorServiceImpl encryptorService = new EncryptorServiceImpl();
        PlaintextVo plaintextVo = new PlaintextVo();
        plaintextVo.setPlaintext("test string.");

        // mock static method
        PowerMockito.mockStatic(KmcHelper.class);
        KmcHelper kmcHelper = PowerMockito.mock(KmcHelper.class);
        PowerMockito.when(KmcHelper.getInstance()).thenReturn(kmcHelper);

        // then
        CiphertextVo ciphertextVo = encryptorService.encrypt(plaintextVo);
        Assert.assertNotNull(ciphertextVo);
    }
}