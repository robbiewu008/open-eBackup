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
