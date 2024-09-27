/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 */

package com.huawei.emeistor.console.controller;

import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.service.CaptchaService;
import com.huawei.emeistor.console.util.CheckCodeUtil;

import org.apache.hc.core5.http.HttpStatus;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;

import java.awt.image.BufferedImage;
import java.io.File;

import javax.imageio.ImageIO;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * CaptchaController
 *
 * @author y00407642
 * @since 2019-10-26
 */
@Controller
@RequestMapping(ConfigConstant.CONSOLE)
public class CaptchaController {
    @Value("${captcha.path}")
    private String captchaPath;

    @Autowired
    private CaptchaService captchaService;

    /**
     * 工具类生成captcha验证码路径
     *
     * @param request HttpServletRequest
     * @param response HttpServletResponse
     * @throws Exception 异常
     */
    @ExterAttack
    @GetMapping(value = "/v1/captcha", produces = "image/jpeg")
    public void captcha(HttpServletRequest request, HttpServletResponse response) throws Exception {
        response.setHeader("Cache-Control", "no-cache");
        response.setContentType("image/jpeg");
        response.setHeader("X-UA-Compatible", "IE=edge,chrome=1");
        response.setHeader("X-Download-Options", "noopen");
        String random = captchaService.generateVerificationCode();
        BufferedImage im = CheckCodeUtil.createImage(random);
        File file = new File(captchaPath);
        if (!file.exists() && !file.mkdirs()) {
            response.setStatus(HttpStatus.SC_INTERNAL_SERVER_ERROR);
            return;
        }
        ImageIO.setCacheDirectory(new File(captchaPath));
        ImageIO.write(im, "JPEG", response.getOutputStream());
    }
}
