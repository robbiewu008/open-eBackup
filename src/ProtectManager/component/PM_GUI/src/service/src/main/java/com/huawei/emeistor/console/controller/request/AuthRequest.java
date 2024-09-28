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
