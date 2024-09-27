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
package com.huawei.emeistor.console.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import com.huawei.emeistor.console.service.CaptchaService;
import com.huawei.emeistor.console.service.impl.CaptchaServiceImpl;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.api.RBucket;
import org.redisson.api.RedissonClient;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.util.ReflectionTestUtils;

import java.io.File;

import javax.servlet.ServletOutputStream;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * 功能描述
 *
 * @author w00504341
 * @since 2021-03-15
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {CaptchaController.class, CaptchaServiceImpl.class,File.class})
public class CaptchaControllerTest {
    @Mock
    private RedissonClient redissonClient;

    @Mock
    private CaptchaService captchaService;

    @InjectMocks
    private CaptchaController captchaController;

    @Before
    public void setUp() {
        ReflectionTestUtils.setField(captchaController, "captchaPath", "/v1path");
    }

    @Test
    public void captchaSuccess() throws Exception {
        HttpServletRequest servletRequest = PowerMockito.mock(HttpServletRequest.class);
        HttpServletResponse servletResponse = PowerMockito.mock(HttpServletResponse.class);
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket(any())).thenReturn(rBucket);
        ServletOutputStream stream = PowerMockito.mock(ServletOutputStream.class);
        PowerMockito.when(servletResponse.getOutputStream()).thenReturn(stream);
        PowerMockito.when(servletRequest.getHeader(anyString())).thenReturn("123");
        PowerMockito.when(captchaService.generateVerificationCode()).thenReturn("test");
        captchaController.captcha(servletRequest, servletResponse);
    }

    @Test
    public void captchaEmpty() throws Exception {
        HttpServletRequest servletRequest = PowerMockito.mock(HttpServletRequest.class);
        HttpServletResponse servletResponse = PowerMockito.mock(HttpServletResponse.class);
        RBucket rBucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket(any())).thenReturn(rBucket);
        ServletOutputStream stream = PowerMockito.mock(ServletOutputStream.class);
        PowerMockito.when(servletResponse.getOutputStream()).thenReturn(stream);
        PowerMockito.when(servletRequest.getHeader(anyString())).thenReturn("123");
        PowerMockito.when(captchaService.generateVerificationCode()).thenReturn("test");
        PowerMockito.mockStatic(File.class);
        File mock = PowerMockito.mock(File.class);
        PowerMockito.when(mock.exists()).thenReturn(false);
        PowerMockito.when(mock.mkdirs()).thenReturn(false);
        PowerMockito.whenNew(File.class).withAnyArguments().thenReturn(mock);
        captchaController.captcha(servletRequest, servletResponse);
    }
}
