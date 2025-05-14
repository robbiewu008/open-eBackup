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
package com.huawei.oceanprotect.system.base.controller;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import com.huawei.oceanprotect.system.base.alarm.common.utils.UserUtils;

import openbackup.system.base.config.GlobalExceptionHandler;
import openbackup.system.base.sdk.system.model.EsnInfo;
import com.huawei.oceanprotect.system.base.license.serialnumber.service.DeviceInfoService;

import org.junit.Assert;
import org.junit.Before;
import org.junit.ClassRule;
import org.junit.Test;
import org.junit.contrib.java.lang.system.EnvironmentVariables;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureWebMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.MessageSource;
import org.springframework.http.MediaType;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.util.ReflectionTestUtils;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.setup.MockMvcBuilders;

import java.lang.reflect.Field;

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.result.MockMvcResultHandlers.print;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

/**
 * {@link SystemInternalController} 测试类
 *
 * @version OceanCyber 300 1.1.0
 * @since 2023-09-26
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@AutoConfigureWebMvc
@AutoConfigureMockMvc
@SpringBootTest(classes = {
    SystemInternalController.class
})
@PrepareForTest( {
    UserUtils.class
})
public class SystemInternalControllerTest {
    /**
     * 设置环境变量部署形态为安全一体机
     */
    @ClassRule
    public static final EnvironmentVariables ENVIRONMENT_VARIABLES =
        new EnvironmentVariables().set("DEPLOY_TYPE", "d5");

    @Autowired
    private MockMvc mockMvc;

    @MockBean
    private MessageSource messageSource;

    @MockBean
    private DeviceInfoService deviceInfoService;

    @MockBean
    private ClusterBasicService clusterBasicService;

    private SystemInternalController systemInternalController = new SystemInternalController();

    @Before
    public void init() throws IllegalAccessException {
        GlobalExceptionHandler handler = new GlobalExceptionHandler();
        mockMvc = MockMvcBuilders.standaloneSetup(systemInternalController).setControllerAdvice(handler).build();
        Field messageSourceField = PowerMockito.field(GlobalExceptionHandler.class, "messageSource");
        messageSourceField.set(handler, messageSource);

        PowerMockito.mockStatic(UserUtils.class);
        PowerMockito.when(UserUtils.isCurrentUserDpAdmin()).thenReturn(false);
    }

    /**
     * 测试场景：测试queryEsn函数基本功能
     * 前置条件: 无
     * 检查点：无异常抛出，正常返回esn值，接口访问通过
     */
    @Test
    public void query_esn_success() throws Exception {
        clusterBasicService = Mockito.mock(ClusterBasicService.class);
        PowerMockito.when(clusterBasicService.getCurrentClusterEsn()).thenReturn("Test");
        ReflectionTestUtils.setField(systemInternalController, "clusterBasicService", clusterBasicService);
        EsnInfo esnInfo = systemInternalController.queryEsnInternal();
        Assert.assertEquals("Test", esnInfo.getEsn());
        mockMvc.perform(get("/v1/system/esn").contentType(MediaType.APPLICATION_JSON))
            .andDo(print())
            .andExpect(status().isOk());
    }


    /**
     * 测试场景：测试queryEsn函数基本功能
     * 前置条件: 无
     * 检查点：无异常抛出，正常返回esn值，接口访问通过
     */
    @Test
    public void query_esn_internal_success() throws Exception {
        clusterBasicService = Mockito.mock(ClusterBasicService.class);
        PowerMockito.when(clusterBasicService.getCurrentClusterEsn()).thenReturn("Test");
        ReflectionTestUtils.setField(systemInternalController, "clusterBasicService", clusterBasicService);
        EsnInfo esnInfo = systemInternalController.queryEsnInternal();
        Assert.assertEquals("Test", esnInfo.getEsn());
        mockMvc.perform(get("/v1/internal/system/esn").contentType(MediaType.APPLICATION_JSON))
            .andDo(print())
            .andExpect(status().isOk());
    }
}