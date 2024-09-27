/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.emeistor.console.controller.request;

import lombok.Getter;
import lombok.Setter;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotBlank;
import javax.validation.constraints.NotNull;

/**
 * 发送动态口令请求
 *
 * @author c00826511
 * @since 2022-12-28
 */
@Getter
@Setter
public class SendDynamicCodeRequest {
    @NotBlank(message = "The username cannot be blank.")
    private String userName;

    @NotNull
    @NotBlank(message = "The password cannot be blank.")
    private String password;

    /**
     * 验证码
     */
    private String verifyCode;

    @NotNull
    @NotBlank(message = "The userType cannot be blank.")
    private String userType;

    @Max(value = 2, message = "out of range")
    @Min(value = 1, message = "out of range")
    private int language;
}
