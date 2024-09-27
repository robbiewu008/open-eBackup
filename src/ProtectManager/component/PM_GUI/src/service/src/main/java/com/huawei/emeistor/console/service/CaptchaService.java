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

import com.huawei.emeistor.console.controller.request.AuthRequest;

/**
 * 验证码相关service
 *
 * @author t00482481
 * @since 2020-9-06
 */
public interface CaptchaService {
    /**
     * 校验验证码是否符合要求
     *
     * @param authRequest 登录请求参数
     */
    void checkVerifyCode(AuthRequest authRequest);

    /**
     * 校验验证码是否符合要求
     *
     * @param verifyCode 登录验证码
     */
    void checkVerifyCode(String verifyCode);

    /**
     * 生成验证码
     *
     * @return 验证码
     */
    String generateVerificationCode();

    /**
     * 添加验证码标识
     */
    void addVerificationCodeTag();

    /**
     * 清除验证码标识
     *
     * @param ip IP地址
     */
    void cleanVerificationCodeTag(String ip);

    /**
     * 使验证码失效
     */
    void invalidVerificationCode();
}
