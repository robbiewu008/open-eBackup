/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package com.huawei.emeistor.console.service.impl;

import com.huawei.emeistor.console.contant.CommonErrorCode;
import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.controller.request.AuthRequest;
import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.service.CaptchaService;
import com.huawei.emeistor.console.util.CheckCodeUtil;
import com.huawei.emeistor.console.util.RequestUtil;

import org.apache.commons.lang3.StringUtils;
import org.redisson.api.RBucket;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * 验证码相关service
 *
 * @author t00482481
 * @since 2020-9-06
 */
@Service
public class CaptchaServiceImpl implements CaptchaService {
    private static final String NEED_VRF = "need_vrf_";

    private static final int FOUR = 4;

    @Autowired
    private HttpServletRequest request;

    @Autowired
    private HttpServletResponse response;

    @Autowired
    private RedissonClient redissonClient;

    /**
     * 校验验证码是否符合要求
     *
     * @param authRequest 登录请求参数
     */
    @Override
    public void checkVerifyCode(AuthRequest authRequest) {
        checkVerifyCode(authRequest.getVerifyCode());
    }

    @Override
    public void checkVerifyCode(String verifyCode) {
        String storeCode = getVrf();
        if (StringUtils.isEmpty(storeCode)) {
            return;
        }
        if (!StringUtils.isEmpty(verifyCode)
            && verifyCode.trim().equalsIgnoreCase(storeCode)) {
            Cookie cookie = new Cookie(ConfigConstant.CAPTCHA_CODE, null);
            cookie.setPath(ConfigConstant.COOKIE_PATH);
            cookie.setMaxAge(0);
            response.addCookie(cookie);
            return;
        }
        addVerificationCodeTag();
        throw new LegoCheckedException(CommonErrorCode.WRONG_VRF_CODE);
    }

    @Override
    @ExterAttack
    public String generateVerificationCode() {
        String random = CheckCodeUtil.runVerifyCode(FOUR);
        String ip = RequestUtil.getClientIpAddress(request);
        RBucket<String> rb = redissonClient.getBucket(NEED_VRF + ip);
        rb.set(random);
        return random;
    }

    @ExterAttack
    private String getVrf() {
        String ip = RequestUtil.getClientIpAddress(request);
        RBucket<String> rb = redissonClient.getBucket(NEED_VRF + ip);
        if (rb.isExists()) {
            return rb.get();
        }
        return StringUtils.EMPTY;
    }

    /**
     * 添加验证码标识
     */
    @Override
    public void addVerificationCodeTag() {
        Cookie cookie = new Cookie(ConfigConstant.CAPCHA, "true");
        cookie.setPath(ConfigConstant.COOKIE_PATH);
        cookie.setSecure(true);
        response.addCookie(cookie);
    }

    @Override
    @ExterAttack
    public void cleanVerificationCodeTag(String ip) {
        RBucket<String> rb = redissonClient.getBucket(NEED_VRF + ip);
        if (rb.isExists()) {
            rb.delete();
        }
    }

    /**
     * 登录过一次之后，使验证码失效
     */
    @Override
    @ExterAttack
    public void invalidVerificationCode() {
        String ip = RequestUtil.getClientIpAddress(request);
        RBucket<String> rb = redissonClient.getBucket(NEED_VRF + ip);
        if (rb.isExists()) {
            rb.set(CheckCodeUtil.runVerifyCode(FOUR));
        }
    }
}
