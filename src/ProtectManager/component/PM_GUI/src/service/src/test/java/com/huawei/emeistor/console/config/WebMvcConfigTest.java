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
package com.huawei.emeistor.console.config;

import org.checkerframework.checker.units.qual.A;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.ApplicationContext;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.util.ReflectionTestUtils;
import org.springframework.web.servlet.config.annotation.ResourceHandlerRegistry;
import org.springframework.web.servlet.config.annotation.ViewControllerRegistry;

import javax.servlet.ServletContext;

/**
 * 功能描述
 *
 */
@RunWith(SpringRunner.class)
public class WebMvcConfigTest {
    /**
     * 用例名称：添加ResourceHandler成功
     * 前置条件：无
     * check点：无报错
     */
    @Test
    public void should_returnCorrectValue_when_addResourceHandlers() {
        WebMvcConfig config = new WebMvcConfig();
        ApplicationContext applicationContext = Mockito.mock(ApplicationContext.class);
        ServletContext servletContext = Mockito.mock(ServletContext.class);
        config.addResourceHandlers(new ResourceHandlerRegistry(applicationContext, servletContext));
    }

    /**
     * 用例名称：添加ViewController成功
     * 前置条件：无
     * check点：无报错
     */
    @Test
    public void should_returnCorrectValue_when_addViewControllers() {
        WebMvcConfig config = new WebMvcConfig();
        ApplicationContext mock = Mockito.mock(ApplicationContext.class);
        config.addViewControllers(new ViewControllerRegistry(mock));
    }
}
