/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.emeistor.console.bean;

import lombok.Getter;
import lombok.Setter;

import java.io.Serializable;

/**
 * session信息
 *
 * @author y00280557
 * @version [OceanProtect A8000 1.1.0]
 * @since 2020-11-30
 */
@Getter
@Setter
public class SessionInfo implements Serializable {
    // 会话id
    private String sessionId;

    // token
    private String token;

    // 过期时间
    private int expireTime;

    // 用户id
    private String userId;

    // csrfToken
    private String csrfToken;

    // 客户端IP
    private String clientSessionIp;
}
