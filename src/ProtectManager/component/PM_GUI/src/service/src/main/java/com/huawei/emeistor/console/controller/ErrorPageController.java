/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.emeistor.console.controller;

import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.RequestMapping;

import java.io.IOException;

import javax.servlet.http.HttpServletResponse;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-09-22
 */
@Controller
public class ErrorPageController {
    /**
     * 404 错误重定向到首页
     *
     * @param response 响应
     * @throws IOException IO异常
     */
    @RequestMapping(value = "/404.do")
    public void notFound(HttpServletResponse response) throws IOException {
        response.sendRedirect(response.encodeRedirectURL("/console/#/home"));
    }
}
