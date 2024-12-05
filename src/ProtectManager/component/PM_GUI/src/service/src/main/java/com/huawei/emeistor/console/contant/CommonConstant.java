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
package com.huawei.emeistor.console.contant;

/**
 * 常用常量
 *
 */
public class CommonConstant {
    /**
     * 一天多少秒
     */
    public static final int ONE_DAY = 86400;

    /**
     * 1000进制
     */
    public static final int ONE_THOUSAND = 1000;

    /**
     * user
     */
    public static final String USER = "User";

    /**
     * default 用户session数量
     */
    public static final Integer DEFAULT_SESSION_LIMIT = 5;

    /**
     * default 用户session超时时间
     */
    public static final long SESSION_EXPIRE = 3L;

    /**
     * 冒号
     */
    public static final String COLON = ":";

    /**
     * 登录时加解密的私钥key
     */
    public static final String PRIVATE_SYSTEM_CONFIG_KEY = "LOGIN_IN_PRIVATE_KEY";

    /**
     * 登录时加解密的公钥key
     */
    public static final String PUBLIC_SYSTEM_CONFIG_KEY = "LOGIN_IN_PUBLIC_KEY";

    private CommonConstant() {
    }
}
