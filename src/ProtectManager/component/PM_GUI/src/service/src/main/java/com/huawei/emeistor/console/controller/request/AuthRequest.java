/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.emeistor.console.controller.request;

import lombok.Getter;
import lombok.Setter;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;
import javax.validation.constraints.NotBlank;
import javax.validation.constraints.NotNull;

/**
 * 功能描述
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2019-11-15
 */
@Getter
@Setter
public class AuthRequest {
    @NotBlank(message = "The username cannot be blank.")
    private String userName;

    @NotNull
    @NotBlank(message = "The password cannot be blank.")
    private String password;

    /**
     * 验证码
     */
    private String verifyCode;

    @NotBlank(message = "The userType cannot be blank.")
    private String userType = "COMMON";

    private String dynamicCode;

    @Max(value = 2, message = "out of range")
    @Min(value = 1, message = "out of range")
    private int language = 1;
}
