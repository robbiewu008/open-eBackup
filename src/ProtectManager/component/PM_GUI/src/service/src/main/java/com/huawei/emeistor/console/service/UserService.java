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
package com.huawei.emeistor.console.service;

import com.huawei.emeistor.console.bean.ADFSForwardDto;
import com.huawei.emeistor.console.controller.request.AuthRequest;
import com.huawei.emeistor.console.controller.request.SendDynamicCodeRequest;
import com.huawei.emeistor.console.controller.response.LoginResponse;

import java.io.IOException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * 登录相关service
 *
 * @author t00482481
 * @since 2020-9-06
 */
public interface UserService {
    /**
     * 登录
     *
     * @param authRequest 请求参数
     * @return 登录响应
     */
    LoginResponse login(AuthRequest authRequest);

    /**
     * hcs 用户登录处理
     */
    void hcsLogin();

    /**
     * DME 用户登录处理
     */
    void dmeLogin();

    /**
     * 发送动态口令
     *
     * @param sendDynamicCodeRequest sendDynamicCodeRequest
     */
    void sendDynamicCode(SendDynamicCodeRequest sendDynamicCodeRequest);

    /**
     * SAML用户登录处理
     *
     * @param request 请求体
     * @param response 响应体
     * @throws IOException 异常
     */
    void samlLogin(HttpServletRequest request, HttpServletResponse response) throws IOException;

    /**
     * ADFS 用户登录处理
     *
     * @param request 请求
     * @param response 响应
     * @throws IOException 异常
     */
    void adfsLogin(HttpServletRequest request, HttpServletResponse response) throws IOException;

    /**
     * ADFS跳转
     *
     * @param request 请求
     * @param response 响应
     * @return 跳转相关信息
     * @throws IOException 异常
     */
    ADFSForwardDto adfsRedirect(HttpServletRequest request, HttpServletResponse response) throws IOException;
}
