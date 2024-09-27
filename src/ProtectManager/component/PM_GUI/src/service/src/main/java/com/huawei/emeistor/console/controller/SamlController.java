/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.emeistor.console.controller;

import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.service.UserService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.RequestMapping;

import java.io.IOException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * Saml用户登录接口
 *
 * @author w30042425
 * @since 2023-02-14
 */
@Slf4j
@Controller
@RequestMapping(ConfigConstant.CONSOLE)
public class SamlController {
    @Autowired
    private UserService userService;

    /**
     * saml用户登录
     *
     * @param request 请求体
     * @param response 响应体
     * @throws IOException 异常
     */
    @ExterAttack
    @RequestMapping("/v1/auth/saml/login")
    public void samlLogin(HttpServletRequest request, HttpServletResponse response) throws IOException {
        log.info("saml login start");
        userService.samlLogin(request, response);
    }
}
