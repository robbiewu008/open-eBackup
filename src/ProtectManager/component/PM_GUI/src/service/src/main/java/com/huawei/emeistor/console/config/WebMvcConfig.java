/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package com.huawei.emeistor.console.config;

import org.springframework.context.annotation.Configuration;
import org.springframework.web.servlet.config.annotation.ResourceHandlerRegistry;
import org.springframework.web.servlet.config.annotation.ViewControllerRegistry;
import org.springframework.web.servlet.config.annotation.WebMvcConfigurer;

/**
 * 描述
 *
 * @author lwx544155
 * @version [OceanStor DJ V100R003C00, 2020年02月25日]
 * @see [相关类/方法]
 * @since [产品/模块版本]
 */
@Configuration
public class WebMvcConfig implements WebMvcConfigurer {
    // 设置访问路径前缀
    private static final String PATH_PATTERNS = "/**";

    private static final String FRONTEND_ROOT_PATH = "/app/gui/frontend/";

    // 设置资源路径
    private static final String LOCATIONS = "classpath:/static/";

    private static final String INDEX = "/console/index.html";

    private static final String CONSOLE = "/console/";

    private static final String ROOT_PATH = "/";

    /**
     * 添加静态资源文件，外部可以直接访问地址
     *
     * @param registry ResourceHandlerRegistry
     */
    @Override
    public void addResourceHandlers(ResourceHandlerRegistry registry) {
        // 需要配置1：----------- 需要告知系统，这是要被当成静态文件的！
        // 第一个方法设置访问路径前缀，第二个方法设置资源路径
        registry.addResourceHandler(PATH_PATTERNS).addResourceLocations("file:" + FRONTEND_ROOT_PATH);
    }

    /**
     * 设置自动跳转到登录页面
     *
     * @param registry registry
     */
    @Override
    public void addViewControllers(ViewControllerRegistry registry) {
        registry.addRedirectViewController(ROOT_PATH, INDEX);
        registry.addViewController(ROOT_PATH).setViewName(INDEX);
        registry.addViewController(CONSOLE).setViewName(INDEX);
        WebMvcConfigurer.super.addViewControllers(registry);
    }
}
