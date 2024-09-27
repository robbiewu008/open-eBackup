/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.emeistor.console.controller.request;

import lombok.Data;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * KerberosCreateReq
 *
 * @author m00576658
 * @since 2021-08-13
 */
@Data
public class KerberosCreateReq {
    @NotNull
    @Size(min = 1, max = 64, message = "The length of name is 1-64.")
    @Pattern(regexp = "^[a-zA-Z0-9]{1,64}$")
    private String name;

    @Size(max = 2048, message = "The length of password is 0-2048.")
    private String password;

    @NotNull
    @Size(min = 1, max = 64, message = "The length of principal name is 1-64.")
    private String principalName;

    @NotNull
    private String createModel;
}
