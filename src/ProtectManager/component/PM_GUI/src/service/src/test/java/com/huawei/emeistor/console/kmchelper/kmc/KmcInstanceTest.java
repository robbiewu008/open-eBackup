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
package com.huawei.emeistor.console.kmchelper.kmc;

import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.mock;

import com.huawei.emeistor.console.kmchelp.KmcHelper;
import com.huawei.emeistor.console.kmchelp.kmc.KmcInstance;
import com.huawei.kmc.common.InitStage;
import com.huawei.kmc.common.KmcMkInfo;
import com.huawei.kmc.crypt.CryptoAPI;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.core.classloader.annotations.SuppressStaticInitializationFor;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * 功能描述 获取CryptoAPI单例实例测试类
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {KmcInstance.class, CryptoAPI.class, KmcHelper.class})
@SuppressStaticInitializationFor("com.huawei.kmc.crypt.CryptoAPI")
public class KmcInstanceTest {

    /**
     * 用例场景：测试初始化KMC组件
     * 前置条件：mock
     * 检查点：不报错
     */
    @Test
    public void should_init_component() throws Exception {
        PowerMockito.mockStatic(CryptoAPI.class);
        CryptoAPI mock = mock(CryptoAPI.class);
        KmcMkInfo kmcMkInfo = mock(KmcMkInfo.class);

        PowerMockito.when(CryptoAPI.getInstance()).thenReturn(mock);
        PowerMockito.when(mock.getMkCount()).thenReturn(2);
        PowerMockito.when(mock.getMkInfo(anyInt())).thenReturn(kmcMkInfo);
        KmcInstance.initComponent("path", "", false, true);
        PowerMockito.when(CryptoAPI.getInitStage()).thenReturn(2);
        KmcInstance.initComponent("path", "", false, true);
        Assert.assertNotNull(mock);
    }

    /**
     * 用例场景：测试初始化KMC组件
     * 前置条件：mock
     * 检查点：不报错
     */
    @Test
    public void should_return_when_initComponent_given_cryptoAPI_initStage_is_done() throws Exception {
        PowerMockito.mockStatic(CryptoAPI.class);
        PowerMockito.when(CryptoAPI.getInitStage()).thenReturn(InitStage.INIT_KMC_DONE.getValue());
        KmcInstance.initComponent("path", "", false, true);
        Assert.assertNotNull(1);
    }

    /**
     * 用例场景：测试释放CryptoAPI
     * 前置条件：mock
     * 检查点：不报错
     *
     * @throws Exception 声明CryptoAPI静态方法执行可能抛出的异常
     */
    @Test
    public void should_do_nothing_when_releaseComponent_invoke() throws Exception {
        PowerMockito.mockStatic(CryptoAPI.class);
        CryptoAPI cryptoAPI = mock(CryptoAPI.class);
        PowerMockito.when(CryptoAPI.getInstance()).thenReturn(cryptoAPI);
        KmcInstance.releaseComponent();
        Assert.assertNotNull(cryptoAPI);
    }
}