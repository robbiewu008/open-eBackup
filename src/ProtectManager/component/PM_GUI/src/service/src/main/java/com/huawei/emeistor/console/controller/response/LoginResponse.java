/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package com.huawei.emeistor.console.controller.response;

import lombok.Getter;
import lombok.Setter;

/**
 * 登录请求response
 *
 * @author t00482481
 * @since 2020-9-06
 */
@Setter
@Getter
public class LoginResponse {
    private String sessionId;
    private boolean modifyPassword;
    private String userId;
    private long expireDay;
    private String lastLoginTime;
    private String lastLoginIp;
    private String lastLoginZone;
    private String serviceProduct;
}
