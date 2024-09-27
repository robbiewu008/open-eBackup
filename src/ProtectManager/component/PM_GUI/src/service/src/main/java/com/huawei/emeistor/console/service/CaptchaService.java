/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
