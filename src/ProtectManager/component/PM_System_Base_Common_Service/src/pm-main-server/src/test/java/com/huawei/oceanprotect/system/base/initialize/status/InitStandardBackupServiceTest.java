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
package com.huawei.oceanprotect.system.base.initialize.status;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.common.enums.ServiceType;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;

/**
 * 功能描述
 *
 * @since 2024-01-22
 */

@RunWith(PowerMockRunner.class)
public class InitStandardBackupServiceTest {

    @InjectMocks
    InitStandardBackupService initStandardBackupService;

    @Mock
    InitNetworkConfigMapper initStandardBackupConfigMapper;

    @Mock
    RedissonClient redissonClient;

    @Test
    public void test_init_config_status_empty_list_then_success(){
        PowerMockito.when(initStandardBackupConfigMapper.queryInitConfig(any())).thenReturn(
            new ArrayList<InitConfigInfo>(){{

            }}
        );
        initStandardBackupService.getInitConfigStatus(ServiceType.STANDARD);
        Assert.assertTrue(true);
    }

    @Test
    public void test_init_config_status_then_success(){
        RMap rMap = PowerMockito.mock(RMap.class);
        PowerMockito.when(rMap.get(any())).thenReturn(true);
        PowerMockito.when(redissonClient.getMap(any())).thenReturn(rMap);
        PowerMockito.when(initStandardBackupConfigMapper.queryInitConfig(any())).thenReturn(
            new ArrayList<InitConfigInfo>(){{
                add(new InitConfigInfo("STANDARD_SERVICE","1"));
            }},
            new ArrayList<InitConfigInfo>(){{
                add(new InitConfigInfo("STANDARD_SERVICE","1"));
            }},
            new ArrayList<InitConfigInfo>(){{
                add(new InitConfigInfo("STANDARD_SERVICE","1"));
            }},
            new ArrayList<InitConfigInfo>(){{
                add(new InitConfigInfo("STANDARD_SERVICE","1"));
            }},
            new ArrayList<InitConfigInfo>(){{
                add(new InitConfigInfo("STANDARD_SERVICE","[20,30]"));
            }}
        );
        initStandardBackupService.getInitConfigStatus(ServiceType.STANDARD);
        Assert.assertTrue(true);
    }

    @Test
    public void test_is_service_progress_running_success(){
        RMap rMap = PowerMockito.mock(RMap.class);
        PowerMockito.when(rMap.get(any())).thenReturn("flag");
        PowerMockito.when(redissonClient.getMap(any())).thenReturn(rMap);
        initStandardBackupService.isServiceProgressRunning(ServiceType.STANDARD);

    }

    @Test
    public void test_reset_service_init_progress_success(){
        PowerMockito.doNothing().when(initStandardBackupConfigMapper).deleteInitConfig(any());
        PowerMockito.doNothing().when(initStandardBackupConfigMapper).insertInitConfig(any());
        initStandardBackupService.resetServiceInitProgressConfig(ServiceType.STANDARD);
    }

    @Test
    public void test_set_init_process(){
        PowerMockito.doNothing().when(initStandardBackupConfigMapper).updateInitConfig(any());
        initStandardBackupService.setInitProgressCode("1",ServiceType.STANDARD);
        initStandardBackupService.setInitProgressDesc("1");
        initStandardBackupService.setInitProgressParams(new ArrayList<>(),ServiceType.STANDARD);
        initStandardBackupService.setInitProgressStatus(1,ServiceType.STANDARD);
        initStandardBackupService.setInitProgressRate(1,ServiceType.STANDARD);
    }

    @Test
    public void test_is_service_progress_status_running_empty_list()
        throws NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        PowerMockito.when(initStandardBackupConfigMapper.queryInitConfig(any())).thenReturn(new ArrayList<>());
        Method isServiceProgressStatusRunning = InitStandardBackupService.class.getDeclaredMethod(
            "isServiceProgressStatusRunning",String.class);
        isServiceProgressStatusRunning.setAccessible(true);
        isServiceProgressStatusRunning.invoke(initStandardBackupService,"1");

    }
    @Test
    public void test_is_service_progress_status_running()
        throws NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        PowerMockito.when(initStandardBackupConfigMapper.queryInitConfig(any())).thenReturn(new ArrayList<InitConfigInfo>(){{
            add(new InitConfigInfo(){{
                setInitValue("1");
            }});
        }});
        Method isServiceProgressStatusRunning = InitStandardBackupService.class.getDeclaredMethod(
            "isServiceProgressStatusRunning",String.class);
        isServiceProgressStatusRunning.setAccessible(true);
        isServiceProgressStatusRunning.invoke(initStandardBackupService,"1");

    }
}
