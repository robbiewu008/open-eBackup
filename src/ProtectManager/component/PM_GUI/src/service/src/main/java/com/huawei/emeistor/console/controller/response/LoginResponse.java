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
