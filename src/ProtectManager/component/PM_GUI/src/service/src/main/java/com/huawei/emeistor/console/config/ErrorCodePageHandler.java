/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.emeistor.console.config;

import org.springframework.boot.web.server.ErrorPage;
import org.springframework.boot.web.server.ErrorPageRegistrar;
import org.springframework.boot.web.server.ErrorPageRegistry;
import org.springframework.context.annotation.Configuration;
import org.springframework.http.HttpStatus;

/**
 * 针对不同错误码跳转不同页面
 *
 * @author w30042425
 * @since 2023-09-22
 */
@Configuration
public class ErrorCodePageHandler implements ErrorPageRegistrar {
    /**
     * 注册错误码对应路径
     *
     * @param registry the error page registry
     */
    @Override
    public void registerErrorPages(ErrorPageRegistry registry) {
        ErrorPage notFoundError = new ErrorPage(HttpStatus.NOT_FOUND, "/404.do");
        registry.addErrorPages(notFoundError);
    }
}
