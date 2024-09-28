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
