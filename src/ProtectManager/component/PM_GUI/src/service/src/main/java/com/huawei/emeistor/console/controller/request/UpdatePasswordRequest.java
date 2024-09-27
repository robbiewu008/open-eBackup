/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package com.huawei.emeistor.console.controller.request;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 修改密码请求体
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2019-11-28
 */
@Getter
@Setter
@Builder(toBuilder = true)
@NoArgsConstructor
@AllArgsConstructor
@Data
public class UpdatePasswordRequest {
    /**
     * 原密码
     */
    private String originalPassword;

    /**
     * 新密码
     */
    private String newPassword;

    /**
     * 确定密码
     */
    private String confirmPassword;
}
