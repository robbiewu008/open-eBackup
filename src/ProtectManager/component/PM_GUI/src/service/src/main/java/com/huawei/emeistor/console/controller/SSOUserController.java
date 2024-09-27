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

import com.huawei.emeistor.console.bean.ADFSForwardDto;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.service.UserService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.ResponseBody;

import java.io.IOException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * SSO用户(SAML用户,ADFS用户)登录接口
 *
 * @author w30042425
 * @since 2023-02-14
 */
@Slf4j
@Controller
@RequestMapping(ConfigConstant.CONSOLE)
public class SSOUserController {
    @Autowired
    private UserService userService;

    /**
     * ADFS 用户登录
     *
     * @param request 请求体
     * @param response 响应体
     * @throws IOException 异常
     */
    @ExterAttack
    @RequestMapping("/v1/auth/adfs/login")
    public void adfsLogin(HttpServletRequest request, HttpServletResponse response) throws IOException {
        log.info("adfs login start");
        userService.adfsLogin(request, response);
    }

    /**
     * ADFS 跳转
     *
     * @param request 请求头
     * @param response 请求体
     * @return 跳转相关信息
     * @throws IOException 异常
     */
    @ExterAttack
    @RequestMapping("/v1/auth/adfs/forward")
    @ResponseBody
    public ADFSForwardDto adfsForward(HttpServletRequest request, HttpServletResponse response) throws IOException {
        log.info("adfs forward start");
        return userService.adfsRedirect(request, response);
    }
}