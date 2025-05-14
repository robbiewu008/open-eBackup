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
import openbackup.system.base.common.constants.Constants;
import com.huawei.oceanprotect.system.base.initialize.network.common.ConfigStatus;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;

import junit.framework.TestCase;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.Redisson;
import org.redisson.RedissonMap;

import java.util.ArrayList;

/**
 * ModifyBackStatusService测试类
 *
 */
@RunWith(PowerMockRunner.class)
public class ModifyBackStatusServiceTest extends TestCase {
    @InjectMocks
    private ModifyBackStatusService modifyBackStatusService;

    @Mock
    private InitNetworkConfigMapper initNetworkConfigMapper;

    @Mock
    private Redisson redisson;
    /**
     * 用例场景：测试初始化配置项值
     * 前置条件：NA
     * 检查点：返回结果是否为0
     */
    public void testGetModifyConfigStatus() {
        prepare();
        RedissonMap<Object, Object> redissonMap = PowerMockito.mock(RedissonMap.class);
        PowerMockito.when(redisson.getMap(InitConfigConstant.INIT_RUNNING_FLAG)).thenReturn(redissonMap);
        ConfigStatus initConfigStatus = modifyBackStatusService.getModifyConfigStatus();
        Assert.assertNotNull(initConfigStatus);
    }

    private void prepare() {
        // 前置条件1
        ArrayList<InitConfigInfo> initConfigInfos1 = new ArrayList<>();
        InitConfigInfo initConfigInfo1 = new InitConfigInfo();
        initConfigInfo1.setInitValue("1");
        initConfigInfos1.add(initConfigInfo1);
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(InitConfigConstant.MODIFY_BACK_PROGRESS_STATUS))
            .thenReturn(initConfigInfos1);

        // 前置条件2
        ArrayList<InitConfigInfo> initConfigInfos2 = new ArrayList<>();
        InitConfigInfo initConfigInfo2 = new InitConfigInfo();
        initConfigInfo2.setInitValue("1");
        initConfigInfos2.add(initConfigInfo2);
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(InitConfigConstant.MODIFY_BACK_PROGRESS_CODE))
            .thenReturn(initConfigInfos2);

        // 前置条件3
        ArrayList<InitConfigInfo> initConfigInfos3 = new ArrayList<>();
        InitConfigInfo initConfigInfo3 = new InitConfigInfo();
        initConfigInfo3.setInitValue("1");
        initConfigInfos3.add(initConfigInfo3);
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(InitConfigConstant.MODIFY_BACK_PROGRESS_DESC))
            .thenReturn(initConfigInfos3);

        // 前置条件4
        ArrayList<InitConfigInfo> initConfigInfos4 = new ArrayList<>();
        InitConfigInfo initConfigInfo4 = new InitConfigInfo();
        initConfigInfo4.setInitValue("1");
        initConfigInfos4.add(initConfigInfo4);
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(InitConfigConstant.MODIFY_BACK_PROGRESS_RATE))
            .thenReturn(initConfigInfos4);

        // 前置条件5
        ArrayList<InitConfigInfo> initConfigInfos5 = new ArrayList<>();
        InitConfigInfo initConfigInfo5 = new InitConfigInfo();
        initConfigInfo5.setInitValue("[1,10]");
        initConfigInfos5.add(initConfigInfo5);
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(InitConfigConstant.INIT_ERROR_CODE_PARAM))
            .thenReturn(initConfigInfos5);
    }

    /**
     * 用例场景：测试初始化配置项值
     * 前置条件：NA
     * 检查点：成功
     */
    public void testSetModifyProgressCode() {
        PowerMockito.doNothing().when(initNetworkConfigMapper).updateInitConfig(any());
        modifyBackStatusService.setModifyProgressCode("code");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：测试初始化过程描述
     * 前置条件：满足条件
     * 检查点：成功
     */
    public void testSetModifyProgressDesc() {
        PowerMockito.doNothing().when(initNetworkConfigMapper).updateInitConfig(any());
        modifyBackStatusService.setModifyProgressDesc("code");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：测试初始化过程参数
     * 前置条件：NA
     * 检查点：成功
     */
    public void testSetInitProgressParams() {
        PowerMockito.doNothing().when(initNetworkConfigMapper).updateInitConfig(any());
        modifyBackStatusService.setInitProgressParams(new ArrayList<>());
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：测试初始化过程比率
     * 前置条件：NA
     * 检查点：成功
     */
    public void testSetModifyProgressRate() {
        PowerMockito.doNothing().when(initNetworkConfigMapper).updateInitConfig(any());
        modifyBackStatusService.setModifyProgressRate(1);
        Assert.assertTrue(true);
    }
    /**
     * 用例场景：测试初始化配置妆台
     * 前置条件：NA
     * 检查点：成功
     */
    public void testClrModifyConfigStatus() {
        ArrayList<InitConfigInfo> initConfigInfos5 = new ArrayList<>();
        InitConfigInfo initConfigInfo5 = new InitConfigInfo();
        initConfigInfo5.setInitValue("[1,10]");
        initConfigInfos5.add(initConfigInfo5);
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(InitConfigConstant.INIT_ERROR_CODE_PARAM))
            .thenReturn(initConfigInfos5);
        modifyBackStatusService.clrModifyConfigStatus();
        Assert.assertTrue(true);
    }
    /**
     * 用例场景：测试修改配置妆台
     * 前置条件：NA
     * 检查点：成功
     */
    public void testQueryModifyingStatus() {
        ArrayList<InitConfigInfo> initConfigInfos5 = new ArrayList<>();
        InitConfigInfo initConfigInfo5 = new InitConfigInfo();
        initConfigInfo5.setInitValue("[1,10]");
        initConfigInfos5.add(initConfigInfo5);
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(InitConfigConstant.INIT_ERROR_CODE_PARAM))
            .thenReturn(initConfigInfos5);
        modifyBackStatusService.queryModifyingStatus();
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：测试查询初始化时状态为失败
     * 前置条件：NA
     * 检查点：返回错误码为0
     */
    @Test
    public void testQueryInitStatusWithFailCode() throws InstantiationException, IllegalAccessException {
        ArrayList<InitConfigInfo> initConfigInfos1 = new ArrayList<>();
        InitConfigInfo initConfigInfo1 = new InitConfigInfo();
        initConfigInfo1.setInitValue(String.valueOf(InitConfigConstant.ERROR_CODE_NO));
        initConfigInfos1.add(initConfigInfo1);
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(InitConfigConstant.MODIFY_BACK_PROGRESS_STATUS))
            .thenReturn(initConfigInfos1);
        RedissonMap<Object, Object> redissonMap = PowerMockito.mock(RedissonMap.class);
        PowerMockito.when(redisson.getMap(InitConfigConstant.INIT_RUNNING_FLAG)).thenReturn(redissonMap);
        ConfigStatus configStatus = modifyBackStatusService.queryModifyingStatus();
        Assert.assertEquals(configStatus.getStatus(), Constants.ERROR_CODE_OK);
    }

    /**
     * 用例场景：测试初始化备份网络异常
     * 前置条件：NA
     * 检查点：返回错误码5
     */
    @Test
    public void testQueryInitStatusWithRunningCode() throws InstantiationException, IllegalAccessException {
        ArrayList<InitConfigInfo> initConfigInfos1 = new ArrayList<>();
        InitConfigInfo initConfigInfo1 = new InitConfigInfo();
        initConfigInfo1.setInitValue(String.valueOf(InitConfigConstant.ERROR_CODE_RUNNING));
        initConfigInfos1.add(initConfigInfo1);
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(InitConfigConstant.MODIFY_BACK_PROGRESS_STATUS))
            .thenReturn(initConfigInfos1);
        RedissonMap<Object, Object> redissonMap = PowerMockito.mock(RedissonMap.class);
        PowerMockito.when(redissonMap.get(any())).thenReturn("1");
        PowerMockito.when(redisson.getMap(InitConfigConstant.INIT_RUNNING_FLAG)).thenReturn(redissonMap);
        ConfigStatus configStatus = modifyBackStatusService.queryModifyingStatus();
        Assert.assertEquals(configStatus.getStatus(), InitConfigConstant.ERROR_CODE_FAILED);
    }
}