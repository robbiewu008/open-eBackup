/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * @author w00616953
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-28
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
