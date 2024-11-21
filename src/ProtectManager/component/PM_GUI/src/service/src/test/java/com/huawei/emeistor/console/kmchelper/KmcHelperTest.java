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
package com.huawei.emeistor.console.kmchelper;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.when;

import com.huawei.emeistor.console.kmchelp.KmcHelper;
import com.huawei.emeistor.console.kmchelp.kmc.CryptLogger;
import com.huawei.emeistor.console.kmchelp.kmc.KmcInstance;
import com.huawei.kmc.common.AppException;
import com.huawei.kmc.crypt.CryptoAPI;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.io.IOException;
import java.nio.charset.StandardCharsets;

/**
 * KmcHelper LLT
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({KmcHelper.class, KmcInstance.class})
public class KmcHelperTest {
    /**
     * 用例场景： 测试能否获取config path
     * 前置条件：无
     * 检  查  点：获取path不为空
     */
    @Test
    public void get_config_path_success() {
        CryptLogger cryptLogger = new CryptLogger();
        String path = null;
        try {
            path = KmcHelper.getConfigPath();
        } catch (IOException e) {
            cryptLogger.error(e.toString());
        }
        Assert.assertNotNull(path);
    }

    /**
     * 用例场景：获取实例
     * 前置条件：mock
     * 检查点：成功获取
     */
    @Test
    public void should_get_instance_successful() throws Exception {
        // mock static method
        PowerMockito.mockStatic(KmcInstance.class);

        // then
        KmcHelper kmcHelper = KmcHelper.getInstance();
        Assert.assertNotNull(kmcHelper);
    }

    /**
     * 用例场景：解密
     * 前置条件：mock
     * 检查点：成功运行解密过程
     */
    @Test
    public void should_decrypt_successful() throws AppException {
        // mock static method
        PowerMockito.mockStatic(KmcInstance.class);
        CryptoAPI cryptoAPI = mock(CryptoAPI.class);
        PowerMockito.when(KmcInstance.getInstance()).thenReturn(cryptoAPI);
        when(cryptoAPI.decrypt("test".getBytes(StandardCharsets.UTF_8))).thenReturn("success".getBytes(StandardCharsets.UTF_8));
        String str = KmcHelper.getInstance().decrypt("test");
        Assert.assertEquals("success", str);
    }

    /**
     * 用例场景：解密
     * 前置条件：mock
     * 检查点：运行解密，返回值为null
     */
    @Test
    public void should_return_null_when_decrypt_is_given_null() {
        // mock static method
        PowerMockito.mockStatic(KmcInstance.class);
        String str = KmcHelper.getInstance().decrypt(null);
        Assert.assertNull(str);
    }

    /**
     * 用例场景：加密
     * 前置条件：mock
     * 检查点：成功运行加密过程
     */
    @Test
    public void should_encrypt_successful() throws AppException {
        // mock static method
        PowerMockito.mockStatic(KmcInstance.class);
        CryptoAPI cryptoAPI = mock(CryptoAPI.class);
        PowerMockito.when(KmcInstance.getInstance()).thenReturn(cryptoAPI);
        when(cryptoAPI.encrypt("test".getBytes(StandardCharsets.UTF_8))).thenReturn("success".getBytes(StandardCharsets.UTF_8));
        String str = KmcHelper.getInstance().encrypt("test");
        Assert.assertEquals("success", str);
    }

    /**
     * 用例场景：加密
     * 前置条件：mock
     * 检查点：运行加密，返回值为null
     */
    @Test
    public void should_return_null_when_encrypt_is_given_null() {
        // mock static method
        PowerMockito.mockStatic(KmcInstance.class);
        String str = KmcHelper.getInstance().encrypt(null);
        Assert.assertNull(str);
    }


    /**
     * 用例场景：解密字符串失败
     * 前置条件：mock
     * 检查点：返回值为null
     *
     * @throws AppException decrypt方法可能抛出的异常
     */
    @Test
    public void should_return_null_when_decrypt_happens_exception() throws AppException {
        // mock static method
        PowerMockito.mockStatic(KmcInstance.class);
        CryptoAPI cryptoAPI = mock(CryptoAPI.class);
        PowerMockito.when(KmcInstance.getInstance()).thenReturn(cryptoAPI);
        when(cryptoAPI.decrypt("test".getBytes(StandardCharsets.UTF_8))).thenThrow(new AppException("test_decrypt"));
        KmcHelper kmcHelper = KmcHelper.getInstance();
        KmcHelper spy = spy(kmcHelper);
        String str = spy.decrypt("test");
        Assert.assertNull(str);
    }

    /**
     * 用例场景：加密字符串失败
     * 前置条件：mock
     * 检查点：返回值为null
     *
     * @throws AppException decrypt方法可能抛出的异常
     */
    @Test
    public void should_return_null_when_encrypt_happens_exception() throws AppException {
        // mock static method
        PowerMockito.mockStatic(KmcInstance.class);
        CryptoAPI cryptoAPI = mock(CryptoAPI.class);
        PowerMockito.when(KmcInstance.getInstance()).thenReturn(cryptoAPI);
        when(cryptoAPI.decrypt("test".getBytes(StandardCharsets.UTF_8))).thenThrow(new AppException("test_decrypt"));
        KmcHelper kmcHelper = KmcHelper.getInstance();
        KmcHelper spy = spy(kmcHelper);
        String str = spy.encrypt("test");
        Assert.assertNull(str);
    }

}