/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.emeistor.console.service;

import com.huawei.emeistor.console.bean.SecurityPolicyBo;

import org.springframework.http.HttpEntity;

/**
 * 安全策略相关的操作类
 *
 * @author t00482481
 * @since 2020-07-05
 */
public interface SecurityPolicyService {
    /**
     * 更新安全策略信息
     *
     * @param requestEntity 待更新的安全信息
     */
    void updateSecurityPolicy(HttpEntity<SecurityPolicyBo> requestEntity);

    /**
     * 获取安全策略信息
     *
     * @param httpEntity 带header的请求体
     * @return 获取安全策略信息
     */
    SecurityPolicyBo getSecurityPolicy(HttpEntity httpEntity);
}
