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
package com.huawei.oceanprotect.system.base.service.impl.pacific;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.sdk.system.model.StorageAuth;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import com.huawei.oceanprotect.system.base.service.impl.InitConfigServiceImpl;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;

/**
 * 获取配置信息serviceImpl类
 *
 * @author swx1010572
 * @version: [DataBackup 1.5.0]
 * @since 2023-07-25
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(JSONObject.class)
public class InitConfigServiceImplTest {

    @InjectMocks
    InitConfigServiceImpl initConfigService;

    @Mock
    InitNetworkConfigMapper initNetworkConfigMapper;

    @Mock
    EncryptorService encryptorService;


    @Test
    public void test_init_config_service(){
        new InitConfigServiceImpl(null,null);
        Assert.assertTrue(true);
    }

    @Test
    public void test_update_local_storage_auth(){
        PowerMockito.when(encryptorService.encrypt(any())).thenReturn("pswd");
        PowerMockito.doNothing().when(initNetworkConfigMapper).deleteInitConfig(any());
        PowerMockito.doNothing().when(initNetworkConfigMapper).insertInitConfig(any());
        Assert.assertTrue(true);
    }

    @Test
    public void test_get_local_storage_auth(){
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(any())).thenReturn(new ArrayList<InitConfigInfo>(){{
            add(new InitConfigInfo(){{
                setInitValue("aa");
            }});
        }});
        PowerMockito.mockStatic(JSONObject.class);
        PowerMockito.when(JSONObject.toBean(anyString(),any())).thenReturn(new StorageAuth());
        PowerMockito.when(encryptorService.decrypt(any())).thenReturn("decryptedPassword");
        Assert.assertTrue(initConfigService.getLocalStorageAuth().getPassword().equals("decryptedPassword"));
    }

    @Test
    public void test_update_local_storage_device_id(){
        PowerMockito.doNothing().when(initNetworkConfigMapper).deleteInitConfig(any());
        PowerMockito.doNothing().when(initNetworkConfigMapper).insertInitConfig(any());
        Assert.assertTrue(true);
    }

    @Test
    public void test_local_storage_device_id(){
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(any())).thenReturn(new ArrayList<InitConfigInfo>(){{
            add(new InitConfigInfo(){{
                setInitValue("aa");
            }});
        }});

        Assert.assertEquals("aa",initConfigService.getLocalStorageDeviceId());
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(any())).thenReturn(null);
        Assert.assertThrows(LegoCheckedException.class,()->initConfigService.getLocalStorageDeviceId());
    }

    @Test
    public void update_local_storage_device_ip() {
        PowerMockito.doNothing().when(initNetworkConfigMapper).deleteInitConfig(any());
        PowerMockito.doNothing().when(initNetworkConfigMapper).insertInitConfig(any());
        Assert.assertTrue(true);
    }

    @Test
    public void get_local_storage_device_ip() {
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(any())).thenReturn(null);
        Assert.assertEquals("",initConfigService.getLocalStorageDeviceIp());
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(any())).thenReturn(new ArrayList<InitConfigInfo>(){{
            add(new InitConfigInfo(){{
                setInitValue("aa");
            }});
        }});
        Assert.assertEquals("aa",initConfigService.getLocalStorageDeviceIp());
    }
}
